/*
-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------
*/
#include "CallTipManager.h"

#include "OgreDataStream.h"
#include "OgreString.h"

using Ogre::DataStream;
using Ogre::DataStreamPtr;
using Ogre::FileStreamDataStream;
using Ogre::String;

CallTipManager::CallTipManager()
{
}

CallTipManager::~CallTipManager()
{
}

void CallTipManager::load(wxString& path)
{
	// TODO: Clear tips list

	std::ifstream fp;
	fp.open(path, std::ios::in | std::ios::binary);
	if(fp)
	{
		DataStreamPtr stream(new FileStreamDataStream(path.c_str(), &fp, false));

		int index = -1;
		String line;
		String key;
		while(!stream->eof())
		{
			line = stream->getLine();

			// Ignore blank lines and comments (comment lines start with '#')
			if(line.length() > 0 && line.at(0) != '#')
			{
				if(line.at(0) == '[')
				{
					int endBrace = (int)line.find(']');
					if(endBrace != -1)
					{
						key = line.substr(1, endBrace - 1);
					}
				}
				else
				{
					if(mCallTips.find(key) != mCallTips.end())
						mCallTips[key] = mCallTips[key] + "\n" + line;
					else
						mCallTips[key] = line;
				}
			}
		}
	}
}

void CallTipManager::addTip(wxString& key, wxString& tip)
{
	mCallTips[key] = tip;
}

void CallTipManager::removeTip(wxString& key)
{
	CallTipMap::iterator it = mCallTips.find(key);
	if(it != mCallTips.end()) mCallTips.erase(it);
}

void CallTipManager::addTrigger(wxChar& trigger)
{
	mTriggers.push_back(trigger);
}

void CallTipManager::removeTrigger(wxChar& trigger)
{
	TriggerList::iterator it;
	for(it = mTriggers.begin(); it != mTriggers.end(); ++it)
	{
		if((*it) == trigger)
		{
			mTriggers.erase(it);
			return;
		}
	}
}

bool CallTipManager::isTrigger(wxChar& ch)
{
	TriggerList::iterator it;
	for(it = mTriggers.begin(); it != mTriggers.end(); ++it)
	{
		if((*it) == ch) return true;
	}

	return false;
}

wxString* CallTipManager::find(wxString& s)
{
	if(mCallTips.find(s) != mCallTips.end())
		return &mCallTips[s];

	return NULL;
}