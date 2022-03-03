#include "wx/filedlg.h"  // for opening files from OpenFile
#include "wx/sizer.h"    // for positioning controls/wxBoxSizer
#include "wx/timer.h"    // timer for updating status bar
#include "wx/textdlg.h"  // for getting user text from OpenURL/Debug
#include "wx/dnd.h"      // drag and drop for the playlist
#include "wx/filename.h" // For wxFileName::GetName()
#include "wx/config.h"   // for native wxConfig
#include "wx/vector.h"

#include "wxMediaPlayerFrame.hpp"
#include "wxMediaPlayerListCtrl.hpp"
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

wxMediaPlayerFrame::wxMediaPlayerFrame(const wxString &title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1366, 768))
{
    wxMenu *fileMenu = new wxMenu;
    wxMenu *helpMenu = new wxMenu;

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

    helpMenu->Append(wxID_ABOUT,
                     "&About\tF1",
                     "Show about dialog");

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "&File");
    menuBar->Append(helpMenu, "&Help");
    SetMenuBar(menuBar);

    m_notebook = new wxNotebook(this, wxID_NOTEBOOK);

    CreateStatusBar(1);

    Bind(wxEVT_MENU, &wxMediaPlayerFrame::OnAbout, this,
         wxID_ABOUT);
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

    wxTheApp->Bind(wxEVT_KEY_DOWN, &wxMediaPlayerFrame::OnKeyDown, this);

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

void wxMediaPlayerFrame::AddToPlayList(const wxString &szString)
{
    wxMediaPlayerNotebookPage *currentpage =
        ((wxMediaPlayerNotebookPage *)m_notebook->GetCurrentPage());

    currentpage->m_playlist->AddToPlayList(szString);
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
