/*
-----------------------------------------------------------------------------
This source file is part of OGRE
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
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "Threading/OgreThreadHeaders.h"

namespace Ogre {
    OGRE_STATIC_MUTEX( msDictionaryMutex );
    /// Dictionary of parameters
    static ParamDictionaryMap msDictionary;

    ParamDictionary::ParamDictionary() {}
    ParamDictionary::~ParamDictionary() {}

    ParamCommand* ParamDictionary::getParamCommand(const String& name)
    {
        ParamCommandMap::iterator i = mParamCommands.find(name);
        if (i != mParamCommands.end())
        {
            return i->second;
        }
        else
        {
            return 0;
        }
    }

    const ParamCommand* ParamDictionary::getParamCommand(const String& name) const
    {
        ParamCommandMap::const_iterator i = mParamCommands.find(name);
        if (i != mParamCommands.end())
        {
            return i->second;
        }
        else
        {
            return 0;
        }
    }

    void ParamDictionary::addParameter(const ParameterDef& paramDef, ParamCommand* paramCmd)
    {
        mParamDefs.push_back(paramDef);
        mParamCommands[paramDef.name] = paramCmd;
    }

    bool StringInterface::createParamDictionary(const String& className)
    {
        OGRE_LOCK_MUTEX( msDictionaryMutex );

        ParamDictionaryMap::iterator it = msDictionary.find(className);

        if ( it == msDictionary.end() )
        {
            mParamDict = &msDictionary.insert( std::make_pair( className, ParamDictionary() ) ).first->second;
            mParamDictName = className;
            return true;
        }
        else
        {
            mParamDict = &it->second;
            mParamDictName = className;
            return false;
        }
    }

    const ParameterList& StringInterface::getParameters(void) const
    {
        static ParameterList emptyList;

        const ParamDictionary* dict = getParamDictionary();
        if (dict)
            return dict->getParameters();
        else
            return emptyList;

    }

    String StringInterface::getParameter(const String& name) const
    {
        // Get dictionary
        const ParamDictionary* dict = getParamDictionary();

        if (dict)
        {
            // Look up command object
            const ParamCommand* cmd = dict->getParamCommand(name);

            if (cmd)
            {
                return cmd->doGet(this);
            }
        }

        // Fallback
        return "";
    }

    bool StringInterface::setParameter(const String& name, const String& value)
    {
        // Get dictionary
        ParamDictionary* dict = getParamDictionary();

        if (dict)
        {
            // Look up command object
            ParamCommand* cmd = dict->getParamCommand(name);
            if (cmd)
            {
                cmd->doSet(this, value);
                return true;
            }
        }
        // Fallback
        return false;
    }
    //-----------------------------------------------------------------------
    void StringInterface::setParameterList(const NameValuePairList& paramList)
    {
        NameValuePairList::const_iterator i, iend;
        iend = paramList.end();
        for (i = paramList.begin(); i != iend; ++i)
        {
            setParameter(i->first, i->second);
        }
    }

    void StringInterface::copyParametersTo(StringInterface* dest) const
    {
        // Get dictionary
        const ParamDictionary* dict = getParamDictionary();

        if (dict)
        {
            // Iterate through own parameters
            ParameterList::const_iterator i;

            for (i = dict->mParamDefs.begin();
            i != dict->mParamDefs.end(); ++i)
            {
                dest->setParameter(i->name, getParameter(i->name));
            }
        }
    }

    //-----------------------------------------------------------------------
    void StringInterface::cleanupDictionary ()
    {
            OGRE_LOCK_MUTEX( msDictionaryMutex );

        msDictionary.clear();
    }
}
