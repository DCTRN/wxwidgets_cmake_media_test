#pragma once

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/dnd.h" // drag and drop for the playlist
#include "wx/vector.h"
#include "wxMediaPlayerListCtrl.hpp"

class wxPlayListDropTarget : public wxFileDropTarget
{
public:
    wxPlayListDropTarget(wxMediaPlayerListCtrl &list) : m_list(list) {}
    virtual bool OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                             const wxArrayString &files);
    wxMediaPlayerListCtrl &m_list;
};