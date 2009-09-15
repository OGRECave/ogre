/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreD3D9GpuProgramManager.h"
#include "OgreD3D9GpuProgram.h"
#include "OgreException.h"

namespace Ogre {
    //-----------------------------------------------------------------------------
    D3D9GpuProgramManager::D3D9GpuProgramManager()
        :GpuProgramManager()
    {
        // Superclass sets up members 

        // Register with resource group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

    }
    //-----------------------------------------------------------------------------
    D3D9GpuProgramManager::~D3D9GpuProgramManager()
    {
        // Unregister with resource group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);

    }
    //-----------------------------------------------------------------------------
    Resource* D3D9GpuProgramManager::createImpl(const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader,
        const NameValuePairList* params)
    {
        NameValuePairList::const_iterator paramIt;

        if (!params || (paramIt = params->find("type")) == params->end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "You must supply a 'type' parameter",
                "D3D9GpuProgramManager::createImpl");
        }

        if (paramIt->second == "vertex_program")
        {
            return new D3D9GpuVertexProgram(this, name, handle, group, 
                isManual, loader);
        }
        else
        {
            return new D3D9GpuFragmentProgram(this, name, handle, group, 
                isManual, loader);
        }
    }
    //-----------------------------------------------------------------------------
    Resource* D3D9GpuProgramManager::createImpl(const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader,
        GpuProgramType gptype, const String& syntaxCode)
    {
        if (gptype == GPT_VERTEX_PROGRAM)
        {
            return new D3D9GpuVertexProgram(this, name, handle, group, 
                isManual, loader);
        }
        else
        {
            return new D3D9GpuFragmentProgram(this, name, handle, group, 
                isManual, loader);
        }
    }
}
