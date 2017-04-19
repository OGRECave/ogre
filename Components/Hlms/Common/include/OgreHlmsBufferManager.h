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
#ifndef _OgreHlmsBufferManager_H_
#define _OgreHlmsBufferManager_H_

#include "OgrePrerequisites.h"
#include "OgreHlms.h"
#include "OgreHeaderPrefix.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#   if defined( OGRE_STATIC_LIB ) || defined( OGRE_PBS_STATIC_LIB ) || defined( OGRE_UNLIT_STATIC_LIB )
#       define _OgreHlmsCommonExport
#   else
#       if defined( OgreHlmsPbs_EXPORTS ) || defined( OgreHlmsUnlit_EXPORTS )
#           define _OgreHlmsCommonExport __declspec( dllexport )
#       else
#           if defined( __MINGW32__ )
#               define _OgreHlmsCommonExport
#           else
#               define _OgreHlmsCommonExport __declspec( dllimport )
#           endif
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#   define _OgreHlmsCommonExport __attribute__ ((visibility("default")))
#else
#   define _OgreHlmsCommonExport
#endif

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    /** Managing constant and texture buffers for sending shader parameters
        is a very similar process to most Hlms implementations using them.
        This class offers the shared functionality for them, such as
            1. Rebinding buffers when necessary, with the right offsets and sizes.
            2. Requesting more memory.
            3. Mapping it.
    */
    class _OgreHlmsCommonExport HlmsBufferManager : public Hlms
    {
    protected:
        typedef vector<ConstBufferPacked*>::type ConstBufferPackedVec;
        typedef vector<TexBufferPacked*>::type TexBufferPackedVec;

        VaoManager              *mVaoManager;

        uint32                  mCurrentConstBuffer;    /// Resets every to zero every new frame.
        uint32                  mCurrentTexBuffer;      /// Resets every to zero every new frame.
        ConstBufferPackedVec    mConstBuffers;
        TexBufferPackedVec      mTexBuffers;

        uint32  *mStartMappedConstBuffer;
        uint32  *mCurrentMappedConstBuffer;
        size_t  mCurrentConstBufferSize;

        /// Holds ptr to the start of the mapped region
        float   *mRealStartMappedTexBuffer;
        /// Holds ptr to the start of the **bound** region to the shader slot.
        /// It is always mStartMappedTexBuffer >= mRealStartMappedTexBuffer
        float   *mStartMappedTexBuffer;
        float   *mCurrentMappedTexBuffer;
        /// Bindable size left.
        size_t  mCurrentTexBufferSize;

        /** Holds the offset at which all tex. binds should start from.
            Resets every to zero every new buffer (@see unmapTexBuffer
            and @see mapNextTexBuffer).
        @remarks
            The texture buffer has three location we need to track for:
                * Where the buffer mapping starts (i.e. beginning of each render_pass)
                  Tracked via mRealStartMappedTexBuffer
                * Where the buffer latest bind point starts (i.e. each time we exceed
                  the HW limit for const buffers, usually 64kb; or rendering between
                  render queue IDs). Tracked via mStartMappedTexBuffer.
                * How much data we've written so far for the current texture buffer,
                  tracked via mTexLastOffset.
        */
        size_t  mTexLastOffset;

        /// Stores the offset to the last command buffer's binding command so we can
        /// write the amount of bytes that should be bound (which is only known after
        /// we've written them).
        size_t  mLastTexBufferCmdOffset;

        /// The tex. buffer's size. Try raising this number if your API traces/profilers
        /// show we're constantly binding new textures. Should only be relevant if you
        /// have many skeletally animated meshes with lots of bones.
        size_t mTextureBufferDefaultSize;

        /// For compatibility reasons with D3D11 and GLES3, Const buffers are mapped.
        /// Once we're done with it (even if we didn't fully use it) we discard it
        /// and get a new one. We will at least have to get a new one on every pass.
        /// This is affordable since common Const buffer limits are of 64kb.
        /// At the next frame we restart mCurrentConstBuffer to 0.
        void unmapConstBuffer(void);

        /// Warning: Calling this function affects BOTH mCurrentConstBuffer and mCurrentTexBuffer
        uint32* RESTRICT_ALIAS_RETURN mapNextConstBuffer( CommandBuffer *commandBuffer );

        /// Texture buffers are treated differently than Const buffers. We first map it.
        /// Once we're done with it, we save our progress (in mTexLastOffset) and in the
        /// next pass start where we left off (i.e. if we wrote to the first 2MB chunk,
        /// start mapping from 2MB onwards). Only when the buffer is full, we get a new
        /// Tex Buffer.
        /// At the next frame we restart mCurrentTexBuffer to 0.
        ///
        /// Tex Buffers can be as big as 128MB, thus "restarting" with another 128MB
        /// buffer on every pass is too expensive. This strategy benefits low level RS
        /// like GL3+ and D3D11.1* (Windows 8) and D3D12; whereas on D3D11 and GLES3
        /// drivers dynamic mapping may discover we're writing to a region not in use
        /// or may internally use a new buffer (wasting memory space).
        ///
        /// (*) D3D11.1 allows using MAP_NO_OVERWRITE for texture buffers.
        void unmapTexBuffer( CommandBuffer *commandBuffer );
        float* RESTRICT_ALIAS_RETURN mapNextTexBuffer( CommandBuffer *commandBuffer,
                                                       size_t minimumSizeBytes );

        /** Rebinds the texture buffer. Finishes the last bind command to the tbuffer.
        @param resetOffset
            When true, the tbuffer will be offsetted so that the shader samples
            from 0 at the current offset in mCurrentMappedTexBuffer
            WARNING: mCurrentMappedTexBuffer may be modified due to alignment.
            mStartMappedTexBuffer & mCurrentTexBufferSize will always be modified
        @param minimumTexBufferSize
            If resetOffset is true and the remaining space in the currently mapped
            tbuffer is less than minimumSizeBytes, we will call mapNextTexBuffer
        */
        void rebindTexBuffer( CommandBuffer *commandBuffer, bool resetOffset = false,
                              size_t minimumSizeBytes = 1 );

        virtual void destroyAllBuffers(void);

    public:
        HlmsBufferManager( HlmsTypes type, const String &typeName, Archive *dataFolder,
                           ArchiveVec *libraryFolders );
        ~HlmsBufferManager();

        virtual void _changeRenderSystem( RenderSystem *newRs );

        virtual HlmsCache preparePassHash( const Ogre::CompositorShadowNode *shadowNode,
                                           bool casterPass, bool dualParaboloid,
                                           SceneManager *sceneManager );

        virtual void preCommandBufferExecution( CommandBuffer *commandBuffer );
        virtual void postCommandBufferExecution( CommandBuffer *commandBuffer );

        virtual void frameEnded(void);

        /// Changes the default suggested size for the texture buffer.
        /// Actual size may be lower if the GPU can't honour the request.
        void setTextureBufferDefaultSize( size_t defaultSize );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
