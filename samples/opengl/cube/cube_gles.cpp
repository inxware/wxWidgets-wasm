
// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif

#include "cube_gles.h"

#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "../../sample.xpm"
#endif
#include <cmath>

// control ids
enum
{
    SpinTimer = wxID_HIGHEST + 1
};


struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

struct Mat4 {
    float m[16];
    Mat4() { identity(); }
    void identity() {
        for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
};

Mat4 translate(const Mat4& mat, const Vec3& v) {
    Mat4 result = mat;
    result.m[12] = mat.m[0] * v.x + mat.m[4] * v.y + mat.m[8] * v.z + mat.m[12];
    result.m[13] = mat.m[1] * v.x + mat.m[5] * v.y + mat.m[9] * v.z + mat.m[13];
    result.m[14] = mat.m[2] * v.x + mat.m[6] * v.y + mat.m[10] * v.z + mat.m[14];
    result.m[15] = mat.m[3] * v.x + mat.m[7] * v.y + mat.m[11] * v.z + mat.m[15];
    return result;
}

Mat4 rotate(const Mat4& mat, float angle, const Vec3& axis) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    float t = 1 - c;
    Vec3 norm = axis;
    float len = std::sqrt(norm.x * norm.x + norm.y * norm.y + norm.z * norm.z);
    if (len != 0) {
        norm.x /= len; norm.y /= len; norm.z /= len;
    }

    Mat4 rot;
    rot.m[0] = c + norm.x * norm.x * t;
    rot.m[1] = norm.y * norm.x * t + norm.z * s;
    rot.m[2] = norm.z * norm.x * t - norm.y * s;
    rot.m[4] = norm.x * norm.y * t - norm.z * s;
    rot.m[5] = c + norm.y * norm.y * t;
    rot.m[6] = norm.z * norm.y * t + norm.x * s;
    rot.m[8] = norm.x * norm.z * t + norm.y * s;
    rot.m[9] = norm.y * norm.z * t - norm.x * s;
    rot.m[10] = c + norm.z * norm.z * t;

    Mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i*4+j] = mat.m[i*4] * rot.m[j] +
                               mat.m[i*4+1] * rot.m[4+j] +
                               mat.m[i*4+2] * rot.m[8+j] +
                               mat.m[i*4+3] * rot.m[12+j];
        }
    }
    return result;
}

Mat4 perspective(float fovy, float aspect, float zNear, float zFar) {
    float tanHalfFovy = std::tan(fovy / 2);
    Mat4 result;
    result.m[0] = 1 / (aspect * tanHalfFovy);
    result.m[5] = 1 / tanHalfFovy;
    result.m[10] = -(zFar + zNear) / (zFar - zNear);
    result.m[11] = -1;
    result.m[14] = -(2 * zFar * zNear) / (zFar - zNear);
    result.m[15] = 0;
    return result;
}

Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i*4+j] = a.m[i*4] * b.m[j] +
                               a.m[i*4+1] * b.m[4+j] +
                               a.m[i*4+2] * b.m[8+j] +
                               a.m[i*4+3] * b.m[12+j];
        }
    }
    return result;
}

// Shader source code
const char* vertexShaderSource = R"(
    attribute vec3 a_position;
    attribute vec2 a_texCoord;
    uniform mat4 u_mvpMatrix;
    varying vec2 v_texCoord;
    void main() {
        gl_Position = u_mvpMatrix * vec4(a_position, 1.0);
        v_texCoord = a_texCoord;
    }
)";

const char* fragmentShaderSource = R"(
    precision mediump float;
    varying vec2 v_texCoord;
    uniform sampler2D u_sampler;
    void main() {
        gl_FragColor = texture2D(u_sampler, v_texCoord);
    }
)";

// function to draw the texture for cube faces
static wxImage DrawDice(int size, unsigned num)
{
    wxASSERT_MSG( num >= 1 && num <= 6, "invalid dice index" );

    const int dot = size/16;        // radius of a single dot
    const int gap = 5*size/32;      // gap between dots

    wxBitmap bmp(size, size);
    wxMemoryDC dc;
    dc.SelectObject(bmp);
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();
    dc.SetBrush(*wxBLACK_BRUSH);

    // the upper left and lower right points
    if ( num != 1 )
    {
        dc.DrawCircle(gap + dot, gap + dot, dot);
        dc.DrawCircle(size - gap - dot, size - gap - dot, dot);
    }

    // draw the central point for odd dices
    if ( num % 2 )
    {
        dc.DrawCircle(size/2, size/2, dot);
    }

    // the upper right and lower left points
    if ( num > 3 )
    {
        dc.DrawCircle(size - gap - dot, gap + dot, dot);
        dc.DrawCircle(gap + dot, size - gap - dot, dot);
    }

    // finally those 2 are only for the last dice
    if ( num == 6 )
    {
        dc.DrawCircle(gap + dot, size/2, dot);
        dc.DrawCircle(size - gap - dot, size/2, dot);
    }

    dc.SelectObject(wxNullBitmap);

    return bmp.ConvertToImage();
}

TestGLContext::TestGLContext(wxGLCanvas *canvas)
    : wxGLContext(canvas)
{
    SetCurrent(*canvas);

    InitializeShaders();
    InitializeTextures();
    InitializeBuffers();
}

void TestGLContext::InitializeShaders()
{
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Link shaders
    m_program = glCreateProgram();
    glAttachShader(m_program, vertexShader);
    glAttachShader(m_program, fragmentShader);
    glLinkProgram(m_program);

    // Get attribute and uniform locations
    m_positionAttribute = glGetAttribLocation(m_program, "a_position");
    m_texCoordAttribute = glGetAttribLocation(m_program, "a_texCoord");
    m_mvpUniform = glGetUniformLocation(m_program, "u_mvpMatrix");
    m_samplerUniform = glGetUniformLocation(m_program, "u_sampler");

    // Clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void TestGLContext::InitializeBuffers()
{
    // Vertex data for the cube
    const GLfloat vertices[] = {
        // Positions        // Texture Coordinates
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        // ... (remaining vertices)
    };

    const GLushort indices[] = {
        0, 1, 2, 2, 3, 0,
        // ... (remaining indices)
    };

    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void TestGLContext::DrawRotatedCube(float xangle, float yangle)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_program);

    // Set up MVP matrix
    Mat4 model;
    model = rotate(model, xangle * M_PI / 180.0f, Vec3(1.0f, 0.0f, 0.0f));
    model = rotate(model, yangle * M_PI / 180.0f, Vec3(0.0f, 1.0f, 0.0f));
    Mat4 view = translate(Mat4(), Vec3(0.0f, 0.0f, -2.0f));
    Mat4 projection = perspective(45.0f * M_PI / 180.0f, 1.0f, 0.1f, 100.0f);
    Mat4 mvp = multiply(projection, multiply(view, model));

    glUniformMatrix4fv(m_mvpUniform, 1, GL_FALSE, mvp.m);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

    glVertexAttribPointer(m_positionAttribute, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glVertexAttribPointer(m_texCoordAttribute, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(m_positionAttribute);
    glEnableVertexAttribArray(m_texCoordAttribute);

    for (int i = 0; i < 6; ++i) {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glUniform1i(m_samplerUniform, 0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)(i * 6 * sizeof(GLushort)));
    }

    glDisableVertexAttribArray(m_positionAttribute);
    glDisableVertexAttribArray(m_texCoordAttribute);
}

// ... (previous code remains the same)

void TestGLContext::InitializeTextures()
{
    glGenTextures(6, m_textures);

    for (int i = 0; i < 6; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        wxImage img = DrawDice(256, i + 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.GetWidth(), img.GetHeight(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, img.GetData());
    }
}

TestGLContext::~TestGLContext()
{
    glDeleteProgram(m_program);
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_indexBuffer);
    glDeleteTextures(6, m_textures);
}

// DrawDice function remains the same as in the original implementation

// MyApp implementation
wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    new MyFrame();

    return true;
}

int MyApp::OnExit()
{
    delete m_glContext;

    return wxApp::OnExit();
}

TestGLContext& MyApp::GetContext(wxGLCanvas *canvas)
{
    if (!m_glContext)
    {
        m_glContext = new TestGLContext(canvas);
    }

    m_glContext->SetCurrent(*canvas);

    return *m_glContext;
}

// MyFrame implementation
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(wxID_NEW, MyFrame::OnNewWindow)
    EVT_MENU(wxID_CLOSE, MyFrame::OnClose)
wxEND_EVENT_TABLE()

MyFrame::MyFrame()
       : wxFrame(NULL, wxID_ANY, "wxWidgets OpenGL ES")
{
    int attribList[] = {
        WX_GL_DEPTH_SIZE, 16,
        0
    };

    new TestGLCanvas(this, attribList);

    SetIcon(wxICON(sample));

    // Make a menubar
    wxMenu *menu = new wxMenu;
    menu->Append(wxID_NEW);
    menu->AppendSeparator();
    menu->Append(wxID_CLOSE);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menu, "&Cube");

    SetMenuBar(menuBar);

    CreateStatusBar();

    SetClientSize(400, 400);
    Show();
}

void MyFrame::OnClose(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}

void MyFrame::OnNewWindow(wxCommandEvent& WXUNUSED(event))
{
    new MyFrame();
}

// TestGLCanvas implementation
wxBEGIN_EVENT_TABLE(TestGLCanvas, wxGLCanvas)
    EVT_PAINT(TestGLCanvas::OnPaint)
    EVT_KEY_DOWN(TestGLCanvas::OnKeyDown)
    EVT_TIMER(SpinTimer, TestGLCanvas::OnSpinTimer)
wxEND_EVENT_TABLE()

TestGLCanvas::TestGLCanvas(wxWindow *parent, int *attribList)
    : wxGLCanvas(parent, wxID_ANY, attribList,
                 wxDefaultPosition, wxDefaultSize,
                 wxFULL_REPAINT_ON_RESIZE),
      m_xangle(30.0),
      m_yangle(30.0),
      m_spinTimer(this, SpinTimer)
{
}

void TestGLCanvas::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);

    const wxSize ClientSize = GetClientSize() * GetContentScaleFactor();

    TestGLContext& canvas = wxGetApp().GetContext(this);
    
    glViewport(0, 0, ClientSize.x, ClientSize.y);

    canvas.DrawRotatedCube(m_xangle, m_yangle);

    SwapBuffers();
}

void TestGLCanvas::Spin(float xSpin, float ySpin)
{
    m_xangle += xSpin;
    m_yangle += ySpin;

    Refresh(false);
}

void TestGLCanvas::OnKeyDown(wxKeyEvent& event)
{
    float angle = 5.0;

    switch (event.GetKeyCode())
    {
        case WXK_RIGHT:
            Spin(0.0, -angle);
            break;

        case WXK_LEFT:
            Spin(0.0, angle);
            break;

        case WXK_DOWN:
            Spin(-angle, 0.0);
            break;

        case WXK_UP:
            Spin(angle, 0.0);
            break;

        case WXK_SPACE:
            if (m_spinTimer.IsRunning())
                m_spinTimer.Stop();
            else
                m_spinTimer.Start(25);
            break;

        default:
            event.Skip();
            return;
    }
}

void TestGLCanvas::OnSpinTimer(wxTimerEvent& WXUNUSED(event))
{
    Spin(0.0, 4.0);
}

// Helper function to get OpenGL ES info
wxString glGetwxString(GLenum name)
{
    const GLubyte *v = glGetString(name);
    if (v == 0)
    {
        // The error is not important. It is GL_INVALID_ENUM.
        // We just want to clear the error stack.
        glGetError();

        return wxString();
    }

    return wxString((const char*)v);
}