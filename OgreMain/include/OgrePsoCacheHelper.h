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
#ifndef _OgrePsoCacheHelper_H_
#define _OgrePsoCacheHelper_H_

#include "Vao/OgreVertexBufferPacked.h"
#include "OgreHlmsPso.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /** Utility class to cache PSOs. Useful for porting v1 libraries (eg. Gorilla) that
        render in "immediate mode" style and weren't build with PSOs in mind.
        Ideally a PSO would be created ahead of time and its pointer stored alongside
        the renderable you need to render.
    @par
        Almost all functions already do redundant state checking unless explicitly stated,
        thus there is no need for you to do it yourself.
    @par
        Usage (example) "OK":
            PsoCacheHelper psoCache( renderSystem ); //Save this variable (i.e. per class)

            void render()
            {
                psoCache.clearState();
                psoCache.setRenderTarget( renderTarget );
                for( int i=0; i<numMaterials; ++i )
                {
                    psoCache.setMacroblock( material[i].macroblock );
                    psoCache.setBlendblock( material[i].blendblock );
                    psoCache.setVertexShader( material[i].vertexShader );
                    psoCache.setPixelShader( material[i].pixelShader );
                    for( int j=0; j<numThingsToRenderPerMaterial; ++j )
                    {
                        v1::RenderOperation renderOp = renderables[j].renderOp;
                        //Consider caching 'vertexElements' somewhere as
                        //convertToV2 involves allocations
                        VertexElement2VecVec vertexElements = renderOp.vertexData->
                                vertexDeclaration->convertToV2();
                        psoCache.setVertexFormat( vertexElements,
                                                  renderOp.operationType,
                                                  enablePrimitiveRestart );

                        HlmsPso *pso = psoCache.getPso();
                        renderSystem->_setPipelineStateObject( pso );
                    }
                }
            }

        Usage "MUCH better":
            PsoCacheHelper psoCache( renderSystem ); //Save this variable (i.e. per class)

            //Outside rendering, just ONCE at loading time or when
            //the material changes (or the vertex layout changes):
            psoCache.clearState();
            psoCache.setMacroblock( material[i].macroblock );
            psoCache.setBlendblock( material[i].blendblock );
            psoCache.setVertexShader( material[i].vertexShader );
            psoCache.setPixelShader( material[i].pixelShader );
            for( int j=0; j<numThingsToRenderPerMaterial; ++j )
            {
                v1::RenderOperation renderOp = renderables[j].renderOp;
                //Consider caching 'vertexElements' somewhere as
                //convertToV2 involves allocations
                VertexElement2VecVec vertexElements = renderOp.vertexData->
                        vertexDeclaration->convertToV2();
                psoCache.setVertexFormat( vertexElements,
                                          renderOp.operationType,
                                          enablePrimitiveRestart );

                renderables[j].customSavedValue = psoCache.getRenderableHash();
            }

            //During render:
            void render()
            {
                psoCache.clearState();
                //setRenderTarget still needs to be called.
                psoCache.setRenderTarget( renderTarget );
                for( int i=0; i<numMaterials; ++i )
                {
                    for( int j=0; j<numThingsToRenderPerMaterial; ++j )
                    {
                        HlmsPso *pso = psoCache.getPso( renderables[j].customSavedValue );
                        renderSystem->_setPipelineStateObject( pso );
                    }
                }
            }
    */
    class _OgreExport PsoCacheHelper : public PassAlloc
    {
        static const uint32 RenderableBits;
        static const uint32 PassBits;
        static const uint32 RenderableMask;
        static const uint32 PassMask;

        struct PsoCacheEntry
        {
            uint32  hash;
            HlmsPso pso;

            bool operator ()( uint32 _l, const PsoCacheEntry &_r ) const
            {
                return _l < _r.hash;
            }
            bool operator ()( const PsoCacheEntry &_l, uint32 _r ) const
            {
                return _l.hash < _r;
            }
            bool operator ()( const PsoCacheEntry &_l, const PsoCacheEntry &_r ) const
            {
                return _l.hash < _r.hash;
            }
        };
        struct PassCacheEntry
        {
            HlmsPassPso passKey;
            uint32      hashToMainCache;

            bool operator < ( const PassCacheEntry &_r ) const
            {
                return this->passKey < _r.passKey;
            }
        };
        struct RenderableCacheEntry
        {
            HlmsPso psoRenderableKey;
            uint32  hashToMainCache;

            bool operator < ( const RenderableCacheEntry &_r ) const
            {
                return this->psoRenderableKey.lessThanExcludePassData(_r.psoRenderableKey );
            }
        };
        typedef vector<PsoCacheEntry>::type PsoCacheEntryVec;
        typedef vector<PassCacheEntry>::type PassCacheEntryVec;
        typedef vector<RenderableCacheEntry>::type RenderableCacheEntryVec;

        PsoCacheEntryVec        mPsoCache;
        PassCacheEntryVec       mPassCache;
        RenderableCacheEntryVec mRenderableCache;

        uint32  mPassHashCounter;
        uint32  mRenderableHashCounter;

        uint32      mCurrentPassHash;
        uint32      mLastFinalHash;
        HlmsPso     mCurrentState;
        HlmsPso     *mLastPso;

        RenderSystem *mRenderSystem;

        uint32 getPassHash(void);

    public:
        PsoCacheHelper( RenderSystem *renderSystem );
        ~PsoCacheHelper();

        void clearState(void);

        /** Returns a hash value you can cache into a Renderable (or whatever you're rendering).
            This hash will contain VertexFormat, Macroblock, Blendblock information and shaders
            set, but nothing that is part of the pass (RenderTargets attached, RTT formats,
            stencil parameters, etc).
            You would later insert this value to getPso( renderableHash ); See examples
            in the class description.
        @remarks
            The hash is deterministic, but depends on the order in which they're created.
            The hash is immune to collisions (unless you overflow 2^RenderableBits).
        @par
            Do NOT call this function every frame per object. The goal is that you only
            call this function when the object is being loaded (i.e. once) or when
            the object has changed (e.g. new material, vertex definition information was
            changed).
        @par
            If you can't perform this optimization, just set every state for every object
            every frame (like in the first example) and call getPso.
        @return
            The hash containing all the information.
        */
        uint32 getRenderableHash(void);

        /** You must call this function every frame, and every time the RenderTarget changes.
        @remarks
            Warning: This function will not check for redundant state changes
            (there's a lot that could've changed: stencil settings, depth buffer, etc)

            This function is a PASS state changing function, and its information is
            not part of getRenderableHash.
        */
        void setRenderTarget( RenderTarget *renderTarget );

        /// This function can be skipped if no vertex buffer is used (e.g.
        /// you use gl_VertexID or other trickery) or if you have a renderableHash.
        void setVertexFormat( const VertexElement2VecVec &vertexElements,
                              OperationType operationType,
                              bool enablePrimitiveRestart );

        /// Calls to this function cannot be skipped unless you have a renderableHash.
        void setMacroblock( const HlmsMacroblock *macroblock );
        /// @copydoc setMacroblock
        void setBlendblock( const HlmsBlendblock *blendblock );

        /// Calls to this function can be skipped if it's valid to not have
        /// a shader set at this stage, or if you have a renderableHash.
        void setVertexShader( GpuProgramPtr &shader );
        /// @copydoc setVertexShader
        void setPixelShader( GpuProgramPtr &shader );

        /** Returns an HlmsPso you can set via renderSystem->_setPipelineStateObject
            based on the input hash calculated via getRenderableHash.
        @remarks
            Don't call getRenderableHash for every object every frame and then this function.
            That's inefficient. If you can't save the hash at loading time / when materials
            are set, just call the other getPso overload directly.
        @param renderableHash
            The hash obtained from getRenderableHash which
            you should've saved in the Renderable.
        @param renderableCacheAlreadySet
            Internal parameter. Leave the default.
            Used by the other getPso overload to tell there is no need to perform a linear
            O(N) search in mRenderableCache because mCurrentState is up to date (this search
            only happens if the PSO hadn't been already cached).
        @return
            The HlmsPso to use. Do NOT persistently store this pointer. It may
            be invalidated if the next getPso call needs to create a new PSO.
        */
        HlmsPso* getPso( uint32 renderableHash, bool renderableCacheAlreadySet=false );

        /** Returns an HlmsPso you can set via renderSystem->_setPipelineStateObject
            based on all past calls to setRenderTarget + setVertexFormat + setMacroblock + etc.
            Use this version if for architecture reasons (i.e. legacy code, time constraints)
            you can't store the hash at loading time into the Renderable for later reuse.
        @return
            The HlmsPso to use. Do NOT persistently store this pointer. It may
            be invalidated if the next getPso call needs to create a new PSO.
        */
        HlmsPso* getPso(void);
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
