///////////////////////////////////////////////////////////////////////////////
// Name:        mediaplayer.cpp
// Purpose:     wxMediaCtrl sample
// Author:      Ryan Norton
// Modified by:
// Created:     11/10/04
// Copyright:   (c) Ryan Norton
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// MediaPlayer
//
// This is a somewhat comprehensive example of how to use all the funtionality
// of the wxMediaCtrl class in wxWidgets.
//
// To use this sample, simply select Open File from the file menu,
// select the file you want to play - and MediaPlayer will play the file in
// the current notebook page, showing video if necessary.
//
// You can select one of the menu options, or move the slider around
// to manipulate what is playing.
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Known bugs with wxMediaCtrl:
//
// 1) Certain backends can't play the same media file at the same time (MCI,
//    Cocoa NSMovieView-Quicktime).
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// ============================================================================
// Definitions
// ============================================================================

// ----------------------------------------------------------------------------
// Pre-compiled header stuff
// ----------------------------------------------------------------------------

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

// ----------------------------------------------------------------------------
// Headers
// ----------------------------------------------------------------------------

#include "wx/mediactrl.h" // for wxMediaCtrl
#include "wx/filedlg.h"   // for opening files from OpenFile
#include "wx/slider.h"    // for a slider for seeking within media
#include "wx/sizer.h"     // for positioning controls/wxBoxSizer
#include "wx/timer.h"     // timer for updating status bar
#include "wx/textdlg.h"   // for getting user text from OpenURL/Debug
#include "wx/notebook.h"  // for wxNotebook and putting movies in pages
#include "wx/cmdline.h"   // for wxCmdLineParser (optional)
#include "wx/listctrl.h"  // for wxListCtrl
#include "wx/dnd.h"       // drag and drop for the playlist
#include "wx/filename.h"  // For wxFileName::GetName()
#include "wx/config.h"    // for native wxConfig
#include "wx/vector.h"

enum
{
    // Menu event IDs
    wxID_LOOP = 1,
    wxID_OPENFILESAMEPAGE,
    wxID_OPENFILENEWPAGE,
    wxID_OPENURLSAMEPAGE,
    wxID_OPENURLNEWPAGE,
    wxID_CLOSECURRENTPAGE,
    wxID_PLAY,
    wxID_PAUSE,
    wxID_NEXT,
    wxID_PREV,
    wxID_SELECTBACKEND,
    wxID_SHOWINTERFACE,
    //    wxID_STOP,   [built-in to wxWidgets]
    //    wxID_ABOUT,  [built-in to wxWidgets]
    //    wxID_EXIT,   [built-in to wxWidgets]
    // Control event IDs
    wxID_SLIDER,
    wxID_PBSLIDER,
    wxID_VOLSLIDER,
    wxID_NOTEBOOK,
    wxID_MEDIACTRL,
    wxID_BUTTONNEXT,
    wxID_BUTTONPREV,
    wxID_BUTTONSTOP,
    wxID_BUTTONPLAY,
    wxID_BUTTONVD,
    wxID_BUTTONVU,
    wxID_LISTCTRL,
    wxID_GAUGE
};

class wxMediaPlayerApp : public wxApp
{
public:
    virtual bool OnInit() override;

protected:
    class wxMediaPlayerFrame *m_frame;
};

class wxMediaPlayerFrame : public wxFrame
{
public:
    wxMediaPlayerFrame(const wxString &title);
    ~wxMediaPlayerFrame();

    void OnQuit(wxCommandEvent &event);
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
    void OnShowInterface(wxCommandEvent &event);

    void OnSelectBackend(wxCommandEvent &event);

    void OnKeyDown(wxKeyEvent &event);

    void AddToPlayList(const wxString &szString);

    void OnChangeSong(wxListEvent &event);

    void OnMediaLoaded(wxMediaEvent &event);

    void OnClose(wxCloseEvent &event);

private:
    void OpenFile(bool bNewPage);
    void OpenURL(bool bNewPage);
    void DoOpenFile(const wxString &path, bool bNewPage);
    void DoPlayFile(const wxString &path);

    class wxMediaPlayerTimer *m_timer;
    wxNotebook *m_notebook;

    friend class wxMediaPlayerApp;
    friend class wxMediaPlayerNotebookPage;
    friend class wxMediaPlayerTimer;
};

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

    wxMediaCtrl *m_mediactrl;                // Our media control
    class wxMediaPlayerListCtrl *m_playlist; // Our playlist
    wxSlider *m_slider;                      // The slider below our media control
    wxSlider *m_pbSlider;                    // Lower-left slider for adjusting speed
    wxSlider *m_volSlider;                   // Lower-right slider for adjusting volume
    int m_nLoops;                            // Number of times media has looped
    bool m_bLoop;                            // Whether we are looping or not
    bool m_bIsBeingDragged;                  // Whether the user is dragging the scroll bar
    wxMediaPlayerFrame *m_parentFrame;       // Main wxFrame of our sample
    wxButton *m_prevButton;                  // Go to previous file button
    wxButton *m_playButton;                  // Play/pause file button
    wxButton *m_stopButton;                  // Stop playing file button
    wxButton *m_nextButton;                  // Next file button
    wxButton *m_vdButton;                    // Volume down button
    wxButton *m_vuButton;                    // Volume up button
    wxGauge *m_gauge;                        // Gauge to keep in line with slider
};

class wxMediaPlayerTimer : public wxTimer
{
public:
    wxMediaPlayerTimer(wxMediaPlayerFrame *frame) { m_frame = frame; }

    void Notify() override;

    wxMediaPlayerFrame *m_frame;
};

class wxMediaPlayerListCtrl : public wxListCtrl
{
public:
    void AddToPlayList(const wxString &szString)
    {
        wxListItem kNewItem;
        kNewItem.SetAlign(wxLIST_FORMAT_LEFT);

        int nID = this->GetItemCount();
        kNewItem.SetId(nID);
        kNewItem.SetMask(wxLIST_MASK_DATA);
        kNewItem.SetData(new wxString(szString));

        this->InsertItem(kNewItem);
        this->SetItem(nID, 0, "*");
        this->SetItem(nID, 1, wxFileName(szString).GetName());

        if (nID % 2)
        {
            kNewItem.SetBackgroundColour(wxColour(192, 192, 192));
            this->SetItem(kNewItem);
        }
    }

    void GetSelectedItem(wxListItem &listitem)
    {
        listitem.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
        int nLast = -1, nLastSelected = -1;
        while ((nLast = this->GetNextItem(nLast,
                                          wxLIST_NEXT_ALL,
                                          wxLIST_STATE_SELECTED)) != -1)
        {
            listitem.SetId(nLast);
            this->GetItem(listitem);
            if ((listitem.GetState() & wxLIST_STATE_FOCUSED))
                break;
            nLastSelected = nLast;
        }
        if (nLast == -1 && nLastSelected == -1)
            return;
        listitem.SetId(nLastSelected == -1 ? nLast : nLastSelected);
        this->GetItem(listitem);
    }
};

class wxPlayListDropTarget : public wxFileDropTarget
{
public:
    wxPlayListDropTarget(wxMediaPlayerListCtrl &list) : m_list(list) {}
    ~wxPlayListDropTarget() {}
    virtual bool OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                             const wxArrayString &files) override
    {
        for (size_t i = 0; i < files.GetCount(); ++i)
        {
            m_list.AddToPlayList(files[i]);
        }
        return true;
    }
    wxMediaPlayerListCtrl &m_list;
};

const wxString wxGetMediaStateText(int nState)
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

wxIMPLEMENT_APP(wxMediaPlayerApp);

bool wxMediaPlayerApp::OnInit()
{
    if (!wxApp::OnInit())
        return false;

    SetAppName("wxMediaPlayer");

    wxMediaPlayerFrame *frame =
        new wxMediaPlayerFrame("MediaPlayer wxWidgets Sample");
    frame->Show(true);

    return true;
}

wxMediaPlayerFrame::wxMediaPlayerFrame(const wxString &title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1366, 768))
{
    wxMenu *fileMenu = new wxMenu;
    wxMenu *controlsMenu = new wxMenu;
    wxMenu *optionsMenu = new wxMenu;
    wxMenu *helpMenu = new wxMenu;
    wxMenu *debugMenu = new wxMenu;

    fileMenu->Append(wxID_OPENFILESAMEPAGE, "&Open File\tCtrl-Shift-O",
                     "Open a File in the current notebook page");
    fileMenu->Append(wxID_OPENFILENEWPAGE, "&Open File in a new page",
                     "Open a File in a new notebook page");
    fileMenu->Append(wxID_OPENURLSAMEPAGE, "&Open URL",
                     "Open a URL in the current notebook page");
    fileMenu->Append(wxID_OPENURLNEWPAGE, "&Open URL in a new page",
                     "Open a URL in a new notebook page");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_CLOSECURRENTPAGE, "&Close Current Page\tCtrl-C",
                     "Close current notebook page");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT,
                     "E&xit\tAlt-X",
                     "Quit this program");

    controlsMenu->Append(wxID_PLAY, "&Play/Pause\tCtrl-P", "Resume/Pause playback");
    controlsMenu->Append(wxID_STOP, "&Stop\tCtrl-S", "Stop playback");
    controlsMenu->AppendSeparator();
    controlsMenu->Append(wxID_PREV, "&Previous\tCtrl-B", "Go to previous track");
    controlsMenu->Append(wxID_NEXT, "&Next\tCtrl-N", "Skip to next track");

    optionsMenu->AppendCheckItem(wxID_LOOP,
                                 "&Loop\tCtrl-L",
                                 "Loop Selected Media");
    optionsMenu->AppendCheckItem(wxID_SHOWINTERFACE,
                                 "&Show Interface\tCtrl-I",
                                 "Show wxMediaCtrl native controls");

    debugMenu->Append(wxID_SELECTBACKEND,
                      "&Select Backend...\tCtrl-D",
                      "Select a backend manually");

    helpMenu->Append(wxID_ABOUT,
                     "&About\tF1",
                     "Show about dialog");

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(controlsMenu, "&Controls");
    menuBar->Append(optionsMenu, "&Options");
    menuBar->Append(debugMenu, "&Debug");
    menuBar->Append(helpMenu, "&Help");
    SetMenuBar(menuBar);

    m_notebook = new wxNotebook(this, wxID_NOTEBOOK);

    CreateStatusBar(1);

    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnQuit, this,
         wxID_EXIT);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnAbout, this,
         wxID_ABOUT);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnLoop, this,
         wxID_LOOP);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnShowInterface, this,
         wxID_SHOWINTERFACE);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnOpenFileNewPage, this,
         wxID_OPENFILENEWPAGE);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnOpenFileSamePage, this,
         wxID_OPENFILESAMEPAGE);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnOpenURLNewPage, this,
         wxID_OPENURLNEWPAGE);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnOpenURLSamePage, this,
         wxID_OPENURLSAMEPAGE);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnCloseCurrentPage, this,
         wxID_CLOSECURRENTPAGE);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnPlay, this,
         wxID_PLAY);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnStop, this,
         wxID_STOP);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnNext, this,
         wxID_NEXT);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnPrev, this,
         wxID_PREV);
    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnSelectBackend, this,
         wxID_SELECTBACKEND);

    wxTheApp->Bind(wxEVT_KEY_DOWN, &wxMediaPlayerFrame::OnKeyDown, this);

    Bind(wxEVT_CLOSE_WINDOW, &wxMediaPlayerFrame::OnClose, this);

    wxMediaPlayerNotebookPage *page =
        new wxMediaPlayerNotebookPage(this, m_notebook);
    m_notebook->AddPage(page,
                        "",
                        true);

    wxConfig conf;
    wxString key, outstring;
    for (int i = 0;; ++i)
    {
        key.clear();
        key << i;
        if (!conf.Read(key, &outstring))
            break;
        page->m_playlist->AddToPlayList(outstring);
    }
}

m_timer = new wxMediaPlayerTimer(this);
m_timer->Start(500);
}

wxMediaPlayerFrame::~wxMediaPlayerFrame()
{
    delete m_timer;

    wxMediaPlayerListCtrl *playlist =
        ((wxMediaPlayerNotebookPage *)m_notebook->GetPage(0))->m_playlist;

    wxConfig conf;
    conf.DeleteAll();

    for (int i = 0; i < playlist->GetItemCount(); ++i)
    {
        wxString *pData = (wxString *)playlist->GetItemData(i);
        wxString s;
        s << i;
        conf.Write(s, *(pData));
        delete pData;
    }
}

void wxMediaPlayerFrame::OnClose(wxCloseEvent &event)
{
    event.Skip();
}

void wxMediaPlayerFrame::AddToPlayList(const wxString &szString)
{
    wxMediaPlayerNotebookPage *currentpage =
        ((wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage());

    currentpage->m_playlist->AddToPlayList(szString);
}

void wxMediaPlayerFrame::OnQuit(wxCommandEvent &WXUNUSED(event))
{
    Close(true);
}

void wxMediaPlayerFrame::OnAbout(wxCommandEvent &WXUNUSED(event))
{
    wxString msg;
    msg.Printf("This is a test of wxMediaCtrl.\n\n"

               "Instructions:\n"

               "The top slider shows the current the current position, "
               "which you can change by dragging and releasing it.\n"

               "The gauge (progress bar) shows the progress in "
               "downloading data of the current file - it may always be "
               "empty due to lack of support from the current backend.\n"

               "The lower-left slider controls the volume and the lower-"
               "right slider controls the playback rate/speed of the "
               "media\n\n"

               "Currently using: %s",
               wxVERSION_STRING);

    wxMessageBox(msg, "About wxMediaCtrl test",
                 wxOK | wxICON_INFORMATION, this);
}

void wxMediaPlayerFrame::OnLoop(wxCommandEvent &WXUNUSED(event))
{
    wxMediaPlayerNotebookPage *currentpage =
        ((wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage());

    currentpage->m_bLoop = !currentpage->m_bLoop;
}

void wxMediaPlayerFrame::OnShowInterface(wxCommandEvent &event)
{
    wxMediaPlayerNotebookPage *currentpage =
        ((wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage());

    if (!currentpage->m_mediactrl->ShowPlayerControls(event.IsChecked() ? wxMEDIACTRLPLAYERCONTROLS_DEFAULT : wxMEDIACTRLPLAYERCONTROLS_NONE))
    {
        wxMenuItem *pSIItem = GetMenuBar()->FindItem(wxID_SHOWINTERFACE);
        wxASSERT(pSIItem);
        pSIItem->Check(!event.IsChecked());

        if (event.IsChecked())
            wxMessageBox("Could not show player controls");
        else
            wxMessageBox("Could not hide player controls");
    }
}

void wxMediaPlayerFrame::OnOpenFileSamePage(wxCommandEvent &WXUNUSED(event))
{
    OpenFile(false);
}

void wxMediaPlayerFrame::OnOpenFileNewPage(wxCommandEvent &WXUNUSED(event))
{
    OpenFile(true);
}

void wxMediaPlayerFrame::OpenFile(bool bNewPage)
{
    wxFileDialog fd(this);

    if (fd.ShowModal() == wxID_OK)
    {
        DoOpenFile(fd.GetPath(), bNewPage);
    }
}

void wxMediaPlayerFrame::DoOpenFile(const wxString &path, bool bNewPage)
{
    if (bNewPage)
    {
        m_notebook->AddPage(
            new wxMediaPlayerNotebookPage(this, m_notebook),
            path,
            true);
    }

    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();

    if (currentpage->m_nLastFileId != -1)
        currentpage->m_playlist->SetItemState(currentpage->m_nLastFileId,
                                              0, wxLIST_STATE_SELECTED);

    wxListItem newlistitem;
    newlistitem.SetAlign(wxLIST_FORMAT_LEFT);

    int nID;

    newlistitem.SetId(nID = currentpage->m_playlist->GetItemCount());
    newlistitem.SetMask(wxLIST_MASK_DATA | wxLIST_MASK_STATE);
    newlistitem.SetState(wxLIST_STATE_SELECTED);
    newlistitem.SetData(new wxString(path));

    currentpage->m_playlist->InsertItem(newlistitem);
    currentpage->m_playlist->SetItem(nID, 0, "*");
    currentpage->m_playlist->SetItem(nID, 1, wxFileName(path).GetName());

    if (nID % 2)
    {
        newlistitem.SetBackgroundColour(wxColour(192, 192, 192));
        currentpage->m_playlist->SetItem(newlistitem);
    }

    DoPlayFile(path);
}

void wxMediaPlayerFrame::DoPlayFile(const wxString &path)
{
    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();

    wxListItem listitem;
    currentpage->m_playlist->GetSelectedItem(listitem);

    if ((listitem.GetData() &&
         currentpage->m_nLastFileId == listitem.GetId() &&
         currentpage->m_szFile.compare(path) == 0) ||
        (!listitem.GetData() &&
         currentpage->m_nLastFileId != -1 &&
         currentpage->m_szFile.compare(path) == 0))
    {
        if (currentpage->m_mediactrl->GetState() == wxMEDIASTATE_PLAYING)
        {
            if (!currentpage->m_mediactrl->Pause())
                wxMessageBox("Couldn't pause movie!");
        }
        else
        {
            if (!currentpage->m_mediactrl->Play())
                wxMessageBox("Couldn't play movie!");
        }
    }
    else
    {
        int nNewId = listitem.GetData() ? listitem.GetId() : currentpage->m_playlist->GetItemCount() - 1;
        m_notebook->SetPageText(m_notebook->GetSelection(),
                                wxFileName(path).GetName());

        if (currentpage->m_nLastFileId != -1)
            currentpage->m_playlist->SetItem(
                currentpage->m_nLastFileId, 0, "*");

        wxURI uripath(path);
        if (uripath.IsReference())
        {
            if (!currentpage->m_mediactrl->Load(path))
            {
                wxMessageBox("Couldn't load file!");
                currentpage->m_playlist->SetItem(nNewId, 0, "E");
            }
            else
            {
                currentpage->m_playlist->SetItem(nNewId, 0, "O");
            }
        }
        else
        {
            if (!currentpage->m_mediactrl->Load(uripath))
            {
                wxMessageBox("Couldn't load URL!");
                currentpage->m_playlist->SetItem(nNewId, 0, "E");
            }
            else
            {
                currentpage->m_playlist->SetItem(nNewId, 0, "O");
            }
        }

        currentpage->m_nLastFileId = nNewId;
        currentpage->m_szFile = path;
        currentpage->m_playlist->SetItem(currentpage->m_nLastFileId,
                                         1, wxFileName(path).GetName());
        currentpage->m_playlist->SetItem(currentpage->m_nLastFileId,
                                         2, "");
    }
}

void wxMediaPlayerFrame::OnMediaLoaded(wxMediaEvent &WXUNUSED(evt))
{
    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();

    if (!currentpage->m_mediactrl->Play())
    {
        wxMessageBox("Couldn't play movie!");
        currentpage->m_playlist->SetItem(currentpage->m_nLastFileId, 0, "E");
    }
    else
    {
        currentpage->m_playlist->SetItem(currentpage->m_nLastFileId, 0, ">");
    }
}

void wxMediaPlayerFrame::OnSelectBackend(wxCommandEvent &WXUNUSED(evt))
{
    wxString sBackend = wxGetTextFromUser("Enter backend to use");

    if (sBackend.empty() == false)
    {
        int sel = m_notebook->GetSelection();

        if (sel != wxNOT_FOUND)
        {
            m_notebook->DeletePage(sel);
        }

        m_notebook->AddPage(new wxMediaPlayerNotebookPage(this, m_notebook,
                                                          sBackend),
                            "", true);

        DoOpenFile(
            ((wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage())->m_szFile,
            false);
    }
}

void wxMediaPlayerFrame::OnOpenURLSamePage(wxCommandEvent &WXUNUSED(event))
{
    OpenURL(false);
}

void wxMediaPlayerFrame::OnOpenURLNewPage(wxCommandEvent &WXUNUSED(event))
{
    OpenURL(true);
}

void wxMediaPlayerFrame::OpenURL(bool bNewPage)
{
    wxString sUrl = wxGetTextFromUser(
        "Enter the URL that has the movie to play");

    if (sUrl.empty() == false) // could have been cancelled by user
    {
        DoOpenFile(sUrl, bNewPage);
    }
}

void wxMediaPlayerFrame::OnCloseCurrentPage(wxCommandEvent &WXUNUSED(event))
{
    if (m_notebook->GetPageCount() > 1)
    {
        int sel = m_notebook->GetSelection();

        if (sel != wxNOT_FOUND)
        {
            m_notebook->DeletePage(sel);
        }
    }
    else
    {
        wxMessageBox("Cannot close main page");
    }
}

void wxMediaPlayerFrame::OnPlay(wxCommandEvent &WXUNUSED(event))
{
    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();

    wxListItem listitem;
    currentpage->m_playlist->GetSelectedItem(listitem);
    if (!listitem.GetData())
    {
        int nLast = -1;
        if ((nLast = currentpage->m_playlist->GetNextItem(nLast,
                                                          wxLIST_NEXT_ALL,
                                                          wxLIST_STATE_DONTCARE)) == -1)
        {
            wxMessageBox("No items in playlist!");
        }
        else
        {
            listitem.SetId(nLast);
            currentpage->m_playlist->GetItem(listitem);
            listitem.SetMask(listitem.GetMask() | wxLIST_MASK_STATE);
            listitem.SetState(listitem.GetState() | wxLIST_STATE_SELECTED);
            currentpage->m_playlist->SetItem(listitem);
            wxASSERT(listitem.GetData());
            DoPlayFile((*((wxString *)listitem.GetData())));
        }
    }
    else
    {
        wxASSERT(listitem.GetData());
        DoPlayFile((*((wxString *)listitem.GetData())));
    }
}

void wxMediaPlayerFrame::OnKeyDown(wxKeyEvent &event)
{
    if (event.GetKeyCode() == WXK_BACK)
    {
        wxMediaPlayerNotebookPage *currentpage =
            (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();
        while (true)
        {
            wxInt32 nSelectedItem = currentpage->m_playlist->GetNextItem(
                -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (nSelectedItem == -1)
                break;

            wxListItem listitem;
            listitem.SetId(nSelectedItem);
            currentpage->m_playlist->GetItem(listitem);
            delete (wxString *)listitem.GetData();

            currentpage->m_playlist->DeleteItem(nSelectedItem);
        }
    }

    if (event.GetEventObject() != this)
        event.Skip();
}

void wxMediaPlayerFrame::OnStop(wxCommandEvent &WXUNUSED(evt))
{
    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();

    if (!currentpage->m_mediactrl->Stop())
        wxMessageBox("Couldn't stop movie!");
    else
        currentpage->m_playlist->SetItem(
            currentpage->m_nLastFileId, 0, "[]");
}

void wxMediaPlayerFrame::OnChangeSong(wxListEvent &WXUNUSED(evt))
{
    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();

    wxListItem listitem;
    currentpage->m_playlist->GetSelectedItem(listitem);
    if (listitem.GetData())
        DoPlayFile((*((wxString *)listitem.GetData())));
    else
        wxMessageBox("No selected item!");
}

void wxMediaPlayerFrame::OnPrev(wxCommandEvent &WXUNUSED(event))
{
    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();

    if (currentpage->m_playlist->GetItemCount() == 0)
        return;

    wxInt32 nLastSelectedItem = -1;
    while (true)
    {
        wxInt32 nSelectedItem = currentpage->m_playlist->GetNextItem(nLastSelectedItem,
                                                                     wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (nSelectedItem == -1)
            break;
        nLastSelectedItem = nSelectedItem;
        currentpage->m_playlist->SetItemState(nSelectedItem, 0, wxLIST_STATE_SELECTED);
    }

    if (nLastSelectedItem == -1)
    {
        if (currentpage->m_nLastFileId == 0)
            nLastSelectedItem = currentpage->m_playlist->GetItemCount() - 1;
        else
            nLastSelectedItem = currentpage->m_nLastFileId - 1;
    }
    else if (nLastSelectedItem == 0)
        nLastSelectedItem = currentpage->m_playlist->GetItemCount() - 1;
    else
        nLastSelectedItem -= 1;

    if (nLastSelectedItem == currentpage->m_nLastFileId)
        return;

    wxListItem listitem;
    listitem.SetId(nLastSelectedItem);
    listitem.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
    currentpage->m_playlist->GetItem(listitem);
    listitem.SetMask(listitem.GetMask() | wxLIST_MASK_STATE);
    listitem.SetState(listitem.GetState() | wxLIST_STATE_SELECTED);
    currentpage->m_playlist->SetItem(listitem);

    wxASSERT(listitem.GetData());
    DoPlayFile((*((wxString *)listitem.GetData())));
}

void wxMediaPlayerFrame::OnNext(wxCommandEvent &WXUNUSED(event))
{
    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();

    if (currentpage->m_playlist->GetItemCount() == 0)
        return;

    wxInt32 nLastSelectedItem = -1;
    while (true)
    {
        wxInt32 nSelectedItem = currentpage->m_playlist->GetNextItem(nLastSelectedItem,
                                                                     wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if (nSelectedItem == -1)
            break;
        nLastSelectedItem = nSelectedItem;
        currentpage->m_playlist->SetItemState(nSelectedItem, 0, wxLIST_STATE_SELECTED);
    }

    if (nLastSelectedItem == -1)
    {
        if (currentpage->m_nLastFileId == currentpage->m_playlist->GetItemCount() - 1)
            nLastSelectedItem = 0;
        else
            nLastSelectedItem = currentpage->m_nLastFileId + 1;
    }
    else if (nLastSelectedItem == currentpage->m_playlist->GetItemCount() - 1)
        nLastSelectedItem = 0;
    else
        nLastSelectedItem += 1;

    if (nLastSelectedItem == currentpage->m_nLastFileId)
        return;

    wxListItem listitem;
    listitem.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
    listitem.SetId(nLastSelectedItem);
    currentpage->m_playlist->GetItem(listitem);
    listitem.SetMask(listitem.GetMask() | wxLIST_MASK_STATE);
    listitem.SetState(listitem.GetState() | wxLIST_STATE_SELECTED);
    currentpage->m_playlist->SetItem(listitem);

    wxASSERT(listitem.GetData());
    DoPlayFile((*((wxString *)listitem.GetData())));
}

void wxMediaPlayerFrame::OnVolumeDown(wxCommandEvent &WXUNUSED(event))
{
    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();

    double dVolume = currentpage->m_mediactrl->GetVolume();
    currentpage->m_mediactrl->SetVolume(dVolume < 0.05 ? 0.0 : dVolume - .05);
}

void wxMediaPlayerFrame::OnVolumeUp(wxCommandEvent &WXUNUSED(event))
{
    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage();

    double dVolume = currentpage->m_mediactrl->GetVolume();
    currentpage->m_mediactrl->SetVolume(dVolume > 0.95 ? 1.0 : dVolume + .05);
}

void wxMediaPlayerTimer::Notify()
{
    wxMediaPlayerNotebookPage *currentpage =
        (wxMediaPlayerNotebookPage *)m_frame->m_notebook->GetCurrentPage();
    wxMediaCtrl *currentMediaCtrl = currentpage->m_mediactrl;

    wxLongLong llLength = currentpage->m_mediactrl->Length();
    int nMinutes = (int)(llLength / 60000).GetValue();
    int nSeconds = (int)((llLength % 60000) / 1000).GetValue();

    wxString sDuration;
    sDuration.Printf("%2i:%02i", nMinutes, nSeconds);

    wxLongLong llTell = currentpage->m_mediactrl->Tell();
    nMinutes = (int)(llTell / 60000).GetValue();
    nSeconds = (int)((llTell % 60000) / 1000).GetValue();

    wxString sPosition;
    sPosition.Printf("%2i:%02i", nMinutes, nSeconds);

    if (currentpage->m_nLastFileId >= 0)
        currentpage->m_playlist->SetItem(
            currentpage->m_nLastFileId, 2, sDuration);

    currentpage->m_slider->SetRange(0, (int)(llLength / 1000).GetValue());
    currentpage->m_gauge->SetRange(100);

    if (currentpage->IsBeingDragged() == false)
        currentpage->m_slider->SetValue((long)(llTell / 1000).GetValue());

    wxLongLong llDownloadProgress =
        currentpage->m_mediactrl->GetDownloadProgress();
    wxLongLong llDownloadTotal =
        currentpage->m_mediactrl->GetDownloadTotal();

    if (llDownloadTotal.GetValue() != 0)
    {
        currentpage->m_gauge->SetValue(
            (int)((llDownloadProgress * 100) / llDownloadTotal).GetValue());
    }

    wxSize videoSize = currentMediaCtrl->GetBestSize();

    m_frame->SetStatusText(wxString::Format(
        "Size(x,y):%i,%i "
        "Position:%s/%s Speed:%1.1fx "
        "State:%s Loops:%i D/T:[%i]/[%i] V:%i%%",
        videoSize.x,
        videoSize.y,
        sPosition,
        sDuration,
        currentMediaCtrl->GetPlaybackRate(),
        wxGetMediaStateText(currentpage->m_mediactrl->GetState()),
        currentpage->m_nLoops,
        (int)llDownloadProgress.GetValue(),
        (int)llDownloadTotal.GetValue(),
        (int)(currentpage->m_mediactrl->GetVolume() * 100)));
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
                                   wxST_NO_AUTORESIZE,
                                   szBackend);
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
