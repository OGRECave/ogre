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
#ifndef __GLSLESCgProgram_H__
#define __GLSLESCgProgram_H__

#include "OgreGLSLESProgram.h"
#include "OgreStringVector.h"

namespace Ogre {
    /** Specialisation of HighLevelGpuProgram to provide support for CG

        Cg programs will be converted to GLSL        
    */
    class _OgreGLES2Export GLSLESCgProgram : public GLSLESProgram
    {
    public:
        /// Command object for setting profiles
        class CmdProfiles : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        GLSLESCgProgram(ResourceManager* creator, 
            const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader);
        ~GLSLESCgProgram();


        /// Overridden from GLSLESProgram
        void loadFromSource(void);

        /// Overridden from GLSLESProgram
        const String& getLanguage(void) const;

        /** Sets the entry point for this program ie the first method called. */
        void setEntryPoint(const String& entryPoint) { mEntryPoint = entryPoint; }
        /** Gets the entry point defined for this program. */
        const String& getEntryPoint(void) const { return mEntryPoint; }
        /** Sets the Cg profiles which can be supported by the program. */
        void setProfiles(const StringVector& profiles);
        /** Gets the Cg profiles which can be supported by the program. */
        const StringVector& getProfiles(void) const { return mProfiles; }
    protected:
        static CmdProfiles msCmdProfiles;
        StringVector mProfiles;

        // check if syntax is supported
        bool isSyntaxSupported();
        // deletes the key word ": register(xx)" that hlsl2glsl doesn't know to handle
        String deleteRegisterFromCg(const String& inSource);
    };
}

#endif // __GLSLESCgProgram_H__
