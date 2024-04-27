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
#ifndef __ConfigFile_H__
#define __ConfigFile_H__

#include "OgrePrerequisites.h"

#include "OgreCommon.h"
#include "OgreStringVector.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    template <typename T> class MapIterator;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */
    /** Class for quickly loading settings from a text file.

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
        String getSetting(const String& key, const String& section = BLANKSTRING, const String& defaultValue = BLANKSTRING) const;
        /** Gets all settings from the file with the named key. */
        StringVector getMultiSetting(const String& key, const String& section = BLANKSTRING) const;

        typedef std::multimap<String, String> SettingsMultiMap;
        typedef MapIterator<SettingsMultiMap> SettingsIterator;
        /** Gets an iterator for stepping through all the keys / values in the file. */
        typedef std::map<String, SettingsMultiMap*> SettingsBySection;
        typedef std::map<String, SettingsMultiMap> SettingsBySection_;
        typedef MapIterator<SettingsBySection> SectionIterator;

        /// @deprecated use getSettingsBySection()
        OGRE_DEPRECATED SectionIterator getSectionIterator(void);

        /** Get all the available settings grouped by sections */
        const SettingsBySection_& getSettingsBySection() const {
            return mSettings;
        }

        /// @deprecated use getSettings()
        OGRE_DEPRECATED SettingsIterator getSettingsIterator(const String& section = BLANKSTRING);

        /** Get all the available settings in a section */
        const SettingsMultiMap& getSettings(const String& section = BLANKSTRING) const;
        
        /** Clear the settings */
        void clear(void);
    protected:
        SettingsBySection_ mSettings;
        SettingsBySection mSettingsPtr; // for backwards compatibility
    };
    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
