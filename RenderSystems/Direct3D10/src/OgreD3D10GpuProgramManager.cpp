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
#include "OgreD3D10GpuProgramManager.h"
#include "OgreD3D10GpuProgram.h"
#include "OgreD3D10Device.h"
#include "OgreException.h"

namespace Ogre {
	//-----------------------------------------------------------------------------
	D3D10GpuProgramManager::D3D10GpuProgramManager(D3D10Device & device)
		:GpuProgramManager(), mDevice(device)
	{
		// Superclass sets up members 

		// Register with resource group manager
		ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

	}
	//-----------------------------------------------------------------------------
	D3D10GpuProgramManager::~D3D10GpuProgramManager()
	{
		// Unregister with resource group manager
		ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);

	}
	//-----------------------------------------------------------------------------
	Resource* D3D10GpuProgramManager::createImpl(const String& name, ResourceHandle handle, 
		const String& group, bool isManual, ManualResourceLoader* loader,
		const NameValuePairList* params)
	{
		NameValuePairList::const_iterator paramIt;

		if (!params || (paramIt = params->find("type")) == params->end())
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"You must supply a 'type' parameter",
				"D3D10GpuProgramManager::createImpl");
		}

		if (paramIt->second == "vertex_program")
		{
			return new D3D10GpuVertexProgram(this, name, handle, group, 
				isManual, loader, mDevice);
		}
		else
		{
			return new D3D10GpuFragmentProgram(this, name, handle, group, 
				isManual, loader, mDevice);
		}
	}
	//-----------------------------------------------------------------------------
	Resource* D3D10GpuProgramManager::createImpl(const String& name, ResourceHandle handle, 
		const String& group, bool isManual, ManualResourceLoader* loader,
		GpuProgramType gptype, const String& syntaxCode)
	{
		if (gptype == GPT_VERTEX_PROGRAM)
		{
			return new D3D10GpuVertexProgram(this, name, handle, group, 
				isManual, loader, mDevice);
		}
		else
		{
			return new D3D10GpuFragmentProgram(this, name, handle, group, 
				isManual, loader, mDevice);
		}
	}
}
