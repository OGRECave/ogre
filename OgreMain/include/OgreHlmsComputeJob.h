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
#include "OgreResourceTransition.h"
#include "OgreShaderParams.h"
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

    public:
        enum ThreadGroupsBasedOn
        {
            /// Disabled. (obey setNumThreadGroups)
            ThreadGroupsBasedOnNothing,
            /// Based the number of thread groups on a texture. See setNumThreadGroupsBasedOn
            ThreadGroupsBasedOnTexture,
            /// Based the number of thread groups on a UAV. See setNumThreadGroupsBasedOn
            ThreadGroupsBasedOnUav,
        };

    protected:
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
            BufferPacked *buffer;
            size_t offset;
            size_t sizeBytes;
            TexturePtr texture;
            /// Samplerblock is only used by regular textures (not UAVs or buffers)
            HlmsSamplerblock const *samplerblock;

            //Used by UAVs:

            ResourceAccess::ResourceAccess access;
            int32           mipmapLevel;
            int32           textureArrayIndex;
            PixelFormat     pixelFormat;

            TextureSlot() :
                buffer( 0 ), offset( 0 ), sizeBytes( 0 ), samplerblock( 0 ),
                access( ResourceAccess::Undefined ), mipmapLevel( 0 ), textureArrayIndex( 0 ),
                pixelFormat( PF_UNKNOWN ) {}
        };

        typedef vector<ConstBufferSlot>::type ConstBufferSlotVec;
        typedef vector<TextureSlot>::type TextureSlotVec;

        Hlms    *mCreator;
        IdString mName;

        String          mSourceFilename;
        StringVector    mIncludedPieceFiles;

        /// See setThreadsPerGroup
        uint32  mThreadsPerGroup[3];
        /// See setNumThreadGroups
        uint32  mNumThreadGroups[3];

        ThreadGroupsBasedOn mThreadGroupsBasedOnTexture;
        uint8               mThreadGroupsBasedOnTexSlot;
        uint8               mThreadGroupsBasedDivisor[3];

        ConstBufferSlotVec  mConstBuffers;
        TextureSlotVec      mTextureSlots;
        TextureSlotVec      mUavSlots;

        bool mInformHlmsOfTextureData;
        uint8 mMaxTexUnitReached;
        uint8 mMaxUavUnitReached;
        HlmsPropertyVec mSetProperties;
        /// Don't add or remove directly! See setPiece and see removePiece
        PiecesMap       mPieces;
        size_t          mPsoCacheHash;

        map<IdString, ShaderParams>::type mShaderParams;

        void updateAutoProperties( const TextureSlotVec &textureSlots,
                                   uint8 &outMaxTexUnitReached,
                                   const char *propTexture,
                                   const IdString &propNumTextureSlots,
                                   const IdString &propMaxTextureSlot );

        void removeProperty( IdString key );

        void setBuffer( uint8 slotIdx, BufferPacked *buffer,
                        size_t offset, size_t sizeBytes,
                        ResourceAccess::ResourceAccess access, TextureSlotVec &container );

    public:
        HlmsComputeJob( IdString name, Hlms *creator, const String &sourceFilename,
                        const StringVector &includedPieceFiles );
        virtual ~HlmsComputeJob();

        Hlms* getCreator(void) const                { return mCreator; }

        IdString getName(void) const                { return mName; }

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
        uint32 getThreadsPerGroupX(void) const          { return mThreadsPerGroup[0]; }
        uint32 getThreadsPerGroupY(void) const          { return mThreadsPerGroup[1]; }
        uint32 getThreadsPerGroupZ(void) const          { return mThreadsPerGroup[2]; }
        const uint32* getThreadsPerGroup(void) const    { return mThreadsPerGroup; }

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
        uint32 getNumThreadGroupsX(void) const          { return mNumThreadGroups[0]; }
        uint32 getNumThreadGroupsY(void) const          { return mNumThreadGroups[1]; }
        uint32 getNumThreadGroupsZ(void) const          { return mNumThreadGroups[2]; }
        const uint32* getNumThreadGroups(void) const    { return mNumThreadGroups; }

        /** Instead of calling setNumThreadGroups, Ogre can automatically deduce
            them based on the Texture resolution and the threads per group.
            It is calculated as follows:
                scaledWidth = (textureWidth + divisorX - 1u) / divisorX;
                numThreadGroupsX = (scaledWidth + threadsPerGroupX - 1u) / threadsPerGroupX;
        @remarks
            Unless disabled, this will overwrite your setNumThreadGroups based on the
            texture bound at the time the job is dispatched.
        @par
            If no texture/uav is bound at the given slot (or no such slot exists), we
            will log a warning.
        @param source
            What to use as source for the calculations. See ThreadGroupsBasedOn
        @param texSlot
            Index of the texture/uav unit.
        @param divisorX divisorY divisorZ
            Often compute shaders operate on multiple pixels, thus you need less
            thread groups. For example if you operate on blocks of 2x2, then you
            want divisorX = 2 and divisorY = 2.
        */
        void setNumThreadGroupsBasedOn( ThreadGroupsBasedOn source, uint8 texSlot,
                                        uint8 divisorX, uint8 divisorY, uint8 divisorZ );

        /// INTERNAL USE. Calculates the number of thread groups as specified
        /// in setNumThreadGroupsBasedOn, overriding setNumThreadGroups.
        void _calculateNumThreadGroupsBasedOnSetting();

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

        /** Defines a piece, i.e. the same as doing @piece( pieceName )pieceContent@end
            If the piece doesn't exist, it gets created.
            If the piece already exists, it gets overwritten.
        @remarks
            Because we need to efficiently track changes (to know when to recompile, when
            we can reuse a cached shader, etc), we store a property of the same
            name as pieceName with the hash of the piece's content as value.
            e.g. doing setPiece( pieceName, pieceContent ) implies calling
            setProperty( pieceName, hash( pieceContent ).
            Hence you should NOT manipulate mPieces directly, otherwise we won't
            see changes performed to it, or use shaders from a cache we shouldn't
            use.
        @param pieceName
            Name of the piece.
        @param pieceContent
            The contents of the piece.
        */
        void setPiece( IdString pieceName, const String &pieceContent );

        /// Removes an existing piece. See setPiece.
        /// Does nothing if the piece didn't exist.
        void removePiece( IdString pieceName );

        /// Creates a set of shader paramters with a given key,
        /// e.g. "default" "glsl" "hlsl".
        /// Does nothing if parameters already exist.
        void createShaderParams( IdString key );

        /// Gets a shader parameter with the given key.
        /// e.g. "default" "glsl" "hlsl".
        /// Creates if does not exist.
        ShaderParams& getShaderParams( IdString key );

        /// Gets a shader parameter with the given key.
        /// e.g. "default" "glsl" "hlsl".
        /// Returns null if doesn't exist. See createShaderParams
        ShaderParams* _getShaderParams( IdString key );

        /** Sets a const/uniform bufferat the given slot ID.
        @param slotIdx
            Slot to bind to. It's independent from the texture & UAV ones.
        @param constBuffer
            Const buffer to bind.
        */
        void setConstBuffer( uint8 slotIdx, ConstBufferPacked *constBuffer );

        /// Creates 'numSlots' number of slots before they can be set.
        void setNumTexUnits( uint8 numSlots );
        /// Destroys a given texture unit, displacing all the higher tex units.
        void removeTexUnit( uint8 slotIdx );
        size_t getNumTexUnits(void) const               { return mTextureSlots.size(); }

        const TexturePtr& getTexture( uint8 slotIdx ) const;

        /// @copydoc setNumTexUnits
        void setNumUavUnits( uint8 numSlots );
        /// @copydoc removeTexUnit
        void removeUavUnit( uint8 slotIdx );
        size_t getNumUavUnits(void) const               { return mUavSlots.size(); }

        const TexturePtr& getUavTexture( uint8 slotIdx ) const;
        UavBufferPacked* getUavBuffer( uint8 slotIdx ) const;

        /** Sets a texture buffer at the given slot ID.
        @remarks
            Texture buffer slots are shared with setTexture's. Calling this
            function will remove the settings from previous setTexture calls
            to the same slot index.
        @par
            May trigger a recompilation if setInformHlmsOfTextureData
            is enabled.
        @par
            Setting a RenderTarget that could be used for writing is dangerous
            in explicit APIs (DX12, Vulkan). Use the CompositorPassComputeDef
        @param slotIdx
            See setNumTexUnits.
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
            UAVs and Textures share the same slots in OpenGL, but don't in
            D3D11. For best compatibility, assume they're shared and put
            the UAVs in the first slots.
        @par
            May trigger a recompilation if setInformHlmsOfTextureData
            is enabled.
        @param slotIdx
            See setNumTexUnits.
            The slot index to bind this texture
            In OpenGL, some cards support up to 16-18 texture units, while most
            cards support up to 32
        @param texBuffer
            Texture buffer to bind.
        @param texture
            Texture to bind.
        @param samplerblock
            Optional. We'll create (or retrieve an existing) samplerblock based on the input parameters.
            When null, we leave the previously set samplerblock (if a texture is being set, and if no
            samplerblock was set, we'll create a default one)
        */
        void setTexture( uint8 slotIdx, TexturePtr &texture,
						 const HlmsSamplerblock *refParams=0 );

        /** Sets a samplerblock based on reference parameters
        @param slotIdx
            See setNumTexUnits.
        @param refParams
            We'll create (or retrieve an existing) samplerblock based on the input parameters.
        */
        void setSamplerblock( uint8 slotIdx, const HlmsSamplerblock &refParams );

        /** Sets a samplerblock directly. For internal use / advanced users.
        @param slotIdx
            See setNumTexUnits.
        @param refParams
            Direct samplerblock. Reference count is assumed to already have been increased.
            We won't increase it ourselves.
        @param params
            The sampler block to use as reference.
        */
        void _setSamplerblock( uint8 slotIdx, const HlmsSamplerblock *refParams );

        /** Sets an UAV buffer at the given slot ID.
        @remarks
            UAV slots are shared with setUavTexture. Calling this
            function will remove the settings from previous setUavTexture calls
            to the same slot index.
        @par
            May trigger a recompilation if setInformHlmsOfTextureData
            is enabled.
        @par
            Be very careful when calling this directly. The Compositor needs to
            evaluate memory barriers and resource transitions. Leaving inconsistent
            memory barriers can result in hazards/race conditions in some APIs.
            If in doubt, change the CompositorPassComputeDef instead.
        @param slotIdx
            See setNumUavUnits.
            The slot index to bind this UAV buffer.
        @param access
            Access. Should match what the shader expects. Needed by Ogre to
            resolve memory barrier dependencies.
        @param uavBuffer
            UAV buffer to bind.
        @param offset
            0-based offset. It is possible to bind a region of the buffer.
            Offset needs to be aligned. You can query the RS capabilities for
            the alignment, however 256 bytes is the maximum allowed alignment
            per the OpenGL specification, making it a safe bet to hardcode.
        @param sizeBytes
            Size in bytes to bind the tex buffer. When zero,
            binds from offset until the end of the buffer.
        */
        void _setUavBuffer( uint8 slotIdx, UavBufferPacked *uavBuffer,
                            ResourceAccess::ResourceAccess access,
                            size_t offset=0, size_t sizeBytes=0 );

        /** Sets an UAV texture.
        @remarks
            UAV buffer slots are shared with setUavTexture's. Calling this
            function will remove the settings from previous setUavBuffer calls
            to the same slot index.
        @par
            May trigger a recompilation if setInformHlmsOfTextureData
            is enabled.
        @par
            Be very careful when calling this directly. The Compositor needs to
            evaluate memory barriers and resource transitions. Leaving inconsistent
            memory barriers can result in hazards/race conditions in some APIs.
            If in doubt, change the CompositorPassComputeDef instead.
        @param slot
            See setNumUavUnits.
        @param texture
        @param textureArrayIndex
        @param access
        @param mipmapLevel
        @param pixelFormat
        */
        void _setUavTexture( uint8 slotIdx, TexturePtr &texture, int32 textureArrayIndex,
                             ResourceAccess::ResourceAccess access, int32 mipmapLevel,
                             PixelFormat pixelFormat );

        HlmsComputeJob *clone( const String &cloneName );
        void cloneTo( HlmsComputeJob *dstJob );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
