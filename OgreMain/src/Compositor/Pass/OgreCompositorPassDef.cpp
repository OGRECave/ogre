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

#include "Compositor/Pass/OgreCompositorPassDef.h"
#include "Compositor/Pass/PassClear/OgreCompositorPassClearDef.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"
#include "Compositor/Pass/PassStencil/OgreCompositorPassStencilDef.h"

namespace Ogre
{
	CompositorTargetDef::~CompositorTargetDef()
	{
		CompositorPassDefVec::const_iterator itor = mCompositorPasses.begin();
		CompositorPassDefVec::const_iterator end  = mCompositorPasses.end();

		while( itor != end )
		{
			OGRE_DELETE *itor;
			++itor;
		}

		mCompositorPasses.clear();
	}
	//-----------------------------------------------------------------------------------
	CompositorPassDef* CompositorTargetDef::addPass( CompositorPassType passType )
	{
		CompositorPassDef *retVal = 0;
		switch( passType )
		{
		case PASS_CLEAR:
			retVal = OGRE_NEW CompositorPassClearDef();
			break;
		case PASS_QUAD:
			retVal = OGRE_NEW CompositorPassQuadDef( mParentNodeDef );
			break;
		case PASS_SCENE:
			retVal = OGRE_NEW CompositorPassSceneDef();
			break;
		case PASS_STENCIL:
			retVal = OGRE_NEW CompositorPassStencilDef();
			break;
		}

		mCompositorPasses.push_back( retVal );
		
		return retVal;
	}
	//-----------------------------------------------------------------------------------
}
