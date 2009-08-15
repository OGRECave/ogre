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

#ifndef __StringInterface_H__
#define __StringInterface_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreCommon.h"

namespace Ogre {


    /// List of parameter types available
    enum ParameterType
    {
        PT_BOOL,
        PT_REAL,
        PT_INT,
        PT_UNSIGNED_INT,
        PT_SHORT,
        PT_UNSIGNED_SHORT,
        PT_LONG,
        PT_UNSIGNED_LONG,
        PT_STRING,
        PT_VECTOR3,
        PT_MATRIX3,
        PT_MATRIX4,
        PT_QUATERNION,
        PT_COLOURVALUE
    };

    /// Definition of a parameter supported by a StringInterface class, for introspection
    class _OgreExport ParameterDef
    {
    public:
        String name;
        String description;
        ParameterType paramType;
        ParameterDef(const String& newName, const String& newDescription, ParameterType newType)
            : name(newName), description(newDescription), paramType(newType) {}
    };
    typedef std::vector<ParameterDef> ParameterList;

    /** Abstract class which is command object which gets/sets parameters.*/
    class _OgreExport ParamCommand
    {
    public:
        virtual String doGet(const void* target) const = 0;
        virtual void doSet(void* target, const String& val) = 0;

        virtual ~ParamCommand() { }
    };
    typedef std::map<String, ParamCommand* > ParamCommandMap;

    /** Class to hold a dictionary of parameters for a single class. */
    class _OgreExport ParamDictionary
    {
        friend class StringInterface;
    protected:
        /// Definitions of parameters
        ParameterList mParamDefs;

        /// Command objects to get/set
        ParamCommandMap mParamCommands;

        /** Retrieves the parameter command object for a named parameter. */
        ParamCommand* getParamCommand(const String& name)
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

		const ParamCommand* getParamCommand(const String& name) const
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
    public:
        ParamDictionary()  {}
        /** Method for adding a parameter definition for this class. 
        @param paramDef A ParameterDef object defining the parameter
        @param paramCmd Pointer to a ParamCommand subclass to handle the getting / setting of this parameter.
            NB this class will not destroy this on shutdown, please ensure you do

        */
        void addParameter(const ParameterDef& paramDef, ParamCommand* paramCmd)
        {
            mParamDefs.push_back(paramDef);
            mParamCommands[paramDef.name] = paramCmd;
        }
        /** Retrieves a list of parameters valid for this object. 
        @returns
            A reference to a static list of ParameterDef objects.

        */
        const ParameterList& getParameters(void) const
        {
            return mParamDefs;
        }



    };
    typedef std::map<String, ParamDictionary> ParamDictionaryMap;
    
    /** Class defining the common interface which classes can use to 
        present a reflection-style, self-defining parameter set to callers.
    @remarks
        This class also holds a static map of class name to parameter dictionaries
        for each subclass to use. See ParamDictionary for details. 
    @remarks
        In order to use this class, each subclass must call createParamDictionary in their constructors
        which will create a parameter dictionary for the class if it does not exist yet.
    */
    class _OgreExport StringInterface 
    {
    private:
		OGRE_STATIC_MUTEX( msDictionaryMutex );

        /// Dictionary of parameters
        static ParamDictionaryMap msDictionary;

        /// Class name for this instance to be used as a lookup (must be initialised by subclasses)
        String mParamDictName;
		ParamDictionary* mParamDict;

	protected:
        /** Internal method for creating a parameter dictionary for the class, if it does not already exist.
        @remarks
            This method will check to see if a parameter dictionary exist for this class yet,
            and if not will create one. NB you must supply the name of the class (RTTI is not 
            used or performance).
        @param
            className the name of the class using the dictionary
        @returns
            true if a new dictionary was created, false if it was already there
        */
        bool createParamDictionary(const String& className)
        {
			OGRE_LOCK_MUTEX( msDictionaryMutex )

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

    public:
		StringInterface() : mParamDict(NULL) { }

        /** Virtual destructor, see Effective C++ */
        virtual ~StringInterface() {}

        /** Retrieves the parameter dictionary for this class. 
        @remarks
            Only valid to call this after createParamDictionary.
        @returns
            Pointer to ParamDictionary shared by all instances of this class
            which you can add parameters to, retrieve parameters etc.
        */
        ParamDictionary* getParamDictionary(void)
        {
			return mParamDict;
        }

		const ParamDictionary* getParamDictionary(void) const
        {
			return mParamDict;
        }

        /** Retrieves a list of parameters valid for this object. 
        @returns
            A reference to a static list of ParameterDef objects.

        */
        const ParameterList& getParameters(void) const;

        /** Generic parameter setting method.
        @remarks
            Call this method with the name of a parameter and a string version of the value
            to set. The implementor will convert the string to a native type internally.
            If in doubt, check the parameter definition in the list returned from 
            StringInterface::getParameters.
        @param
            name The name of the parameter to set
        @param
            value String value. Must be in the right format for the type specified in the parameter definition.
            See the StringConverter class for more information.
        @returns
            true if set was successful, false otherwise (NB no exceptions thrown - tolerant method)
        */
        virtual bool setParameter(const String& name, const String& value);
        /** Generic multiple parameter setting method.
        @remarks
            Call this method with a list of name / value pairs
            to set. The implementor will convert the string to a native type internally.
            If in doubt, check the parameter definition in the list returned from 
            StringInterface::getParameters.
        @param
            paramList Name/value pair list
        */
        virtual void setParameterList(const NameValuePairList& paramList);
        /** Generic parameter retrieval method.
        @remarks
            Call this method with the name of a parameter to retrieve a string-format value of
            the parameter in question. If in doubt, check the parameter definition in the
            list returned from getParameters for the type of this parameter. If you
            like you can use StringConverter to convert this string back into a native type.
        @param
            name The name of the parameter to get
        @returns
            String value of parameter, blank if not found
        */
        virtual String getParameter(const String& name) const
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
        /** Method for copying this object's parameters to another object.
        @remarks
            This method takes the values of all the object's parameters and tries to set the
            same values on the destination object. This provides a completely type independent
            way to copy parameters to other objects. Note that because of the String manipulation 
            involved, this should not be regarded as an efficient process and should be saved for
            times outside of the rendering loop.
        @par
            Any unrecognised parameters will be ignored as with setParameter method.
        @param dest Pointer to object to have it's parameters set the same as this object.

        */
        virtual void copyParametersTo(StringInterface* dest) const
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

        /** Cleans up the static 'msDictionary' required to reset Ogre,
        otherwise the containers are left with invalid pointers, which will lead to a crash
        as soon as one of the ResourceManager implementers (e.g. MaterialManager) initializes.*/
        static void cleanupDictionary () ;

    };



}

#endif

