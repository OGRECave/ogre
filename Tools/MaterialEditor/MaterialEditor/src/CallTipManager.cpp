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