///////////////////////////////////////////////////////////////////////////////
// Name:        src/wasm/glegl.cpp
// Purpose:     code common to all EGL-based wxGLCanvas implementations
// Author:      Kamil Wieczorek
// Created:     2023-07-20
// Copyright:   (c) 2023 Kamil Wieczorek <k.wieczorek@inx-systems.co.uk>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_GLCANVAS

#include "wx/glcanvas.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
#endif //WX_PRECOMP


#include <EGL/egl.h>
#include <EGL/eglext.h>

// ----------------------------------------------------------------------------
// wxGLContextAttrs: OpenGL rendering context attributes
// ----------------------------------------------------------------------------
// EGL specific values

wxGLContextAttrs& wxGLContextAttrs::CoreProfile()
{
    AddAttribBits(EGL_CONTEXT_OPENGL_PROFILE_MASK,
                  EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT);
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::MajorVersion(int val)
{
    if ( val > 0 )
    {
        AddAttribute(EGL_CONTEXT_MAJOR_VERSION);
        AddAttribute(val);
    }
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::MinorVersion(int val)
{
    if ( val >= 0 )
    {
        AddAttribute(EGL_CONTEXT_MINOR_VERSION);
        AddAttribute(val);
    }
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::CompatibilityProfile()
{
    AddAttribBits(EGL_CONTEXT_OPENGL_PROFILE_MASK,
                  EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT);
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::ForwardCompatible()
{
    AddAttribute(EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE);
    AddAttribute(EGL_TRUE);
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::ES2()
{
    AddAttribBits(EGL_RENDERABLE_TYPE,
                  EGL_OPENGL_ES2_BIT);
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::DebugCtx()
{
    AddAttribute(EGL_CONTEXT_OPENGL_DEBUG);
    AddAttribute(EGL_TRUE);
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::Robust()
{
    AddAttribute(EGL_CONTEXT_OPENGL_ROBUST_ACCESS);
    AddAttribute(EGL_TRUE);
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::NoResetNotify()
{
    AddAttribute(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY);
    AddAttribute(EGL_NO_RESET_NOTIFICATION);
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::LoseOnReset()
{
    AddAttribute(EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY);
    AddAttribute(EGL_LOSE_CONTEXT_ON_RESET);
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::ResetIsolation()
{
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::ReleaseFlush(int)
{
    return *this;
}

wxGLContextAttrs& wxGLContextAttrs::PlatformDefaults()
{
    return *this;
}

void wxGLContextAttrs::EndList()
{
    AddAttribute(EGL_NONE);
}

// ----------------------------------------------------------------------------
// wxGLAttributes: Visual/FBconfig attributes
// ----------------------------------------------------------------------------

wxGLAttributes& wxGLAttributes::RGBA()
{
    return *this;
}

wxGLAttributes& wxGLAttributes::BufferSize(int val)
{
    if ( val >= 0 )
    {
        AddAttribute(EGL_BUFFER_SIZE);
        AddAttribute(val);
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::Level(int val)
{
    AddAttribute(EGL_LEVEL);
    AddAttribute(val);
    return *this;
}

wxGLAttributes& wxGLAttributes::DoubleBuffer()
{
    return *this;
}

wxGLAttributes& wxGLAttributes::Stereo()
{
    return *this;
}

wxGLAttributes& wxGLAttributes::AuxBuffers(int)
{
    return *this;
}

wxGLAttributes& wxGLAttributes::MinRGBA(int mRed, int mGreen, int mBlue, int mAlpha)
{
    if ( mRed >= 0)
    {
        AddAttribute(EGL_RED_SIZE);
        AddAttribute(mRed);
    }
    if ( mGreen >= 0)
    {
        AddAttribute(EGL_GREEN_SIZE);
        AddAttribute(mGreen);
    }
    if ( mBlue >= 0)
    {
        AddAttribute(EGL_BLUE_SIZE);
        AddAttribute(mBlue);
    }
    if ( mAlpha >= 0)
    {
        AddAttribute(EGL_ALPHA_SIZE);
        AddAttribute(mAlpha);
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::Depth(int val)
{
    if ( val >= 0 )
    {
        AddAttribute(EGL_DEPTH_SIZE);
        AddAttribute(val);
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::Stencil(int val)
{
    if ( val >= 0 )
    {
        AddAttribute(EGL_STENCIL_SIZE);
        AddAttribute(val);
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::MinAcumRGBA(int, int, int, int)
{
    return *this;
}

wxGLAttributes& wxGLAttributes::SampleBuffers(int val)
{
    if ( val >= 0 )
    {
        AddAttribute(EGL_SAMPLE_BUFFERS);
        AddAttribute(val);
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::Samplers(int val)
{
    if ( val >= 0 )
    {
        AddAttribute(EGL_SAMPLES);
        AddAttribute(val);
    }
    return *this;
}

wxGLAttributes& wxGLAttributes::FrameBuffersRGB()
{
    return *this;
}

void wxGLAttributes::EndList()
{
    AddAttribute(EGL_NONE);
}

wxGLAttributes& wxGLAttributes::PlatformDefaults()
{
    // No EGL specific values
    return *this;
}

wxGLAttributes& wxGLAttributes::Defaults()
{
    RGBA().DoubleBuffer().Depth(16).SampleBuffers(1).Samplers(4);
    return *this;
}

void wxGLAttributes::AddDefaultsForWXBefore31()
{
    // ParseAttribList() will add EndList(), don't do it now
    DoubleBuffer();
}


// ============================================================================
// wxGLContext implementation
// ============================================================================

wxIMPLEMENT_CLASS(wxGLContext, wxObject);

wxGLContext::wxGLContext(wxGLCanvas *win,
                         const wxGLContext *other,
                         const wxGLContextAttrs *ctxAttrs)
    : m_glContext(NULL)
{
    const int* contextAttribs = NULL;

    if ( ctxAttrs )
    {
        contextAttribs = ctxAttrs->GetGLAttrs();
    }
    else if ( win->GetGLCTXAttrs().GetGLAttrs() )
    {
        // If OpenGL context parameters were set at wxGLCanvas ctor, get them now
        contextAttribs = win->GetGLCTXAttrs().GetGLAttrs();
    }

    m_isOk = false;

    EGLConfig* fbc = win->GetEGLConfig();
    wxCHECK_RET( fbc, "Invalid EGLConfig for OpenGL" );

    m_glContext = eglCreateContext(wxGLCanvasEGL::GetDisplay(), *fbc,
                                   other ? other->m_glContext : EGL_NO_CONTEXT,
                                   contextAttribs);

    if ( !m_glContext ){
        wxFAIL_MSG(wxString("Couldn't create OpenGL context error ") << eglGetError());
    }else{
        m_isOk = true;
    }
}

wxGLContext::~wxGLContext()
{
    if ( !m_glContext )
        return;

    if ( m_glContext == eglGetCurrentContext() )
        eglMakeCurrent(wxGLCanvasEGL::GetDisplay(), EGL_NO_SURFACE,
                       EGL_NO_SURFACE, EGL_NO_CONTEXT);

    eglDestroyContext(wxGLCanvasEGL::GetDisplay(), m_glContext);
}

bool wxGLContext::SetCurrent(const wxGLCanvas& win) const
{
    if ( !m_glContext )
        return false;

    return eglMakeCurrent(win.GetEGLDisplay(), win.GetEGLSurface(),
                          win.GetEGLSurface(), m_glContext);
}

// ============================================================================
// wxGLCanvasEGL implementation
// ============================================================================

// ----------------------------------------------------------------------------
// initialization methods and dtor
// ----------------------------------------------------------------------------

wxGLCanvasEGL::wxGLCanvasEGL()
{
    m_config = NULL;
    m_display = NULL;
    m_surface = EGL_NO_SURFACE;
    m_readyToDraw = false;
}

bool wxGLCanvasEGL::InitVisual(const wxGLAttributes& dispAttrs)
{
    m_config = InitConfig(dispAttrs);
    if ( !m_config )
    {
        wxFAIL_MSG("Failed to get an EGLConfig for the requested attributes.");
    }
    return m_config != NULL;
}

/* static */
EGLDisplay wxGLCanvasEGL::GetDisplay()
{
    return eglGetDisplay(EGL_DEFAULT_DISPLAY);
}

bool wxGLCanvasEGL::CreateSurface()
{
    m_display = GetDisplay();
    EGLint eglError = eglGetError();
    if ( m_display == EGL_NO_DISPLAY || eglError != EGL_SUCCESS )
    {
        wxFAIL_MSG( wxString("Unable to get EGL Display error no:") << eglError );
        return false;
    }
    // Simply skip window creation because the handle is ignored by Emscripten.
    EGLNativeWindowType dummyWindow;
    m_surface = eglCreateWindowSurface(m_display, m_config, dummyWindow, NULL);

    eglError = eglGetError();

    if ( m_surface == EGL_NO_SURFACE || eglError != EGL_SUCCESS )
    {
        wxFAIL_MSG( wxString("Unable to create EGL surface error no:") << eglError );
        return false;
    }

    m_readyToDraw = true;

    return true;
}

wxGLCanvasEGL::~wxGLCanvasEGL()
{
    if ( m_config && m_config != ms_glEGLConfig )
        delete m_config;
    if ( m_surface )
        eglDestroySurface(m_display, m_surface);

}

// ----------------------------------------------------------------------------
// working with GL attributes
// ----------------------------------------------------------------------------

/* static */
bool wxGLCanvasBase::IsExtensionSupported(const char *extension)
{
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    return IsExtensionInList(eglQueryString(dpy, EGL_EXTENSIONS), extension);
}


/* static */
EGLConfig *wxGLCanvasEGL::InitConfig(const wxGLAttributes& dispAttrs)
{
    const int* attrsList = dispAttrs.GetGLAttrs();
    if ( !attrsList )
    {
        wxFAIL_MSG("wxGLAttributes object is empty.");
        return NULL;
    }

    EGLDisplay dpy = GetDisplay();
    if ( dpy == EGL_NO_DISPLAY ) {
        wxFAIL_MSG("Unable to get EGL Display");
        return NULL;
    }
    EGLint major, minor;
    if ( !eglInitialize(dpy, &major, &minor) )
    {
        wxFAIL_MSG("eglInitialize failed");
        return NULL;
    }

    EGLConfig *config = new EGLConfig;
    int returned;
    // Use the first good match
    if ( eglChooseConfig(dpy, attrsList, config, 1, &returned) && returned == 1 )
    {
        return config;
    }
    else
    {
        wxFAIL_MSG("eglChooseConfig failed");
        delete config;
        return NULL;
    }
}

/* static */
bool wxGLCanvasBase::IsDisplaySupported(const wxGLAttributes& /*dispAttrs*/)
{
    // In WebGL, we can't really check this ahead of time.
    // We could potentially check for WebGL support in general.
    return true;
}

/* static */
bool wxGLCanvasBase::IsDisplaySupported(const int *attribList)
{
    wxGLAttributes dispAttrs;
    ParseAttribList(attribList, dispAttrs);

    return IsDisplaySupported(dispAttrs);
}

// ----------------------------------------------------------------------------
// default visual management
// ----------------------------------------------------------------------------

EGLConfig *wxGLCanvasEGL::ms_glEGLConfig = NULL;

/* static */
bool wxGLCanvasEGL::InitDefaultConfig(const int *attribList)
{
    FreeDefaultConfig();
    wxGLAttributes dispAttrs;
    ParseAttribList(attribList, dispAttrs);

    ms_glEGLConfig = InitConfig(dispAttrs);
    return ms_glEGLConfig != NULL;
}

/* static */
void wxGLCanvasEGL::FreeDefaultConfig()
{
    if ( ms_glEGLConfig )
    {
        delete ms_glEGLConfig;
        ms_glEGLConfig = NULL;
    }
}

// ----------------------------------------------------------------------------
// other GL methods
// ----------------------------------------------------------------------------

bool wxGLCanvasEGL::SwapBuffers()
{
    // Under Wayland, if eglSwapBuffers() is called before the wl_surface has
    // been realized, it will deadlock.  Thus, we need to avoid swapping before
    // this has happened.
    if ( !m_readyToDraw )
        return false;

    return eglSwapBuffers(m_display, m_surface);
}

bool wxGLCanvasEGL::IsShownOnScreen() const
{
    return m_readyToDraw && wxGLCanvasBase::IsShownOnScreen();
}

#endif // wxUSE_GLCANVAS && wxUSE_GLCANVAS_EGL

