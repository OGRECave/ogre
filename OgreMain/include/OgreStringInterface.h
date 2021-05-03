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

#ifndef __StringInterface_H__
#define __StringInterface_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreHeaderPrefix.h"
#include "OgreStringConverter.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */

    /// @deprecated do not use
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

    /// @deprecated directly pass parameter name
    class _OgreExport ParameterDef
    {
    public:
        String name;
        ParameterDef(const String& newName, const String& = "", ParameterType = PT_INT)
            : name(newName) {}
    };
    typedef std::vector<String> ParameterList;

    /** Abstract class which is command object which gets/sets parameters.*/
    class _OgreExport ParamCommand
    {
    public:
        virtual String doGet(const void* target) const = 0;
        virtual void doSet(void* target, const String& val) = 0;

        virtual ~ParamCommand() { }
    };
    typedef std::map<String, ParamCommand* > ParamCommandMap;

#ifndef SWIG
    /** Generic ParamCommand implementation
     stores pointers to the class getter and setter functions */
    template <typename _Class, typename Param, Param (_Class::*getter)() const, void (_Class::*setter)(Param)>
    class SimpleParamCommand : public ParamCommand {
    public:
        String doGet(const void* target) const {
            return StringConverter::toString((static_cast<const _Class*>(target)->*getter)());
        }

        void doSet(void* target, const String& val) {
            typename std::decay<Param>::type tmp;
            StringConverter::parse(val, tmp);
            (static_cast<_Class*>(target)->*setter)(tmp);
        }
    };

    /// specialization for strings
    template <typename _Class, const String& (_Class::*getter)() const, void (_Class::*setter)(const String&)>
    class SimpleParamCommand<_Class, const String&, getter, setter> : public ParamCommand {
    public:
        String doGet(const void* target) const {
            return (static_cast<const _Class*>(target)->*getter)();
        }

        void doSet(void* target, const String& val) {
            (static_cast<_Class*>(target)->*setter)(val);
        }
    };
#endif

    /** Class to hold a dictionary of parameters for a single class. */
    class _OgreExport ParamDictionary
    {
        friend class StringInterface;
        /// Definitions of parameters
        ParameterList mParamDefs;

        /// Command objects to get/set
        ParamCommandMap mParamCommands;

        /** Retrieves the parameter command object for a named parameter. */
        ParamCommand* getParamCommand(const String& name);
        const ParamCommand* getParamCommand(const String& name) const;
    public:
        ParamDictionary();
        ~ParamDictionary();
        /** Method for adding a parameter definition for this class. 
        @param name The name of the parameter
        @param paramCmd Pointer to a ParamCommand subclass to handle the getting / setting of this parameter.
            NB this class will not destroy this on shutdown, please ensure you do

        */
        void addParameter(const String& name, ParamCommand* paramCmd);

        /// @deprecated do not use
        void addParameter(const ParameterDef& def, ParamCommand* paramCmd)
        {
            addParameter(def.name, paramCmd);
        }
        /** Retrieves a list of parameters valid for this object. 
        @return
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
        @return
            true if a new dictionary was created, false if it was already there
        */
        bool createParamDictionary(const String& className);

    public:
        StringInterface() : mParamDict(NULL) { }

        /** Virtual destructor, see Effective C++ */
        virtual ~StringInterface() {}

        /** Retrieves the parameter dictionary for this class. 
        @remarks
            Only valid to call this after createParamDictionary.
        @return
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
        @return
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
        @return
            true if set was successful, false otherwise (NB no exceptions thrown - tolerant method)
        */
        bool setParameter(const String& name, const String& value);
        /** Generic multiple parameter setting method.
        @remarks
            Call this method with a list of name / value pairs
            to set. The implementor will convert the string to a native type internally.
            If in doubt, check the parameter definition in the list returned from 
            StringInterface::getParameters.
        @param
            paramList Name/value pair list
        */
        void setParameterList(const NameValuePairList& paramList);
        /** Generic parameter retrieval method.
        @remarks
            Call this method with the name of a parameter to retrieve a string-format value of
            the parameter in question. If in doubt, check the parameter definition in the
            list returned from getParameters for the type of this parameter. If you
            like you can use StringConverter to convert this string back into a native type.
        @param
            name The name of the parameter to get
        @return
            String value of parameter, blank if not found
        */
        String getParameter(const String& name) const;
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
        void copyParametersTo(StringInterface* dest) const;
        /** Cleans up the static 'msDictionary' required to reset Ogre,
        otherwise the containers are left with invalid pointers, which will lead to a crash
        as soon as one of the ResourceManager implementers (e.g. MaterialManager) initializes.*/
        static void cleanupDictionary () ;

    };

    /** @} */
    /** @} */


}

#include "OgreHeaderSuffix.h"

#endif

