
#include "wxMediaPlayerApp.hpp"

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