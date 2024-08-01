///////////////////////////////////////////////////////////////////////////////
// Name:        wx/wasm/glegl.h
// Purpose:     class common to all EGL-based wxGLCanvas implementations
// Author:      Kamil Wieczorek
// Created:     2023-07-20
// Copyright:   (c) 2023 Kamil Wieczorek <k.wieczorek@inx-systems.co.uk>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_WASM_GLEGL_H_
#define _WX_WASM_GLEGL_H_

#include <GL/gl.h>

// This is to avoid including EGL headers here to pollute namespace
typedef void *EGLDisplay;
typedef void *EGLConfig;
typedef void *EGLSurface;
typedef void *EGLContext;

class wxGLContextAttrs;
class wxGLAttributes;

// ----------------------------------------------------------------------------
// wxGLContext
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_GL wxGLContext : public wxGLContextBase
{
public:
    wxGLContext(wxGLCanvas *win,
                const wxGLContext *other = NULL,
                const wxGLContextAttrs *ctxAttrs = NULL);
    virtual ~wxGLContext();

    virtual bool SetCurrent(const wxGLCanvas& win) const wxOVERRIDE;

private:
    EGLContext m_glContext;

    wxDECLARE_CLASS(wxGLContext);
};

// ----------------------------------------------------------------------------
// wxGLCanvasEGL
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_GL wxGLCanvasEGL : public wxGLCanvasBase
{
public:
    // initialization and dtor
    // -----------------------

    // default ctor doesn't do anything, InitConfig() must be called
    wxGLCanvasEGL();

    // initializes EGLConfig corresponding to the given attributes
    bool InitVisual(const wxGLAttributes& dispAttrs);

    // creates EGLSurface
    bool CreateSurface();

    virtual ~wxGLCanvasEGL();


    // implement wxGLCanvasBase methods
    // --------------------------------

    virtual bool SwapBuffers() wxOVERRIDE;


    // override some wxWindow methods
    // ------------------------------

    // return true only if the window is realized: OpenGL context can't be
    // created until we are
    virtual bool IsShownOnScreen() const wxOVERRIDE;


    // implementation only from now on
    // -------------------------------

    // get the EGLConfig we use
    EGLConfig *GetEGLConfig() const { return m_config; }
    EGLDisplay GetEGLDisplay() const { return m_display; }
    EGLSurface GetEGLSurface() const { return m_surface; }

    static EGLDisplay GetDisplay();

    // initialize the global default GL config, return false if matching config
    // not found
    static bool InitDefaultConfig(const int *attribList);

    // get the default EGL Config (may be NULL, shouldn't be freed by caller)
    static EGLConfig *GetDefaultConfig() { return ms_glEGLConfig; }

    // free the global GL visual, called by wxGLApp
    static void FreeDefaultConfig();

    // initializes EGLConfig
    //
    // returns NULL if EGLConfig couldn't be initialized, otherwise caller
    // is responsible for freeing the pointer
    static EGLConfig *InitConfig(const wxGLAttributes& dispAttrs);

    bool m_readyToDraw;

private:

    EGLConfig *m_config;
    EGLDisplay m_display;
    EGLSurface m_surface;

    // the global/default versions of the above
    static EGLConfig *ms_glEGLConfig;
};

// ----------------------------------------------------------------------------
// wxGLApp
// ----------------------------------------------------------------------------

// this is used in wx/glcanvas.h, prevent it from defining a generic wxGLApp
#define wxGL_APP_DEFINED

class WXDLLIMPEXP_GL wxGLApp : public wxGLAppBase
{
public:
    wxGLApp() : wxGLAppBase() { }

    // implement wxGLAppBase method
    virtual bool InitGLVisual(const int *attribList) wxOVERRIDE
    {
        return wxGLCanvasEGL::InitDefaultConfig(attribList);
    }
    ///////////// @TODO - this probably needs to go
    // This method is not currently used by the library itself, but remains for
    // backwards compatibility and also because wxGTK has it we could start
    // using it for the same purpose in wxX11 too some day.
    //virtual void* GetXVisualInfo() wxOVERRIDE
    //{
    //    return wxGLCanvasEGL::GetDefaultConfig();
    //}

    // and override this wxApp method to clean up
    virtual int OnExit() wxOVERRIDE
    {
        wxGLCanvasEGL::FreeDefaultConfig();

        return wxGLAppBase::OnExit();
    }

private:
    wxDECLARE_DYNAMIC_CLASS(wxGLApp);
};

#endif // _WX_WASM_GLEGL_H_
