#include "wx/filedlg.h"  // for opening files from OpenFile
#include "wx/sizer.h"    // for positioning controls/wxBoxSizer
#include "wx/textdlg.h"  // for getting user text from OpenURL/Debug
#include "wx/filename.h" // For wxFileName::GetName()
#include "wx/config.h"   // for native wxConfig
#include "wx/vector.h"

#include "wxMediaPlayerNotebookPage.hpp"
#include "wxPlayListDropTarget.hpp"
#include "wxMediaPlayerFrame.hpp"
#include "Ids.hpp"

static const wxString wxGetMediaStateText(int nState)
{
    switch (nState)
    {
    case wxMEDIASTATE_PLAYING:
        return "Playing";
    case wxMEDIASTATE_STOPPED:
        return "Stopped";
    /// case wxMEDIASTATE_PAUSED:
    default:
        return "Paused";
    }
}

wxMediaPlayerNotebookPage::wxMediaPlayerNotebookPage(wxMediaPlayerFrame *parentFrame,
                                                     wxNotebook *theBook,
                                                     const wxString &szBackend)
    : wxPanel(theBook, wxID_ANY),
      m_nLastFileId(-1),
      m_nLoops(0),
      m_bLoop(false),
      m_bIsBeingDragged(false),
      m_parentFrame(parentFrame)
{

    wxFlexGridSizer *sizer = new wxFlexGridSizer(2);
    sizer->AddGrowableCol(0);
    this->SetSizer(sizer);

    m_mediactrl = new wxMediaCtrl();

    bool bOK = m_mediactrl->Create(this, wxID_MEDIACTRL, wxEmptyString,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxST_NO_AUTORESIZE);
    wxASSERT_MSG(bOK, "Could not create media control!");
    wxUnusedVar(bOK);

    sizer->Add(m_mediactrl, wxSizerFlags().Expand().Border());

    m_playlist = new wxMediaPlayerListCtrl();
    m_playlist->Create(this, wxID_LISTCTRL, wxDefaultPosition,
                       wxDefaultSize,
                       wxLC_REPORT | wxSUNKEN_BORDER);

    m_playlist->SetBackgroundColour(*wxWHITE);

    m_playlist->AppendColumn(_(""), wxLIST_FORMAT_CENTER, 20);
    m_playlist->AppendColumn(_("File"), wxLIST_FORMAT_LEFT, 305);
    m_playlist->AppendColumn(_("Length"), wxLIST_FORMAT_CENTER, 75);

    m_playlist->SetDropTarget(new wxPlayListDropTarget(*m_playlist));

    sizer->Add(m_playlist, 0, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 5);

    //
    //  Create the control buttons
    //  TODO/FIXME/HACK:  This part about sizers is really a nice hack
    //                    and probably isn't proper
    //
    wxBoxSizer *horsizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vertsizer = new wxBoxSizer(wxHORIZONTAL);

    m_prevButton = new wxButton();
    m_playButton = new wxButton();
    m_stopButton = new wxButton();
    m_nextButton = new wxButton();
    m_vdButton = new wxButton();
    m_vuButton = new wxButton();

    m_prevButton->Create(this, wxID_BUTTONPREV, "|<");
    m_prevButton->SetToolTip("Previous");
    m_playButton->Create(this, wxID_BUTTONPLAY, ">");
    m_playButton->SetToolTip("Play");
    m_stopButton->Create(this, wxID_BUTTONSTOP, "[]");
    m_stopButton->SetToolTip("Stop");
    m_nextButton->Create(this, wxID_BUTTONNEXT, ">|");
    m_nextButton->SetToolTip("Next");
    m_vdButton->Create(this, wxID_BUTTONVD, "((");
    m_vdButton->SetToolTip("Volume down");
    m_vuButton->Create(this, wxID_BUTTONVU, "))");
    m_vuButton->SetToolTip("Volume up");

    vertsizer->Add(m_prevButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    vertsizer->Add(m_playButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    vertsizer->Add(m_stopButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    vertsizer->Add(m_nextButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    vertsizer->Add(m_vdButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    vertsizer->Add(m_vuButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    horsizer1->Add(vertsizer, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    sizer->Add(horsizer1, 0, wxALIGN_CENTER_VERTICAL | wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    m_slider = new wxSlider(this, wxID_SLIDER, 0,
                            0,
                            1,
                            wxDefaultPosition, wxDefaultSize,
                            wxSL_HORIZONTAL);
    sizer->Add(m_slider, 0, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 5);

    m_gauge = new wxGauge();
    m_gauge->Create(this, wxID_GAUGE, 0, wxDefaultPosition, wxDefaultSize,
                    wxGA_HORIZONTAL | wxGA_SMOOTH);
    sizer->Add(m_gauge, 0, wxALIGN_CENTER_HORIZONTAL | wxALL | wxEXPAND, 5);

    wxBoxSizer *horsizer3 = new wxBoxSizer(wxHORIZONTAL);

    m_volSlider = new wxSlider(this, wxID_VOLSLIDER, 100,
                               0,
                               100,
                               wxDefaultPosition, wxDefaultSize,
                               wxSL_HORIZONTAL);
    horsizer3->Add(m_volSlider, 1, wxALL, 5);

    m_pbSlider = new wxSlider(this, wxID_PBSLIDER, 4,
                              1,
                              16,
                              wxDefaultPosition, wxDefaultSize,
                              wxSL_HORIZONTAL);
    horsizer3->Add(m_pbSlider, 1, wxALL, 5);
    sizer->Add(horsizer3, 1, wxCENTRE | wxALL, 5);

    sizer->AddGrowableRow(0);

    Bind(wxEVT_LIST_ITEM_ACTIVATED, &wxMediaPlayerFrame::OnChangeSong, parentFrame,
         wxID_LISTCTRL);

    Bind(wxEVT_SCROLL_THUMBTRACK, &wxMediaPlayerNotebookPage::OnBeginSeek, this,
         wxID_SLIDER);
    Bind(wxEVT_SCROLL_THUMBRELEASE, &wxMediaPlayerNotebookPage::OnEndSeek, this,
         wxID_SLIDER);
    Bind(wxEVT_SCROLL_THUMBRELEASE, &wxMediaPlayerNotebookPage::OnPBChange, this,
         wxID_PBSLIDER);
    Bind(wxEVT_SCROLL_THUMBRELEASE, &wxMediaPlayerNotebookPage::OnVolChange, this,
         wxID_VOLSLIDER);

    Bind(wxEVT_MEDIA_PLAY, &wxMediaPlayerNotebookPage::OnMediaPlay, this,
         wxID_MEDIACTRL);
    Bind(wxEVT_MEDIA_PAUSE, &wxMediaPlayerNotebookPage::OnMediaPause, this,
         wxID_MEDIACTRL);
    Bind(wxEVT_MEDIA_STOP, &wxMediaPlayerNotebookPage::OnMediaStop, this,
         wxID_MEDIACTRL);
    Bind(wxEVT_MEDIA_FINISHED, &wxMediaPlayerNotebookPage::OnMediaFinished, this,
         wxID_MEDIACTRL);
    Bind(wxEVT_MEDIA_LOADED, &wxMediaPlayerFrame::OnMediaLoaded, parentFrame,
         wxID_MEDIACTRL);

    Bind(wxEVT_BUTTON, &wxMediaPlayerFrame::OnPrev, parentFrame,
         wxID_BUTTONPREV);
    Bind(wxEVT_BUTTON, &wxMediaPlayerFrame::OnPlay, parentFrame,
         wxID_BUTTONPLAY);
    Bind(wxEVT_BUTTON, &wxMediaPlayerFrame::OnStop, parentFrame,
         wxID_BUTTONSTOP);
    Bind(wxEVT_BUTTON, &wxMediaPlayerFrame::OnNext, parentFrame,
         wxID_BUTTONNEXT);
    Bind(wxEVT_BUTTON, &wxMediaPlayerFrame::OnVolumeDown, parentFrame,
         wxID_BUTTONVD);
    Bind(wxEVT_BUTTON, &wxMediaPlayerFrame::OnVolumeUp, parentFrame,
         wxID_BUTTONVU);
}

void wxMediaPlayerNotebookPage::OnBeginSeek(wxScrollEvent &WXUNUSED(event))
{
    m_bIsBeingDragged = true;
}

void wxMediaPlayerNotebookPage::OnEndSeek(wxScrollEvent &WXUNUSED(event))
{
    if (m_mediactrl->Seek(
            m_slider->GetValue() * 1000) == wxInvalidOffset)
        wxMessageBox("Couldn't seek in movie!");

    m_bIsBeingDragged = false;
}

bool wxMediaPlayerNotebookPage::IsBeingDragged()
{
    return m_bIsBeingDragged;
}

void wxMediaPlayerNotebookPage::OnVolChange(wxScrollEvent &WXUNUSED(event))
{
    if (m_mediactrl->SetVolume(
            m_volSlider->GetValue() / 100.0) == false)
        wxMessageBox("Couldn't set volume!");
}

void wxMediaPlayerNotebookPage::OnPBChange(wxScrollEvent &WXUNUSED(event))
{
    if (m_mediactrl->SetPlaybackRate(
            m_pbSlider->GetValue() * .25) == false)
        wxMessageBox("Couldn't set playbackrate!");
}

void wxMediaPlayerNotebookPage::OnMediaPlay(wxMediaEvent &WXUNUSED(event))
{
    m_playlist->SetItem(m_nLastFileId, 0, ">");
}

void wxMediaPlayerNotebookPage::OnMediaPause(wxMediaEvent &WXUNUSED(event))
{
    m_playlist->SetItem(m_nLastFileId, 0, "||");
}

void wxMediaPlayerNotebookPage::OnMediaStop(wxMediaEvent &WXUNUSED(event))
{
    m_playlist->SetItem(m_nLastFileId, 0, "[]");
}

void wxMediaPlayerNotebookPage::OnMediaFinished(wxMediaEvent &WXUNUSED(event))
{
    if (m_bLoop)
    {
        if (!m_mediactrl->Play())
        {
            wxMessageBox("Couldn't loop movie!");
            m_playlist->SetItem(m_nLastFileId, 0, "E");
        }
        else
            ++m_nLoops;
    }
    else
    {
        m_playlist->SetItem(m_nLastFileId, 0, "[]");
    }
}
