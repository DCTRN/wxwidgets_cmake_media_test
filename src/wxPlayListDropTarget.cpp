
#include "wxPlayListDropTarget.hpp"

bool wxPlayListDropTarget::OnDropFiles(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                                       const wxArrayString &files)
{
    for (size_t i = 0; i < files.GetCount(); ++i)
    {
        m_list.AddToPlayList(files[i]);
    }
    return true;
}
