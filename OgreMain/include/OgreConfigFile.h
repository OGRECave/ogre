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
#ifndef __ConfigFile_H__
#define __ConfigFile_H__

#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreStringVector.h"
#include "OgreIteratorWrappers.h"
#include "OgreDataStream.h"

namespace Ogre {

    /** Class for quickly loading settings from a text file.
        @remarks
            This class is designed to quickly parse a simple file containing
            key/value pairs, mainly for use in configuration settings.
        @par
            This is a very simplified approach, no multiple values per key
            are allowed, no grouping or context is being kept etc.
        @par
            By default the key/values pairs are tokenised based on a
            separator of Tab, the colon (:) or equals (=) character. Each
            key - value pair must end in a carriage return.
        @par
            Settings can be optionally grouped in sections, using a header
            beforehand of the form [SectionName]. 
    */
	class _OgreExport ConfigFile : public ConfigAlloc
    {
    public:

        ConfigFile();
        virtual ~ConfigFile();
        /// load from a filename (not using resource group locations)
        void load(const String& filename, const String& separators = "\t:=", bool trimWhitespace = true);
        /// load from a filename (using resource group locations)
        void load(const String& filename, const String& resourceGroup, const String& separators = "\t:=", bool trimWhitespace = true);
        /// load from a data stream
        void load(const DataStreamPtr& stream, const String& separators = "\t:=", bool trimWhitespace = true);
		/// load from a filename (not using resource group locations)
		void loadDirect(const String& filename, const String& separators = "\t:=", bool trimWhitespace = true);
		/// load from a filename (using resource group locations)
		void loadFromResourceSystem(const String& filename, const String& resourceGroup, const String& separators = "\t:=", bool trimWhitespace = true);

        /** Gets the first setting from the file with the named key. 
        @param key The name of the setting
        @param section The name of the section it must be in (if any)
		@param defaultValue The value to return if the setting is not found
        */
        String getSetting(const String& key, const String& section = StringUtil::BLANK, const String& defaultValue = StringUtil::BLANK) const;
        /** Gets all settings from the file with the named key. */
        StringVector getMultiSetting(const String& key, const String& section = StringUtil::BLANK) const;

        typedef std::multimap<String, String> SettingsMultiMap;
        typedef MapIterator<SettingsMultiMap> SettingsIterator;
        /** Gets an iterator for stepping through all the keys / values in the file. */
        typedef std::map<String, SettingsMultiMap*> SettingsBySection;
        typedef MapIterator<SettingsBySection> SectionIterator;
        /** Get an iterator over all the available sections in the config file */
        SectionIterator getSectionIterator(void);
        /** Get an iterator over all the available settings in a section */
        SettingsIterator getSettingsIterator(const String& section = StringUtil::BLANK);


        
        /** Clear the settings */
        void clear(void);
    protected:
        SettingsBySection mSettings;
    };

}


#endif
