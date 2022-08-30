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

#ifndef __GLGpuProgramManager_H__
#define __GLGpuProgramManager_H__

#include "OgreGLPrerequisites.h"
#include "OgreGpuProgramManager.h"

namespace Ogre {

typedef GpuProgram* (*CreateGpuProgramCallback)(ResourceManager* creator,
    const String& name, ResourceHandle handle,
    const String& group, bool isManual, ManualResourceLoader* loader,
    GpuProgramType gptype, const String& syntaxCode);

struct CreateCallbackWrapper : public GpuProgramFactory
{
    String language;
    CreateGpuProgramCallback callback;
    const String& getLanguage(void) const override { return language; };
    GpuProgram* create(ResourceManager* creator, const String& name, ResourceHandle handle,
                       const String& group, bool isManual, ManualResourceLoader* loader) override
    {
        // type and syntax code will be corrected by GpuProgramManager
        return callback(creator, name, handle, group, isManual, loader, GPT_VERTEX_PROGRAM, "");
    }
    CreateCallbackWrapper(const String& lang, CreateGpuProgramCallback cb) : language(lang), callback(cb) {}
};

class GLGpuProgramManager
{
    std::list<CreateCallbackWrapper> mFactories;
public:
    void registerProgramFactory(const String& syntaxCode, CreateGpuProgramCallback createFn)
    {
        mFactories.emplace_back(syntaxCode, createFn);
        GpuProgramManager::getSingleton().addFactory(&mFactories.back());
    }
};

} //namespace Ogre

#endif //__GLGpuProgramManager_H__
