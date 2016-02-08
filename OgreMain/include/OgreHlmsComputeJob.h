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
#ifndef _OgreHlmsComputeJob_H_
#define _OgreHlmsComputeJob_H_

#include "OgreHlmsDatablock.h"
#include "OgreMatrix4.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Material
    *  @{
    */

    //class _OgreExport HlmsComputeJob : public HlmsDatablock
    class _OgreExport HlmsComputeJob : public PassAlloc
    {
        friend class HlmsCompute;

        struct ConstBufferSlot
        {
            uint8 slotIdx;
            ConstBufferPacked *buffer;

            bool operator () ( const ConstBufferSlot &left, uint8 right ) const
            {
                return left.slotIdx < right;
            }
            bool operator () ( uint8 left, const ConstBufferSlot &right ) const
            {
                return left < right.slotIdx;
            }
            bool operator () ( const ConstBufferSlot &left, const ConstBufferSlot &right ) const
            {
                return left.slotIdx < right.slotIdx;
            }
        };

        struct TextureSlot
        {
            uint8 slotIdx;
            TexBufferPacked *buffer;
            size_t offset;
            size_t sizeBytes;
            TexturePtr texture;
            HlmsSamplerblock const *samplerblock;

            bool operator () ( const TextureSlot &left, uint8 right ) const
            {
                return left.slotIdx < right;
            }
            bool operator () ( uint8 left, const TextureSlot &right ) const
            {
                return left < right.slotIdx;
            }
            bool operator () ( const TextureSlot &left, const TextureSlot &right ) const
            {
                return left.slotIdx < right.slotIdx;
            }
        };

        typedef vector<ConstBufferSlot>::type ConstBufferSlotVec;
        typedef vector<TextureSlot>::type TextureSlotVec;

        Hlms    *mCreator;
        IdString mName;

        String          mSourceFilename;
        StringVector    mIncludedPieceFiles;

        /// @see setThreadsPerGroup
        uint32  mThreadsPerGroup[3];
        /// @see setNumThreadGroups
        uint32  mNumThreadGroups[3];

        ConstBufferSlotVec  mConstBuffers;
        TextureSlotVec      mTextureSlots;

        bool mInformHlmsOfTextureData;
        uint8 mMaxTexUnitReached;
        HlmsPropertyVec mSetProperties;
        size_t          mPsoCacheHash;

        void removeProperty( IdString key );

    public:
        HlmsComputeJob( IdString name, Hlms *creator, const String &sourceFilename,
                        const StringVector &includedPieceFiles );
        virtual ~HlmsComputeJob();

        void _updateAutoProperties(void);

        /** The Hlms has the ability to pass data to the shader source
            code via its syntax system to add hardcoded values.
            For example, you may want to unroll a loop based on the width
            of a texture for maximum performance.
        @par
            Enabling this feature informs the Hlms to reparse the shader
            on any change to bound textures that could trigger a recompilation.
            If you don't need it, keep this feature disabled to prevent
            unnecessary recompilations.
        @param bInformHlms
            True to enable this feature, false to disable.
        */
        void setInformHlmsOfTextureData( bool bInformHlms );

        /** Sets the number of threads per group. Note the actual value may be
            changed by the shader template using the @pset() function.
            These values are passed to the template as:
                * threads_per_group_x
                * threads_per_group_y
                * threads_per_group_z
        @remarks
            May trigger a recompilation if the value changes, regardless of
            what setInformHlmsOfTextureData says.
            There may be API / HW limitations on the max values for each
            dimension.
        */
        void setThreadsPerGroup( uint32 threadsPerGroupX, uint32 threadsPerGroupY, uint32 threadsPerGroupZ );

        /** Sets the number of groups of threads to dispatch. Note the actual value may be
            changed by the shader template using the @pset() function.
            These values are passed to the template as:
                * num_thread_groups_x
                * num_thread_groups_y
                * num_thread_groups_z
        @remarks
            As an example, it's typical to do:
                numThreadGroupsX = ceil( threadsPerGroupX / image.width );
                numThreadGroupsY = ceil( threadsPerGroupY / image.height );
        @par
            May trigger a recompilation if the value changes, regardless of
            what setInformHlmsOfTextureData says.
            There may be API / HW limitations on the max values for each
            dimension.
        */
        void setNumThreadGroups( uint32 numThreadGroupsX, uint32 numThreadGroupsY, uint32 numThreadGroupsZ );

        /** Sets an arbitrary property to pass to the shader.
        @remarks
            Will trigger a recompilation if the value changes, regardless of
            what setInformHlmsOfTextureData says.
        @param key
            Name of the property
        @param value
            Value to set
        */
        void setProperty( IdString key, int32 value );
        int32 getProperty( IdString key, int32 defaultVal=0 ) const;

        /** Sets a const/uniform bufferat the given slot ID.
        @param slotIdx
            Slot to bind to. It's independent from the texture & UAV ones.
        @param constBuffer
            Const buffer to bind.
        */
        void setConstBuffer( uint8 slotIdx, ConstBufferPacked *constBuffer );

        /** Sets a texture buffer at the given slot ID.
        @remarks
            Texture buffer slots are shared with setTexture's. Calling this
            function will remove the settings from previous setTexture calls
            to the same slot index.
        @par
            May trigger a recompilation if @see setInformHlmsOfTextureData
            is enabled.
        @param slotIdx
            The slot index to bind this texture buffer
            In OpenGL, a few cards support between to 16-18 texture units,
            while most cards support up to 32
        @param texBuffer
            Texture buffer to bind.
        @param offset
            0-based offset. It is possible to bind a region of the buffer.
            Offset needs to be aligned. You can query the RS capabilities for
            the alignment, however 256 bytes is the maximum allowed alignment
            per the OpenGL specification, making it a safe bet to hardcode.
        @param sizeBytes
            Size in bytes to bind the tex buffer. When zero,
            binds from offset until the end of the buffer.
        */
        void setTexBuffer( uint8 slotIdx, TexBufferPacked *texBuffer,
                           size_t offset=0, size_t sizeBytes=0 );

        /** Sets a texture buffer at the given slot ID.
        @remarks
            Texture slots are shared with setTexBuffer's. Calling this
            function will remove the settings from previous setTexBuffer
            calls to the same slot index.
        @par
            To use UAVs, they must be set via uav passes in the compositor.
            UAVs and Textures share the same slots; and UAVs always take
            precedence.
            @see CompositorPassUavDef. Refer to TutorialUav01 and TutorialUav02
            in the samples to know how to do this.
        @par
            May trigger a recompilation if @see setInformHlmsOfTextureData
            is enabled.
        @param slotIdx
            The slot index to bind this texture
            In OpenGL, some cards support up to 16-18 texture units, while most
            cards support up to 32
        @param texBuffer
            Texture buffer to bind.
        @param texture
            Texture to bind.
        @param samplerblock
            Samplerblock to use.
        */
        void setTexture( uint8 slotIdx, TexturePtr &texture,
                         const HlmsSamplerblock &refParams );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
