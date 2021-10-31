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
#ifndef __ShaderProgramWriterManager_H__
#define __ShaderProgramWriterManager_H__

#include "OgreShaderPrerequisites.h"
#include "OgreSingleton.h"

namespace Ogre {
namespace RTShader {

    class ProgramWriter;

/** \addtogroup Optional
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/// @deprecated
class _OgreRTSSExport OGRE_DEPRECATED ProgramWriterFactory : public RTShaderSystemAlloc
{
public:
    ProgramWriterFactory() {}
    virtual ~ProgramWriterFactory() {}
    
    /// Get the name of the language this factory creates programs for
    virtual const String& getTargetLanguage(void) const = 0;
    
    /// Create writer instance
    virtual ProgramWriter* create(void) OGRE_NODISCARD = 0;
};

class _OgreRTSSExport ProgramWriterManager 
    : public Singleton<ProgramWriterManager>, public RTShaderSystemAlloc
{
    std::map<String, ProgramWriter*> mProgramWriters;
public:
    typedef std::map<String, ProgramWriterFactory*> FactoryMap;
protected:
    /// unused
    FactoryMap mFactories;

public:
    ProgramWriterManager();
    ~ProgramWriterManager();

    /// register and transfer ownership of writer
    void addProgramWriter(const String& lang, ProgramWriter* writer);

    /** Returns whether a given high-level language is supported. */
    bool isLanguageSupported(const String& lang);

    /// @deprecated
    OGRE_DEPRECATED void addFactory(ProgramWriterFactory* factory)
    {
        addProgramWriter(factory->getTargetLanguage(), factory->create());
    }

    /// @deprecated
    OGRE_DEPRECATED void removeFactory(ProgramWriterFactory* factory)
    {
        mProgramWriters.erase(factory->getTargetLanguage());
    }

    /// @deprecated
    OGRE_DEPRECATED ProgramWriter* createProgramWriter( const String& language);

    ProgramWriter* getProgramWriter(const String& language) const
    {
        auto it = mProgramWriters.find(language);
        if (it != mProgramWriters.end())
            return it->second;
        OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No program writer for language " + language);
        return nullptr;
    }

    /** Override standard Singleton retrieval.
    @remarks
    Why do we do this? Well, it's because the Singleton
    implementation is in a .h file, which means it gets compiled
    into anybody who includes it. This is needed for the
    Singleton template to work, but we actually only want it
    compiled into the implementation of the class based on the
    Singleton, not all of them. If we don't change this, we get
    link errors when trying to use the Singleton-based class from
    an outside dll.
    @par
    This method just delegates to the template version anyway,
    but the implementation stays in this single compilation unit,
    preventing link errors.
    */
    static ProgramWriterManager& getSingleton();

    /// @copydoc Singleton::getSingleton()
    static ProgramWriterManager* getSingletonPtr();
};
/** @} */
/** @} */
}
}


#endif
