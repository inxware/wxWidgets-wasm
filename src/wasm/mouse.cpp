/////////////////////////////////////////////////////////////////////////////
// Name:        src/wasm/mouse.cpp
// Purpose      Mouse event converter
// Author:      Adam Hilss
// Copyright:   (c) 2022 Adam Hilss
// Licence:     LGPL v2
/////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#include "wx/event.h"
#include "wx/log.h"
#include <emscripten/html5.h>

//#define HAS_MOUSE_DETAIL

namespace
{

wxEventType GetMouseDownEventType(int button, int numClicks)
{
    switch (button)
    {
        case 0:
            return numClicks % 2 == 1 ? wxEVT_LEFT_DOWN : wxEVT_LEFT_DCLICK;
            break;
        case 1:
            return numClicks % 2 == 1 ? wxEVT_MIDDLE_DOWN : wxEVT_MIDDLE_DCLICK;
            break;
        case 2:
            return numClicks % 2 == 1 ? wxEVT_RIGHT_DOWN : wxEVT_RIGHT_DCLICK;
            break;
        default:
            wxFAIL_MSG(wxT("invalid mouse button"));
            return wxEVT_NULL;
            break;
    }
}

wxEventType GetMouseUpEventType(int button)
{
    switch (button)
    {
        case 0:
            return wxEVT_LEFT_UP;
            break;
        case 1:
            return wxEVT_MIDDLE_UP;
            break;
        case 2:
            return wxEVT_RIGHT_UP;
            break;
        default:
            wxFAIL_MSG(wxT("invalid mouse button"));
            return wxEVT_NULL;
            break;
    }
}

wxEventType GetMouseEventType(int emscriptenEventType,
                              const EmscriptenMouseEvent &event)
{
    wxEventType eventType;
    std::string eventName;

#ifdef HAS_MOUSE_DETAIL
    int clickCount = event.detail;
#else
    int clickCount = 1;
#endif

    switch (emscriptenEventType)
    {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
            eventName = "MOUSEDOWN";
            eventType = GetMouseDownEventType(event.button, clickCount);
            break;
        case EMSCRIPTEN_EVENT_MOUSEUP:
            eventName = "MOUSEUP";
            eventType = GetMouseUpEventType(event.button);
            break;
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
            eventName = "MOUSEMOVE";
            eventType = wxEVT_MOTION;
            break;
        case EMSCRIPTEN_EVENT_MOUSEENTER:
            eventName = "MOUSEENTER";
            eventType = wxEVT_ENTER_WINDOW;
            break;
        case EMSCRIPTEN_EVENT_MOUSELEAVE:
            eventName = "MOUSELEAVE";
            eventType = wxEVT_LEAVE_WINDOW;
            break;
        case EMSCRIPTEN_EVENT_CLICK:
            eventName = "CLICK";
            eventType = wxEVT_NULL;
            break;
        case EMSCRIPTEN_EVENT_DBLCLICK:
            eventName = "DBLCLICK";
            eventType = wxEVT_NULL;
            break;
        default:
            wxFAIL_MSG(wxT("invalid mouse event type"));
            eventType = wxEVT_NULL;
            break;
    }

    //printf("event: %s\n", eventName.c_str());

    return eventType;
}

void SetKeyboardState(const EmscriptenMouseEvent& emscriptenEvent,
                      wxMouseEvent *event)
{
    if ((wxGetOsVersion() & wxOS_MAC) != 0)
    {
        event->SetControlDown(emscriptenEvent.metaKey);
        event->SetMetaDown(false);
    }
    else
    {
        event->SetControlDown(emscriptenEvent.ctrlKey);
        event->SetMetaDown(emscriptenEvent.metaKey);
    }
    event->SetShiftDown(emscriptenEvent.shiftKey);
    event->SetAltDown(emscriptenEvent.altKey);
    event->SetRawControlDown(emscriptenEvent.ctrlKey);
}

void SetMouseState(const EmscriptenMouseEvent& emscriptenEvent, wxMouseState *event)
{
    event->SetX(emscriptenEvent.targetX);
    event->SetY(emscriptenEvent.targetY);
    event->SetLeftDown((emscriptenEvent.buttons & 1) != 0);
    event->SetRightDown((emscriptenEvent.buttons & 2) != 0);
    event->SetMiddleDown((emscriptenEvent.buttons & 4) != 0);
    event->SetAux1Down(false);
    event->SetAux2Down(false);
}

void InitMouseEventCommon(const EmscriptenMouseEvent& emscriptenEvent, wxMouseEvent *event)
{
    SetKeyboardState(emscriptenEvent, event);
    SetMouseState(emscriptenEvent, event);
}

} // anonymous namespace

bool EmscriptenWheelEventToWXEvent(const EmscriptenWheelEvent &emscriptenEvent,
                                   wxOrientation orientation,
                                   wxMouseEvent *mouseEvent)
{
    float delta = orientation == wxHORIZONTAL ? delta = emscriptenEvent.deltaX
                  : delta = -emscriptenEvent.deltaY;

    if (delta == 0)
    {
        return false;
    }

    mouseEvent->SetEventType(wxEVT_MOUSEWHEEL);
    InitMouseEventCommon(emscriptenEvent.mouse, mouseEvent);

    mouseEvent->m_wheelAxis =
        orientation == wxHORIZONTAL ? wxMOUSE_WHEEL_HORIZONTAL
        : wxMOUSE_WHEEL_VERTICAL;

    mouseEvent->m_wheelRotation = delta;
    mouseEvent->m_wheelDelta = 10;

    mouseEvent->m_linesPerAction = 1;
    mouseEvent->m_columnsPerAction = 1;

    return true;
}

bool EmscriptenMouseEventToWXEvent(int emscriptenEventType,
                                   const EmscriptenMouseEvent &emscriptenEvent,
                                   wxMouseEvent *mouseEvent)
{
    wxEventType eventType = GetMouseEventType(emscriptenEventType, emscriptenEvent);
    if (eventType == wxEVT_NULL)
    {
        return false;
    }

    mouseEvent->SetEventType(eventType);
    InitMouseEventCommon(emscriptenEvent, mouseEvent);

    if (eventType == wxEVT_MOTION)
    {
        mouseEvent->m_clickCount = 0;
    }
    else if (eventType == wxEVT_RIGHT_DCLICK ||
             eventType == wxEVT_MIDDLE_DCLICK ||
             eventType == wxEVT_LEFT_DCLICK)
    {
        mouseEvent->m_clickCount = 2;
    }
    else
    {
        mouseEvent->m_clickCount = 1;
    }

    //printf("mouse event type: %d\n", emscriptenEventType);
    //printf("mouse button: %d\n", emscriptenEvent.button);
    //printf("click count: %ld\n", emscriptenEvent.detail);
    //printf("mouse: left down: %d, right down: %d\n",
    //        mouseEvent->LeftIsDown(), mouseEvent->RightIsDown());

    return true;
}
