
#include "wxMediaPlayerListCtrl.hpp"
#include "Ids.hpp"

#include "wx/filename.h" // For wxFileName::GetName()
#include "wx/vector.h"

void wxMediaPlayerListCtrl::AddToPlayList(const wxString &szString)
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

void wxMediaPlayerListCtrl::GetSelectedItem(wxListItem &listitem)
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
