///////////////////////////////////////////////////////////////////////////////
// Name:        src/wasm/glcanvas.cpp
// Purpose:     wxGLCanvas, for using wasm OpenGL ES with wxWidgets
// Author:      Kamil Wieczorek
// Created:     2023-07-20
// Copyright:   (c) 2023 Kamil Wieczorek <k.wieczorek@inx-systems.co.uk>
///////////////////////////////////////////////////////////////////////////////

// TODO: merge this with wxGTK version in some src/unix/glcanvasx11.cpp

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
    #include "wx/app.h"
    #include "wx/utils.h"
#endif

// ============================================================================
// implementation
// ============================================================================

wxIMPLEMENT_CLASS(wxGLCanvas, wxWindow);

wxGLCanvas::wxGLCanvas(wxWindow *parent,
                       const wxGLAttributes& dispAttrs,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const wxString& name,
                       const wxPalette& palette)
{
    Create(parent, dispAttrs, id, pos, size, style, name, palette);
}

wxGLCanvas::wxGLCanvas(wxWindow *parent,
                       wxWindowID id,
                       const int *attribList,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const wxString& name,
                       const wxPalette& palette)
{
    Create(parent, id, pos, size, style, name, attribList, palette);
}

bool wxGLCanvas::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const wxString& name,
                        const int *attribList,
                        const wxPalette& palette)
{
    // Separate 'GLXFBConfig/XVisual' attributes.
    // Also store context attributes for wxGLContext ctor
    wxGLAttributes dispAttrs;
    if ( ! ParseAttribList(attribList, dispAttrs, &m_GLCTXAttrs) )
        return false;

    return Create(parent, dispAttrs, id, pos, size, style, name, palette);
}

bool wxGLCanvas::Create(wxWindow *parent,
                        const wxGLAttributes& dispAttrs,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const wxString& name,
                        const wxPalette& WXUNUSED(palette))
{
    if ( !wxWindow::Create(parent, id, pos, size, style, name) )
        return false;

    if ( !InitVisual(dispAttrs) )
        return false;

    // @TODO - this may have to be moved where window is ready for paint. 
    if( ! CreateSurface() )
        return false;

    return true;
}


#endif // wxUSE_GLCANVAS
