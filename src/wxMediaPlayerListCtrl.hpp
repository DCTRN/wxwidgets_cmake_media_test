#pragma once

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/listctrl.h" // for wxListCtrl

class wxMediaPlayerListCtrl : public wxListCtrl
{
public:
    wxMediaPlayerListCtrl() = default;
    void AddToPlayList(const wxString &szString);
    void GetSelectedItem(wxListItem &listitem);
};
