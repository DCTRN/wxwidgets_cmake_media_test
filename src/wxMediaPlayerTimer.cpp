
#include "wxMediaPlayerNotebookPage.hpp"
#include "wxMediaPlayerTimer.hpp"
#include "wxMediaPlayerFrame.hpp"
#include "wx/mediactrl.h" // for wxMediaCtrl
#include "Ids.hpp"

#include "wx/filedlg.h" // for opening files from OpenFile

#include "wx/sizer.h" // for positioning controls/wxBoxSizer

#include "wx/textdlg.h" // for getting user text from OpenURL/Debug

#include "wx/dnd.h"      // drag and drop for the playlist
#include "wx/filename.h" // For wxFileName::GetName()
#include "wx/config.h"   // for native wxConfig
#include "wx/vector.h"

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
