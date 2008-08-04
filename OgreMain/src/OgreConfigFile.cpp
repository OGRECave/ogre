/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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
    void ConfigFile::load(const String& filename, const String& resourceGroup, 
        const String& separators, bool trimWhitespace)
    {
		loadFromResourceSystem(filename, resourceGroup, separators, trimWhitespace);
    }
	//-----------------------------------------------------------------------
	void ConfigFile::loadDirect(const String& filename, const String& separators, 
		bool trimWhitespace)
	{
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
                    /* Find the first seperator character and split the string there */
                    std::string::size_type separator_pos = line.find_first_of(separators, 0);
                    if (separator_pos != std::string::npos)
                    {
                        optName = line.substr(0, separator_pos);
                        /* Find the first non-seperator character following the name */
                        std::string::size_type nonseparator_pos = line.find_first_not_of(separators, separator_pos);
                        /* ... and extract the value */
                        /* Make sure we don't crash on an empty setting (it might be a valid value) */
                        optVal = (nonseparator_pos == std::string::npos) ? "" : line.substr(nonseparator_pos);
                        if (trimWhitespace)
                        {
                            StringUtil::trim(optVal);
                            StringUtil::trim(optName);
                        }
                        currentSettings->insert(std::multimap<String, String>::value_type(optName, optVal));
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
                return StringUtil::BLANK;
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
