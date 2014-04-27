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

#include "OgreHlmsGui2DMobile.h"
#include "OgreHlmsGui2DMobileDatablock.h"

#include "OgreHighLevelGpuProgramManager.h"
#include "OgreHighLevelGpuProgram.h"

#include "OgreSceneManager.h"
#include "Compositor/OgreCompositorShadowNode.h"

namespace Ogre
{
    const String c_vsPerObjectUniforms[] =
    {
        "worldViewProj"
    };
    const String c_psPerObjectUniforms[] =
    {
        "constColour"
    };

    HlmsGui2DMobile::HlmsGui2DMobile( Archive *dataFolder ) : Hlms( HLMS_PBS, dataFolder )
    {
    }
    //-----------------------------------------------------------------------------------
    HlmsGui2DMobile::~HlmsGui2DMobile()
    {
    }
    //-----------------------------------------------------------------------------------
    const HlmsCache* HlmsGui2DMobile::createShaderCacheEntry( uint32 renderableHash,
                                                         const HlmsCache &passCache,
                                                         uint32 finalHash,
                                                         const QueuedRenderable &queuedRenderable )
    {
        const HlmsCache *retVal = Hlms::createShaderCacheEntry( renderableHash, passCache, finalHash,
                                                                queuedRenderable );

        GpuNamedConstants *constantsDef;
        //Nasty const_cast, but the refactor required to remove this is 100x nastier.
        constantsDef = const_cast<GpuNamedConstants*>( &retVal->vertexShader->getConstantDefinitions() );
        for( size_t i=0; i<sizeof( c_vsPerObjectUniforms ) / sizeof( String* ); ++i )
        {
            GpuConstantDefinitionMap::iterator it = constantsDef->map.find( c_vsPerObjectUniforms[i] );
            if( it != constantsDef->map.end() )
                it->second.variability = GPV_PER_OBJECT;
        }

        //Nasty const_cast, but the refactor required to remove this is 100x nastier.
        constantsDef = const_cast<GpuNamedConstants*>( &retVal->pixelShader->getConstantDefinitions() );
        for( size_t i=0; i<sizeof( c_vsPerObjectUniforms ) / sizeof( String* ); ++i )
        {
            GpuConstantDefinitionMap::iterator it = constantsDef->map.find( c_psPerObjectUniforms[i] );
            if( it != constantsDef->map.end() )
                it->second.variability = GPV_PER_OBJECT;
        }

        assert( dynamic_cast<const HlmsGui2DMobileDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsGui2DMobileDatablock *datablock = static_cast<const HlmsGui2DMobileDatablock*>(
                                                    queuedRenderable.renderable->getDatablock() );

        //Set samplers.
        GpuProgramParametersSharedPtr psParams = retVal->pixelShader->getDefaultParameters();

        if( datablock->mNumDiffuseTextures > 0 )
        {
            uint texUnit = 0;
            vector<int>::type diffuseTex;
            diffuseTex.reserve( datablock->mNumDiffuseTextures );
            for( texUnit=0; texUnit<datablock->mNumDiffuseTextures; ++texUnit )
            {
                assert( !datablock->mDiffuseTextures[texUnit].isNull() );
                diffuseTex.push_back( texUnit );
                ++texUnit;
            }

            psParams->setNamedConstant( "texDiffuseMap", &diffuseTex[0], diffuseTex.size(), 1 );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsCache HlmsGui2DMobile::preparePassHash( const CompositorShadowNode *shadowNode, bool casterPass,
                                            bool dualParaboloid, SceneManager *sceneManager )
    {
        HlmsCache retVal = Hlms::preparePassHash( shadowNode, casterPass, dualParaboloid, sceneManager );
        Camera *camera = sceneManager->getCameraInProgress();
        Matrix4 viewMatrix = camera->getViewMatrix(true);

        mPreparedPass.viewProjMatrix    = camera->getProjectionMatrix() * viewMatrix; //TODO

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsGui2DMobile::fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                     bool casterPass, const HlmsCache *lastCache,
                                     uint32 lastTextureHash )
    {
        GpuProgramParametersSharedPtr vpParams = cache->vertexShader->getDefaultParameters();
        GpuProgramParametersSharedPtr psParams = cache->pixelShader->getDefaultParameters();
        float *vsUniformBuffer = vpParams->getFloatPointer( 0 );
        float *psUniformBuffer = psParams->getFloatPointer( 0 );

        assert( dynamic_cast<const HlmsGui2DMobileDatablock*>( queuedRenderable.renderable->getDatablock() ) );
        const HlmsGui2DMobileDatablock *datablock = static_cast<const HlmsGui2DMobileDatablock*>(
                                                queuedRenderable.renderable->getDatablock() );

        uint16 variabilityMask = cache != lastCache ? GPV_ALL : GPV_PER_OBJECT;

        const Matrix4 &worldMat = queuedRenderable.movableObject->_getParentNodeFullTransform();

        //---------------------------------------------------------------------------
        //                          ---- VERTEX SHADER ----
        //---------------------------------------------------------------------------
#if !OGRE_DOUBLE_PRECISION
        //mat4 worldViewProj
        Matrix4 tmp = mPreparedPass.viewProjMatrix * worldMat;
        memcpy( vsUniformBuffer, &tmp, sizeof(Matrix4) );
        vsUniformBuffer += 16;
#else
    #error Not Coded Yet! (can't use memcpy on Matrix4)
#endif

        memcpy( vsUniformBuffer, datablock->mTextureMatrices,
                16 * datablock->mNumTextureMatrices * sizeof(float) );
        vsUniformBuffer += 16 * datablock->mNumTextureMatrices;

        //---------------------------------------------------------------------------
        //                          ---- PIXEL SHADER ----
        //---------------------------------------------------------------------------
        if( datablock->mHasColour )
        {
            memcpy( psUniformBuffer, &datablock->mR, 4 * sizeof(float) );
            psUniformBuffer += 4;
        }

        if( datablock->mTextureHash != lastTextureHash )
        {
            //Rebind textures
            for( uint texUnit=0; texUnit<datablock->mNumDiffuseTextures; ++texUnit )
            {
                mRenderSystem->_setTexture( texUnit, true, datablock->mDiffuseTextures[texUnit] );
                ++texUnit;
            }

            mRenderSystem->_disableTextureUnitsFrom( datablock->mNumDiffuseTextures );
        }

        assert( vsUniformBuffer - vpParams->getFloatPointer( 0 ) == vpParams->getFloatConstantList().size() );
        assert( psUniformBuffer - psParams->getFloatPointer( 0 ) == psParams->getFloatConstantList().size() );

        mRenderSystem->bindGpuProgramParameters( GPT_VERTEX_PROGRAM, vpParams, variabilityMask );
        mRenderSystem->bindGpuProgramParameters( GPT_FRAGMENT_PROGRAM, psParams, variabilityMask );
    }
    //-----------------------------------------------------------------------------------
    HlmsDatablock* HlmsGui2DMobile::createDatablockImpl( const HlmsParamVec &paramVec,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    IdString datablockName )
    {
        return OGRE_NEW HlmsGui2DMobileDatablock( datablockName, this, macroblock, blendblock, paramVec );
    }
}
