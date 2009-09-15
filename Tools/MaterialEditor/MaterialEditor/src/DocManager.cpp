/*
-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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