#ifndef _RESOURCEPANEL_H_
#define _RESOURCEPANEL_H_

#include <wx/panel.h>

class wxBoxSizer;
class wxTreeCtrl;

class ResourcePanel : public wxPanel
{
public:
	ResourcePanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL, const wxString& name = "ResourcePanel");
	~ResourcePanel();

private:
	wxBoxSizer* mBoxSizer;
	wxTreeCtrl* mTreeControl;
};

#endif // _RESOURCEPANEL_H_