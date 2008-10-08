/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
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

#include "OgreGLESGpuProgramManager.h"
#include "OgreLogManager.h"

namespace Ogre {
    GLESGpuProgramManager::GLESGpuProgramManager()
    {
        // Superclass sets up members

        // Register with resource group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }

    GLESGpuProgramManager::~GLESGpuProgramManager()
    {
        // Unregister with resource group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }

    bool GLESGpuProgramManager::registerProgramFactory(const String& syntaxCode,
                                                     CreateGpuProgramCallback createFn)
    {
        return mProgramMap.insert(ProgramMap::value_type(syntaxCode, createFn)).second;
    }

    bool GLESGpuProgramManager::unregisterProgramFactory(const String& syntaxCode)
    {
        return mProgramMap.erase(syntaxCode) != 0;
    }

    Resource* GLESGpuProgramManager::createImpl(const String& name,
                                                ResourceHandle handle,
                                                const String& group, bool isManual,
                                                ManualResourceLoader* loader,
                                                const NameValuePairList* params)
    {
        NameValuePairList::const_iterator paramSyntax, paramType;

        if (!params || (paramSyntax = params->find("syntax")) == params->end() ||
            (paramType = params->find("type")) == params->end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "You must supply 'syntax' and 'type' parameters",
                "GLESGpuProgramManager::createImpl");
        }

        ProgramMap::const_iterator iter = mProgramMap.find(paramSyntax->second);
        if(iter == mProgramMap.end())
        {
            // TODO not implemented
            return 0;
        }

        GpuProgramType gpt;
        if (paramType->second == "vertex_program")
        {
            gpt = GPT_VERTEX_PROGRAM;
        }
        else
        {
            gpt = GPT_FRAGMENT_PROGRAM;
        }

        return (iter->second)(this, name, handle, group, isManual,
                              loader, gpt, paramSyntax->second);
    }

    Resource* GLESGpuProgramManager::createImpl(const String& name,
                                                ResourceHandle handle,
                                                const String& group, bool isManual,
                                                ManualResourceLoader* loader,
                                                GpuProgramType gptype,
                                                const String& syntaxCode)
    {
        // TODO not implemented
        return 0;
    }
}
