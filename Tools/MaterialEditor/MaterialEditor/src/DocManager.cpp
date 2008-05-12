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
#include "DocManager.h"

#include "OgreDataStream.h"
#include "OgreString.h"

using Ogre::DataStream;
using Ogre::DataStreamPtr;
using Ogre::FileStreamDataStream;
using Ogre::String;

DocManager::DocManager()
{
}

DocManager::~DocManager()
{
}

void DocManager::load(wxString& path)
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

			// Ignore comments
			if(line.length() > 0 && line.at(0) == '#') continue;

			if(line.length() > 0 && line.at(0) == '[')
			{
				int endBrace = (int)line.find(']');
				if(endBrace != -1)
				{
					key = line.substr(1, endBrace - 1);
				}
			}
			else
			{
				if(mDocs.find(key) != mDocs.end())
					mDocs[key] = mDocs[key] + "\n" + line;
				else
					mDocs[key] = line;
			}
		}
	}
}

void DocManager::addDoc(wxString& key, wxString& doc)
{
	mDocs[key] = doc;
}

void DocManager::removeDoc(wxString& key)
{
	DocMap::iterator it = mDocs.find(key);
	if(it != mDocs.end()) mDocs.erase(it);
}

wxString* DocManager::find(wxString& s)
{
	if(mDocs.find(s) != mDocs.end())
		return &mDocs[s];

	return NULL;
}