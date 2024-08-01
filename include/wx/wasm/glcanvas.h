/////////////////////////////////////////////////////////////////////////////
// Name:        wx/wasm/glcanvas.h
// Purpose:     wxGLCanvas, for using WebAssembly OpenGL ES 2.0/3.0 
// Author:      Kamil Wieczorek
// Created:     2023-07-20
// Copyright:   (c) 2023 Kamil Wieczorek <k.wieczorek@inx-systems.co.uk>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GLCANVAS_H_
#define _WX_GLCANVAS_H_

#include "wx/setup.h"


#include "wx/wasm/glegl.h"
typedef wxGLCanvasEGL wxGLCanvasImpl;

// @TODO - add webGL implementation ?

//---------------------------------------------------------------------------
// wxGLCanvas
//---------------------------------------------------------------------------

class WXDLLIMPEXP_GL wxGLCanvas : public wxGLCanvasImpl
{
    typedef wxGLCanvasImpl BaseType;
public:
    wxGLCanvas(wxWindow *parent,
               const wxGLAttributes& dispAttrs,
               wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxString& name = wxGLCanvasName,
               const wxPalette& palette = wxNullPalette);

    explicit // avoid implicitly converting a wxWindow* to wxGLCanvas
    wxGLCanvas(wxWindow *parent,
               wxWindowID id = wxID_ANY,
               const int *attribList = NULL,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = 0,
               const wxString& name = wxGLCanvasName,
               const wxPalette& palette = wxNullPalette);

    bool Create(wxWindow *parent,
                const wxGLAttributes& dispAttrs,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxGLCanvasName,
                const wxPalette& palette = wxNullPalette);

    bool Create(wxWindow *parent,
                wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxGLCanvasName,
                const int *attribList = NULL,
                const wxPalette& palette = wxNullPalette);

private:
    wxDECLARE_CLASS(wxGLCanvas);
};

#endif // _WX_GLCANVAS_H_

