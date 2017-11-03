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

#include "Compositor/Pass/OgreCompositorPassDef.h"
#include "Compositor/Pass/PassClear/OgreCompositorPassClearDef.h"
#include "Compositor/Pass/PassCompute/OgreCompositorPassComputeDef.h"
#include "Compositor/Pass/PassDepthCopy/OgreCompositorPassDepthCopyDef.h"
#include "Compositor/Pass/PassMipmap/OgreCompositorPassMipmapDef.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"
#include "Compositor/Pass/PassStencil/OgreCompositorPassStencilDef.h"
#include "Compositor/Pass/PassUav/OgreCompositorPassUavDef.h"

#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/Pass/OgreCompositorPassProvider.h"

#include "OgreLwString.h"

namespace Ogre
{
    const char *CompositorPassTypeEnumNames[PASS_CUSTOM+1u] =
    {
        "INVALID",
        "SCENE",
        "QUAD",
        "CLEAR",
        "STENCIL",
        "RESOLVE",
        "DEPTHCOPY",
        "UAV",
        "MIPMAP",
        "COMPUTE",
        "CUSTOM"
    };

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
    CompositorPassDef* CompositorTargetDef::addPass( CompositorPassType passType, IdString customId )
    {
        CompositorPassDef *retVal = 0;
        switch( passType )
        {
        case PASS_CLEAR:
            retVal = OGRE_NEW CompositorPassClearDef( this );
            break;
        case PASS_QUAD:
            retVal = OGRE_NEW CompositorPassQuadDef( mParentNodeDef, this );
            break;
        case PASS_SCENE:
            retVal = OGRE_NEW CompositorPassSceneDef( this );
            break;
        case PASS_STENCIL:
            retVal = OGRE_NEW CompositorPassStencilDef( this );
            break;
        case PASS_DEPTHCOPY:
            retVal = OGRE_NEW CompositorPassDepthCopyDef( mParentNodeDef, this );
            break;
        case PASS_UAV:
            retVal = OGRE_NEW CompositorPassUavDef( mParentNodeDef, this );
            break;
        case PASS_MIPMAP:
            retVal = OGRE_NEW CompositorPassMipmapDef( this );
            break;
        case PASS_COMPUTE:
            retVal = OGRE_NEW CompositorPassComputeDef( mParentNodeDef, this );
            break;
        case PASS_CUSTOM:
            {
                CompositorPassProvider *passProvider = mParentNodeDef->getCompositorManager()->
                                                                    getCompositorPassProvider();
                if( !passProvider )
                {
                    OGRE_EXCEPT( Exception::ERR_INVALID_STATE,
                                 "Using custom compositor passes but no provider is set",
                                 "CompositorTargetDef::addPass" );
                }

                retVal = passProvider->addPassDef( passType, customId, this, mParentNodeDef );
            }
            break;
        default:
            break;
        }

        char tmpBuffer[128];
        LwString profilingId( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );
        profilingId.a( CompositorPassTypeEnumNames[passType], " ",
                       Id::generateNewId<CompositorPassDef>() );
        retVal->mProfilingId = profilingId.c_str();

        mParentNodeDef->postInitializePassDef( retVal );

        mCompositorPasses.push_back( retVal );
        
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    uint32 CompositorPassDef::getRtIndex() const
    {
        return mParentTargetDef->getRtIndex();
    }
    //-----------------------------------------------------------------------------------
    const CompositorTargetDef* CompositorPassDef::getParentTargetDef() const
    {
        return mParentTargetDef;
    }
}
