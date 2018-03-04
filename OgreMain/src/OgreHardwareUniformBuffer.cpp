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

#include "OgreStableHeaders.h"
#include "OgreHardwareUniformBuffer.h"
#include "OgreDefaultHardwareBufferManager.h"

namespace Ogre {

    HardwareUniformBuffer::HardwareUniformBuffer(HardwareBufferManagerBase* mgr, size_t sizeBytes, 
                                    HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name)
        : HardwareBuffer(usage, false, useShadowBuffer)
        , mMgr(mgr)
        , mName(name)
    {
        // Calculate the size of the vertices
        mSizeInBytes = sizeBytes;

        // Create a shadow buffer if required
        if (mUseShadowBuffer)
        {
            mShadowBuffer.reset(new DefaultHardwareUniformBuffer(mMgr, sizeBytes, HardwareBuffer::HBU_DYNAMIC, false));
        }
    }
    
    HardwareUniformBuffer::~HardwareUniformBuffer()
    {
        if (mMgr)
        {
            mMgr->_notifyUniformBufferDestroyed(this);
        }
    }

    /*
    bool HardwareUniformBuffer::writeParams(GpuProgramParametersSharedPtr params)
    {
        // Lock buffer
        void* mappedData = this->lock(HardwareBuffer::HBL_DISCARD);
        if (!mappedData)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                "Cannot update uniform buffer\nError description: error locking uniform buffer",
                "HardwareUniformBuffer::writeParams");
        }

        // Store temporary data address
        void* srcData = 0;

        // Iterate through variables
        ShaderVariableIterator it = mShaderVars.begin();
        ShaderVariableIterator end = mShaderVars.end();
        while(it != end)
        {
            String varName = it->name;

            // hack for cg parameter with strange prefix
            if (varName.size() > 0 && varName[0] == '_')
            {
                varName.erase(0,1);
            }

            const GpuConstantDefinition& def = params->getConstantDefinition(varName);
            if (def.isFloat())
            {
                srcData = (void *)&(*(params->getFloatConstantList().begin() + def.physicalIndex));
            }
            else
            {
                srcData = (void *)&(*(params->getIntConstantList().begin() + def.physicalIndex));
            }

            memcpy( &(((char *)(mappedData))[it->startOffset]), srcData , it->size);
        }

        // Unlock buffer
        this->unlock();

        return true;
    }

    bool HardwareUniformBuffer::writeSharedParams(GpuSharedParametersPtr sharedParams)
    {
        // Lock buffer
        void* mappedData = this->lock(HardwareBuffer::HBL_DISCARD);
        if (!mappedData)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                "Cannot update uniform buffer\nError description: error locking uniform buffer",
                "HardwareUniformBuffer::writeParams");
        }

        // Store temporary data address
        void* srcData = 0;

        // Iterate through variables
        ShaderVariableIterator it = mShaderVars.begin();
        ShaderVariableIterator end = mShaderVars.end();
        while(it != end)
        {
            String varName = it->name;

            // hack for cg parameter with strange prefix
            if (varName.size() > 0 && varName[0] == '_')
            {
                varName.erase(0,1);
            }

            const GpuConstantDefinition& def = sharedParams->getConstantDefinition(varName);
            if (def.isFloat())
            {
                srcData = (void *)&(*(sharedParams->getFloatConstantList().begin() + def.physicalIndex));
            }
            else
            {
                srcData = (void *)&(*(sharedParams->getIntConstantList().begin() + def.physicalIndex));
            }

            memcpy( &(((char *)(mappedData))[it->startOffset]), srcData , it->size);
        }

        // Unlock buffer
        this->unlock();

        return true;
    }
    */
}
