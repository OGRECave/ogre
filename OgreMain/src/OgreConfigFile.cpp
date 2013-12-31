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
#include "OgreConfigFile.h"
#include "OgreResourceGroupManager.h"

#include "OgreException.h"

#include <iostream>

namespace Ogre {

    //-----------------------------------------------------------------------
    ConfigFile::ConfigFile()
    {
    }
    //-----------------------------------------------------------------------
    ConfigFile::~ConfigFile()
    {
        SettingsBySection::iterator seci, secend;
        secend = mSettings.end();
        for (seci = mSettings.begin(); seci != secend; ++seci)
        {
            OGRE_DELETE_T(seci->second, SettingsMultiMap, MEMCATEGORY_GENERAL);
        }
    }
    //-----------------------------------------------------------------------
    void ConfigFile::clear(void)
    {
        for (SettingsBySection::iterator seci = mSettings.begin(); 
            seci != mSettings.end(); ++seci)
        {
             OGRE_DELETE_T(seci->second, SettingsMultiMap, MEMCATEGORY_GENERAL);
        }
        mSettings.clear();
    }
    //-----------------------------------------------------------------------
    void ConfigFile::load(const String& filename, const String& separators, bool trimWhitespace)
    {
        loadDirect(filename, separators, trimWhitespace);
    }
	//-----------------------------------------------------------------------
	void ConfigFile::loadDirect(const String& filename, const String& separators, 
		bool trimWhitespace)
	{
#if OGRE_PLATFORM == OGRE_PLATFORM_NACL
        OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "loadDirect is not supported on NaCl - tried to open: " + filename,
            "ConfigFile::loadDirect");
#endif

        /* Open the configuration file */
		std::ifstream fp;
        // Always open in binary mode
		fp.open(filename.c_str(), std::ios::in | std::ios::binary);
		if(!fp)
			OGRE_EXCEPT(
			Exception::ERR_FILE_NOT_FOUND, "'" + filename + "' file not found!", "ConfigFile::load" );

		// Wrap as a stream
		DataStreamPtr stream(OGRE_NEW FileStreamDataStream(filename, &fp, false));

		load(stream, separators, trimWhitespace);

	}
	//-----------------------------------------------------------------------
	void ConfigFile::loadFromResourceSystem(const String& filename, 
		const String& resourceGroup, const String& separators, bool trimWhitespace)
	{
		DataStreamPtr stream = 
			ResourceGroupManager::getSingleton().openResource(filename, resourceGroup);
		load(stream, separators, trimWhitespace);
	}
    //-----------------------------------------------------------------------
    void ConfigFile::load(const DataStreamPtr& stream, const String& separators, 
        bool trimWhitespace)
    {
        /* Clear current settings map */
        clear();

        String currentSection = StringUtil::BLANK;
        SettingsMultiMap* currentSettings = OGRE_NEW_T(SettingsMultiMap, MEMCATEGORY_GENERAL)();
        mSettings[currentSection] = currentSettings;


        /* Process the file line for line */
        String line, optName, optVal;
        while (!stream->eof())
        {
            line = stream->getLine();
            /* Ignore comments & blanks */
            if (line.length() > 0 && line.at(0) != '#' && line.at(0) != '@')
            {
                if (line.at(0) == '[' && line.at(line.length()-1) == ']')
                {
                    // Section
                    currentSection = line.substr(1, line.length() - 2);
					SettingsBySection::const_iterator seci = mSettings.find(currentSection);
					if (seci == mSettings.end())
					{
						currentSettings = OGRE_NEW_T(SettingsMultiMap, MEMCATEGORY_GENERAL)();
						mSettings[currentSection] = currentSettings;
					}
					else
					{
						currentSettings = seci->second;
					} 
                }
                else
                {
                    /* Find the first separator character and split the string there */
					Ogre::String::size_type separator_pos = line.find_first_of(separators, 0);
                    if (separator_pos != Ogre::String::npos)
                    {
                        optName = line.substr(0, separator_pos);
                        /* Find the first non-separator character following the name */
                        Ogre::String::size_type nonseparator_pos = line.find_first_not_of(separators, separator_pos);
                        /* ... and extract the value */
                        /* Make sure we don't crash on an empty setting (it might be a valid value) */
                        optVal = (nonseparator_pos == Ogre::String::npos) ? "" : line.substr(nonseparator_pos);
                        if (trimWhitespace)
                        {
                            StringUtil::trim(optVal);
                            StringUtil::trim(optName);
                        }
                        currentSettings->insert(SettingsMultiMap::value_type(optName, optVal));
                    }
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    String ConfigFile::getSetting(const String& key, const String& section, const String& defaultValue) const
    {
        
        SettingsBySection::const_iterator seci = mSettings.find(section);
        if (seci == mSettings.end())
        {
            return defaultValue;
        }
        else
        {
            SettingsMultiMap::const_iterator i = seci->second->find(key);
            if (i == seci->second->end())
            {
                return defaultValue;
            }
            else
            {
                return i->second;
            }
        }
    }
    //-----------------------------------------------------------------------
    StringVector ConfigFile::getMultiSetting(const String& key, const String& section) const
    {
        StringVector ret;


        SettingsBySection::const_iterator seci = mSettings.find(section);
        if (seci != mSettings.end())
        {
            SettingsMultiMap::const_iterator i;

            i = seci->second->find(key);
            // Iterate over matches
            while (i != seci->second->end() && i->first == key)
            {
                ret.push_back(i->second);
                ++i;
            }
        }
        return ret;


    }
    //-----------------------------------------------------------------------
    ConfigFile::SettingsIterator ConfigFile::getSettingsIterator(const String& section)
    {
        SettingsBySection::const_iterator seci = mSettings.find(section);
        if (seci == mSettings.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot find section " + section, 
                "ConfigFile::getSettingsIterator");
        }
        else
        {
            return SettingsIterator(seci->second->begin(), seci->second->end());
        }
    }
    //-----------------------------------------------------------------------
    ConfigFile::SectionIterator ConfigFile::getSectionIterator(void)
    {
        return SectionIterator(mSettings.begin(), mSettings.end());
    }

}
