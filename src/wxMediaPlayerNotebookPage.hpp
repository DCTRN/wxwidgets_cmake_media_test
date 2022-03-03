#pragma once

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/mediactrl.h" // for wxMediaCtrl
#include "wx/notebook.h"  // for wxNotebook and putting movies in pages
#include "wx/slider.h"    // for a slider for seeking within media

#include "wxMediaPlayerListCtrl.hpp"

class wxMediaPlayerFrame;

class wxMediaPlayerNotebookPage : public wxPanel
{
    wxMediaPlayerNotebookPage(wxMediaPlayerFrame *parentFrame,
                              wxNotebook *book, const wxString &be = wxEmptyString);

    void OnBeginSeek(wxScrollEvent &event);
    void OnEndSeek(wxScrollEvent &event);
    void OnPBChange(wxScrollEvent &event);
    void OnVolChange(wxScrollEvent &event);

    void OnMediaPlay(wxMediaEvent &event);
    void OnMediaPause(wxMediaEvent &event);
    void OnMediaStop(wxMediaEvent &event);
    void OnMediaFinished(wxMediaEvent &event);

public:
    bool IsBeingDragged();

    friend class wxMediaPlayerFrame;

    int m_nLastFileId;
    wxString m_szFile;

    wxMediaCtrl *m_mediactrl;          // Our media control
    wxMediaPlayerListCtrl *m_playlist; // Our playlist
    wxSlider *m_slider;                // The slider below our media control
    wxSlider *m_pbSlider;              // Lower-left slider for adjusting speed
    wxSlider *m_volSlider;             // Lower-right slider for adjusting volume
    int m_nLoops;                      // Number of times media has looped
    bool m_bLoop;                      // Whether we are looping or not
    bool m_bIsBeingDragged;            // Whether the user is dragging the scroll bar
    wxMediaPlayerFrame *m_parentFrame; // Main wxFrame of our sample
    wxButton *m_prevButton;            // Go to previous file button
    wxButton *m_playButton;            // Play/pause file button
    wxButton *m_stopButton;            // Stop playing file button
    wxButton *m_nextButton;            // Next file button
    wxButton *m_vdButton;              // Volume down button
    wxButton *m_vuButton;              // Volume up button
    wxGauge *m_gauge;                  // Gauge to keep in line with slider
};