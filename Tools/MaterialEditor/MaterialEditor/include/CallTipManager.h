/*
-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE
-------------------------------------------------------------------------
*/
#ifndef _CALLTIPMANAGER_H_
#define _CALLTIPMANAGER_H_

#include <list>
#include <map>

#include <wx/string.h>

using namespace std;

typedef map<wxString, wxString> CallTipMap;
typedef list<wxString> TriggerList;

class CallTipManager
{
public:
	CallTipManager();
	virtual ~CallTipManager();

	void load(wxString& path);

	void addTip(wxString& key, wxString& tip);
	void removeTip(wxString& key);

	void addTrigger(wxChar& trigger);
	void removeTrigger(wxChar& trigger);
	bool isTrigger(wxChar& ch);

	wxString* find(wxString& s);

protected:
	CallTipMap mCallTips;
	TriggerList mTriggers;
};

#endif // _CALLTIPMANAGER_H_


