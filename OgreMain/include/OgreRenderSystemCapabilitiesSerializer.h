/*
-----------------------------------------------------------------------------
This source file is part of OGRE
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
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef __RenderSystemCapabilitiesSerializer_H__
#define __RenderSystemCapabilitiesSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreStringVector.h"
#include "OgreDataStream.h"



namespace Ogre {


	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
	/** Class for serializing RenderSystemCapabilities to / from a .rendercaps script.*/
	class _OgreExport RenderSystemCapabilitiesSerializer : public RenderSysAlloc
    {

    public:
        /** default constructor*/
        RenderSystemCapabilitiesSerializer();
        /** default destructor*/
        virtual ~RenderSystemCapabilitiesSerializer() {};

        /** Writes a RenderSystemCapabilities object to a data stream */
        void writeScript(const RenderSystemCapabilities* caps, String name, String filename);

        /** Parses a RenderSystemCapabilities script file passed as a stream.
            Adds it to RenderSystemCapabilitiesManager::_addRenderSystemCapabilities
        */
        void parseScript(DataStreamPtr& stream);

    protected:


        enum CapabilityKeywordType {UNDEFINED_CAPABILITY_TYPE = 0, SET_STRING_METHOD, SET_INT_METHOD, SET_BOOL_METHOD, SET_REAL_METHOD,
                                SET_CAPABILITY_ENUM_BOOL, ADD_SHADER_PROFILE_STRING};
        // determines what keyword is what type of capability. For example:
        // "automipmap" and "pbuffer" are both activated with setCapability (passing RSC_AUTOMIPMAP and RSC_PBUFFER respectivelly)
        // while "max_num_multi_render_targets" is an integer and has it's own method: setMaxMultiNumRenderTargets
        // we need to know these types to automatically parse each capability
        typedef map<String, CapabilityKeywordType>::type KeywordTypeMap;
        KeywordTypeMap mKeywordTypeMap;

        typedef void (RenderSystemCapabilities::*SetStringMethod)(const String&);
        // maps capability keywords to setCapability(String& cap) style methods
        typedef map<String, SetStringMethod>::type SetStringMethodDispatchTable;
        SetStringMethodDispatchTable mSetStringMethodDispatchTable;

        // SET_INT_METHOD parsing tables
        typedef void (RenderSystemCapabilities::*SetIntMethod)(ushort);
        typedef map<String, SetIntMethod>::type SetIntMethodDispatchTable;
        SetIntMethodDispatchTable mSetIntMethodDispatchTable;

        // SET_BOOL_METHOD parsing tables
        typedef void (RenderSystemCapabilities::*SetBoolMethod)(bool);
        typedef map<String, SetBoolMethod>::type SetBoolMethodDispatchTable;
        SetBoolMethodDispatchTable mSetBoolMethodDispatchTable;

        // SET_REAL_METHOD parsing tables
        typedef void (RenderSystemCapabilities::*SetRealMethod)(Real);
        typedef map<String, SetRealMethod>::type SetRealMethodDispatchTable;
        SetRealMethodDispatchTable mSetRealMethodDispatchTable;

        typedef map<String, Capabilities>::type CapabilitiesMap;
        CapabilitiesMap mCapabilitiesMap;

        inline void addCapabilitiesMapping(String name, Capabilities cap)
        {
            mCapabilitiesMap.insert(CapabilitiesMap::value_type(name, cap));
        }


        // capabilities lines for parsing are collected along with their line numbers for debugging
        typedef vector<std::pair<String, int> >::type CapabilitiesLinesList;
        // the set of states that the parser can be in
        enum ParseAction {PARSE_HEADER, FIND_OPEN_BRACE, COLLECT_LINES};

        int mCurrentLineNumber;
        String* mCurrentLine;
        DataStreamPtr mCurrentStream;

        RenderSystemCapabilities* mCurrentCapabilities;

        inline void addKeywordType(String keyword, CapabilityKeywordType type)
        {
            mKeywordTypeMap.insert(KeywordTypeMap::value_type(keyword, type));
        }

        inline CapabilityKeywordType getKeywordType(const String& keyword) const
        {
						KeywordTypeMap::const_iterator it = mKeywordTypeMap.find(keyword);
            if(it != mKeywordTypeMap.end())
							 return (*it).second;
						else
						{
							 logParseError("Can't find the type for keyword: " + keyword);
							 return UNDEFINED_CAPABILITY_TYPE;
						}
        }

        inline void addSetStringMethod(String keyword, SetStringMethod method)
        {
            mSetStringMethodDispatchTable.insert(SetStringMethodDispatchTable::value_type(keyword, method));
        }

        inline void callSetStringMethod(String& keyword, String& val)
        {
            SetStringMethodDispatchTable::iterator methodIter = mSetStringMethodDispatchTable.find(keyword);
            if (methodIter != mSetStringMethodDispatchTable.end())
            {
						    SetStringMethod m = (*methodIter).second;
                (mCurrentCapabilities->*m)(val);
            }
            else
            {
                logParseError("undefined keyword: " + keyword);
            }
        }


        inline void addSetIntMethod(String keyword, SetIntMethod method)
        {
            mSetIntMethodDispatchTable.insert(SetIntMethodDispatchTable::value_type(keyword, method));
        }

        inline void callSetIntMethod(String& keyword, ushort val)
        {
            SetIntMethodDispatchTable::iterator methodIter = mSetIntMethodDispatchTable.find(keyword);
            if (methodIter != mSetIntMethodDispatchTable.end())
            {
						    SetIntMethod m = (*methodIter).second;
                (mCurrentCapabilities->*m)(val);
            }
            else
            {
                logParseError("undefined keyword: " + keyword);
            }  
        }


        inline void addSetBoolMethod(String keyword, SetBoolMethod method)
        {
            mSetBoolMethodDispatchTable.insert(SetBoolMethodDispatchTable::value_type(keyword, method));
        }

        inline void callSetBoolMethod(String& keyword, bool val)
        {
            SetBoolMethodDispatchTable::iterator methodIter = mSetBoolMethodDispatchTable.find(keyword);
            if (methodIter != mSetBoolMethodDispatchTable.end())
            {
						    SetBoolMethod m = (*methodIter).second;
                (mCurrentCapabilities->*m)(val);
            }
            else
            {
                logParseError("undefined keyword: " + keyword);
						}
        }


        inline void addSetRealMethod(String keyword, SetRealMethod method)
        {
            mSetRealMethodDispatchTable.insert(SetRealMethodDispatchTable::value_type(keyword, method));
        }

        inline void callSetRealMethod(String& keyword, Real val)
        {
            SetRealMethodDispatchTable::iterator methodIter = mSetRealMethodDispatchTable.find(keyword);
            if (methodIter != mSetRealMethodDispatchTable.end())
            {
						    SetRealMethod m = (*methodIter).second;
                (mCurrentCapabilities->*m)(val);
            }
            else
            {
                logParseError("undefined keyword: " + keyword);
						}
        }

        inline void addShaderProfile(String& val)
        {
            mCurrentCapabilities->addShaderProfile(val);
        }

        inline void setCapabilityEnumBool(String& name, bool val)
        {
            // check for errors
            if(mCapabilitiesMap.find(name) == mCapabilitiesMap.end())
            {
                logParseError("Undefined capability: " + name);
                return;
            }
            // only set true capabilities, we can't unset false
            if(val)
            {
                Capabilities cap = mCapabilitiesMap[name];
                mCurrentCapabilities->setCapability(cap);
            }
        }

        void initialiaseDispatchTables();

        void parseCapabilitiesLines(CapabilitiesLinesList& linesList);

        void logParseError(const String& error) const;

    };
	/** @} */
	/** @} */

}
#endif
