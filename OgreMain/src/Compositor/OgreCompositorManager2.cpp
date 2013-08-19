/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"

namespace Ogre
{
	bool CompositorManager2::hasNodeDefinition( IdString nodeDefName ) const
	{
		return mNodeDefinitions.find( nodeDefName ) != mNodeDefinitions.end();
	}
	//-----------------------------------------------------------------------------------
	const CompositorNodeDef* CompositorManager2::getNodeDefinition( IdString nodeDefName ) const
	{
		CompositorNodeDef const *retVal = 0;
		
		CompositorNodeDefMap::const_iterator itor = mNodeDefinitions.find( nodeDefName );
		if( itor == mNodeDefinitions.end() )
		{
			OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Node definition with name '" +
							nodeDefName.getFriendlyText() + "' not found",
							"CompositorManager2::getNodeDefinition" );
		}
		else
		{
			retVal = itor->second;
		}

		return retVal;
	}
	//-----------------------------------------------------------------------------------
	CompositorNodeDef* CompositorManager2::addNodeDefinition( IdString name )
	{
		CompositorNodeDef *retVal = 0;

		if( mNodeDefinitions.find( name ) == mNodeDefinitions.end() )
		{
			retVal = new CompositorNodeDef( name );
		}
		else
		{
			OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM, "A node definition with name '" +
							name.getFriendlyText() + "' already exists",
							"CompositorManager2::addNodeDefinition" );
		}

		return retVal;
	}
	//-----------------------------------------------------------------------------------
	CompositorWorkspaceDef* CompositorManager2::addWorkspaceDefinition( IdString name )
	{
		CompositorWorkspaceDef *retVal = 0;

		if( mWorkspaceDefs.find( name ) == mWorkspaceDefs.end() )
		{
			retVal = new CompositorWorkspaceDef( name, this );
		}
		else
		{
			OGRE_EXCEPT( Exception::ERR_DUPLICATE_ITEM, "A workspace with name '" +
							name.getFriendlyText() + "' already exists",
							"CompositorManager2::addWorkspaceDefinition" );
		}

		return retVal;
	}
}
