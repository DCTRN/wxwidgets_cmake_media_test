#pragma once

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/mediactrl.h" // for wxMediaCtrl
#include "wx/listctrl.h"  // for wxListCtrl

class wxMediaPlayerTimer;
class wxMediaPlayerApp;
class wxMediaPlayerNotebookPage;
class wxMediaPlayerTimer;
class wxNotebook;

class wxMediaPlayerFrame : public wxFrame
{
public:
    wxMediaPlayerFrame(const wxString &title);
    ~wxMediaPlayerFrame();

    void OnAbout(wxCommandEvent &event);

    void OnOpenFileSamePage(wxCommandEvent &event);
    void OnOpenFileNewPage(wxCommandEvent &event);
    void OnOpenURLSamePage(wxCommandEvent &event);
    void OnOpenURLNewPage(wxCommandEvent &event);
    void OnCloseCurrentPage(wxCommandEvent &event);

    void OnPlay(wxCommandEvent &event);
    void OnPause(wxCommandEvent &event);
    void OnStop(wxCommandEvent &event);
    void OnNext(wxCommandEvent &event);
    void OnPrev(wxCommandEvent &event);
    void OnVolumeDown(wxCommandEvent &event);
    void OnVolumeUp(wxCommandEvent &event);

    void OnLoop(wxCommandEvent &event);

    void OnKeyDown(wxKeyEvent &event);

    void AddToPlayList(const wxString &szString);

    void OnChangeSong(wxListEvent &event);

    void OnMediaLoaded(wxMediaEvent &event);

private:
    void OpenFile(bool bNewPage);
    void OpenURL(bool bNewPage);
    void DoOpenFile(const wxString &path, bool bNewPage);
    void DoPlayFile(const wxString &path);

    wxMediaPlayerTimer *m_timer;
    wxNotebook *m_notebook;

    friend class wxMediaPlayerApp;
    friend class wxMediaPlayerNotebookPage;
    friend class wxMediaPlayerTimer;
};