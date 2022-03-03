#pragma once

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/timer.h" // timer for updating status bar

class wxMediaPlayerFrame;

class wxMediaPlayerTimer : public wxTimer
{
public:
    wxMediaPlayerTimer(wxMediaPlayerFrame *frame) { m_frame = frame; }

    void Notify() override;

    wxMediaPlayerFrame *m_frame;
};