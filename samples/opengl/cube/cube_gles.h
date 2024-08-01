#ifndef _WX_CUBE_H_
#define _WX_CUBE_H_

#include "wx/glcanvas.h"
#include <GLES3/gl3.h>
#include <EGL/egl.h>

class TestGLContext : public wxGLContext
{
public:
    TestGLContext(wxGLCanvas *canvas);
    ~TestGLContext();

    void DrawRotatedCube(float xangle, float yangle);

private:
    void InitializeShaders();
    void InitializeTextures();
    void InitializeBuffers();

    GLuint m_program;
    GLuint m_vertexBuffer;
    GLuint m_indexBuffer;
    GLuint m_textures[6];

    GLint m_positionAttribute;
    GLint m_texCoordAttribute;
    GLint m_mvpUniform;
    GLint m_samplerUniform;
};

class MyApp : public wxApp
{
public:
    MyApp() { m_glContext = NULL; }

    TestGLContext& GetContext(wxGLCanvas *canvas);

    virtual bool OnInit() wxOVERRIDE;
    virtual int OnExit() wxOVERRIDE;

private:
    TestGLContext *m_glContext;
};

class MyFrame : public wxFrame
{
public:
    MyFrame();

private:
    void OnClose(wxCommandEvent& event);
    void OnNewWindow(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

class TestGLCanvas : public wxGLCanvas
{
public:
    TestGLCanvas(wxWindow *parent, int *attribList);

private:
    void OnPaint(wxPaintEvent& event);
    void Spin(float xSpin, float ySpin);
    void OnKeyDown(wxKeyEvent& event);
    void OnSpinTimer(wxTimerEvent& WXUNUSED(event));

    float m_xangle, m_yangle;
    wxTimer m_spinTimer;

    wxDECLARE_EVENT_TABLE();
};

#endif // _WX_CUBE_H_