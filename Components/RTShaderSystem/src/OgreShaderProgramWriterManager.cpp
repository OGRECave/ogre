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

#include "OgreShaderPrecompiledHeaders.h"

namespace Ogre {

//-----------------------------------------------------------------------
template<> 
RTShader::ProgramWriterManager* Singleton<RTShader::ProgramWriterManager>::msSingleton = 0;

namespace RTShader {
//-----------------------------------------------------------------------
ProgramWriterManager* ProgramWriterManager::getSingletonPtr(void)
{
    return msSingleton;
}
//-----------------------------------------------------------------------
ProgramWriterManager& ProgramWriterManager::getSingleton(void)
{  
    assert( msSingleton );  
    return ( *msSingleton );  
}
//-----------------------------------------------------------------------
ProgramWriterManager::ProgramWriterManager()
{
    // Add standard shader writer factories
#if OGRE_PLATFORM != OGRE_PLATFORM_ANDROID
    addProgramWriter("glsl", new GLSLProgramWriter());
    addProgramWriter("hlsl", new CGProgramWriter());
#endif
    addProgramWriter("glslang", new GLSLProgramWriter());
    addProgramWriter("glsles", new GLSLESProgramWriter());
}
//-----------------------------------------------------------------------
ProgramWriterManager::~ProgramWriterManager()
{
    for(auto& it : mProgramWriters)
    {
        delete it.second;
    }
}
//-----------------------------------------------------------------------
void ProgramWriterManager::addProgramWriter(const String& lang, ProgramWriter* writer)
{
    mProgramWriters[lang] = writer;
}
//-----------------------------------------------------------------------
bool ProgramWriterManager::isLanguageSupported(const String& lang)
{
    return mProgramWriters.find(lang) != mProgramWriters.end();
}
}
}
