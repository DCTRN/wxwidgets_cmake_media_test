#pragma once

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxMediaPlayerFrame.hpp"

class wxMediaPlayerApp : public wxApp
{
public:
    virtual bool OnInit() override;

protected:
    class wxMediaPlayerFrame *m_frame;
};