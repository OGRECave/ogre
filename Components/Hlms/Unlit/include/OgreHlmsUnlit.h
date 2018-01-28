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
#ifndef _OgreHlmsUnlit_H_
#define _OgreHlmsUnlit_H_

#include "OgreHlmsUnlitPrerequisites.h"
#include "OgreHlmsBufferManager.h"
#include "OgreConstBufferPool.h"
#include "OgreMatrix4.h"
#include "OgreHeaderPrefix.h"
#include "OgreRoot.h"

namespace Ogre
{
    class CompositorShadowNode;
    struct QueuedRenderable;

    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    class HlmsUnlitDatablock;

    /** Implementation without lighting or skeletal animation specfically designed for
        OpenGL 3+, D3D11 and other RenderSystems which support uniform buffers.
        Useful for GUI, ParticleFXs, other misc objects that don't require lighting.
    */
    class _OgreHlmsUnlitExport HlmsUnlit : public HlmsBufferManager, public ConstBufferPool
    {
    protected:
        typedef vector<HlmsDatablock*>::type HlmsDatablockVec;

        struct PassData
        {
            Matrix4 viewProjMatrix[2];
        };

        PassData                mPreparedPass;
        ConstBufferPackedVec    mPassBuffers;
        uint32                  mCurrentPassBuffer;     /// Resets to zero every new frame.

        ConstBufferPool::BufferPool const *mLastBoundPool;

        uint32 mLastTextureHash;

        bool    mUsingExponentialShadowMaps;
        uint16  mEsmK; /// K parameter for ESM.

        virtual const HlmsCache* createShaderCacheEntry( uint32 renderableHash,
                                                         const HlmsCache &passCache,
                                                         uint32 finalHash,
                                                         const QueuedRenderable &queuedRenderable );

        virtual HlmsDatablock* createDatablockImpl( IdString datablockName,
                                                    const HlmsMacroblock *macroblock,
                                                    const HlmsBlendblock *blendblock,
                                                    const HlmsParamVec &paramVec );

        void setTextureProperty( IdString propertyName, HlmsUnlitDatablock *datablock,
                                 uint8 texType );

        virtual void calculateHashForPreCreate( Renderable *renderable, PiecesMap *inOutPieces );
        virtual void calculateHashForPreCaster( Renderable *renderable, PiecesMap *inOutPieces );

        virtual void destroyAllBuffers(void);

        FORCEINLINE uint32 fillBuffersFor( const HlmsCache *cache,
                                           const QueuedRenderable &queuedRenderable,
                                           bool casterPass, uint32 lastCacheHash,
                                           CommandBuffer *commandBuffer, bool isV1 );

    public:
        HlmsUnlit( Archive *dataFolder, ArchiveVec *libraryFolders );
        HlmsUnlit( Archive *dataFolder, ArchiveVec *libraryFolders,
                   HlmsTypes type, const String &typeName );
        virtual ~HlmsUnlit();

        virtual void _changeRenderSystem( RenderSystem *newRs );

        /// Not supported
        virtual void setOptimizationStrategy( OptimizationStrategy optimizationStrategy ) {}

        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                           bool casterPass, bool dualParaboloid,
                                           SceneManager *sceneManager );

        virtual uint32 fillBuffersFor( const HlmsCache *cache, const QueuedRenderable &queuedRenderable,
                                       bool casterPass, uint32 lastCacheHash,
                                       uint32 lastTextureHash );

        virtual uint32 fillBuffersForV1( const HlmsCache *cache,
                                         const QueuedRenderable &queuedRenderable,
                                         bool casterPass, uint32 lastCacheHash,
                                         CommandBuffer *commandBuffer );

        virtual uint32 fillBuffersForV2( const HlmsCache *cache,
                                         const QueuedRenderable &queuedRenderable,
                                         bool casterPass, uint32 lastCacheHash,
                                         CommandBuffer *commandBuffer );

        virtual void frameEnded(void);

        void setShadowSettings( bool useExponentialShadowMaps );
        bool getShadowFilter(void) const                    { return mUsingExponentialShadowMaps; }

        /// @copydoc HlmsPbs::setEsmK
        void setEsmK( uint16 K );
        uint16 getEsmK(void) const                          { return mEsmK; }

        /// @copydoc HlmsPbs::getDefaultPaths
        static void getDefaultPaths( String& outDataFolderPath, StringVector& outLibraryFoldersPaths );

#if !OGRE_NO_JSON
		/// @copydoc Hlms::_loadJson
        virtual void _loadJson( const rapidjson::Value &jsonValue, const HlmsJson::NamedBlocks &blocks,
                                HlmsDatablock *datablock, HlmsJsonListener *listener,
                                const String &additionalTextureExtension ) const;
		/// @copydoc Hlms::_saveJson
        virtual void _saveJson( const HlmsDatablock *datablock, String &outString,
                                HlmsJsonListener *listener,
                                const String &additionalTextureExtension ) const;

		/// @copydoc Hlms::_collectSamplerblocks
		virtual void _collectSamplerblocks(set<const HlmsSamplerblock*>::type &outSamplerblocks,
			const HlmsDatablock *datablock) const;
#endif
	};

 

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
