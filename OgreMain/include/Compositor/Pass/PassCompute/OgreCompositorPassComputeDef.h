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

#ifndef _OgreCompositorPassComputeDef_H_
#define _OgreCompositorPassComputeDef_H_

#include "OgreHeaderPrefix.h"

#include "../OgreCompositorPassDef.h"
#include "OgreCommon.h"
#include "OgrePixelFormat.h"

namespace Ogre
{
    class CompositorNodeDef;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

	class _OgreExport CompositorPassComputeDef : public CompositorPassDef
    {

    public:
		struct ComputeTextureSource
        {
            /// Index of texture unit state to change
            uint32      texUnitIdx;
            /// Name of the texture (can come from input channel, local textures, or global ones)
            IdString    textureName;
            /// Index in case of MRT. Ignored if textureSource isn't mrt
            uint32      mrtIndex;

            //Used by UAVs
            ResourceAccess::ResourceAccess access;
            int32           mipmapLevel;
            int32           textureArrayIndex;
            PixelFormat     pixelFormat;
            bool            allowWriteAfterWrite;

			ComputeTextureSource( size_t _texUnitIdx, IdString _textureName, size_t _mrtIndex ) :
                texUnitIdx( _texUnitIdx ), textureName( _textureName ), mrtIndex( _mrtIndex ),
                access( ResourceAccess::Undefined ), mipmapLevel( 0 ), textureArrayIndex( 0 ),
                pixelFormat( PF_UNKNOWN ), allowWriteAfterWrite( false ) {}

            ComputeTextureSource( size_t _texUnitIdx, IdString _textureName, size_t _mrtIndex,
                                  ResourceAccess::ResourceAccess _access, int32 _mipmapLevel,
                                  int32 _textureArrayIndex, PixelFormat _pixelFormat,
                                  bool _allowWriteAfterWrite ) :
                texUnitIdx( _texUnitIdx ), textureName( _textureName ), mrtIndex( _mrtIndex ),
                access( _access ), mipmapLevel( _mipmapLevel ), textureArrayIndex( _textureArrayIndex ),
                pixelFormat( _pixelFormat ), allowWriteAfterWrite( _allowWriteAfterWrite ) {}
        };
        typedef vector<ComputeTextureSource>::type TextureSources;

        struct BufferSource
        {
            uint32      slotIdx;
            IdString    bufferName;
            ResourceAccess::ResourceAccess access;
            size_t      offset;
            size_t      sizeBytes;
            bool        allowWriteAfterWrite;
            //PixelFormat pixelFormat; /// PF_UNKNOWN if used as UAV.

            BufferSource( uint32 _slotIdx, IdString _bufferName,
                          ResourceAccess::ResourceAccess _access, size_t _offset=0,
                          size_t _sizeBytes=0, bool _allowWriteAfterWrite=false ) :
                slotIdx( _slotIdx ), bufferName( _bufferName ), access( _access ), offset( _offset ),
                sizeBytes( _sizeBytes ), allowWriteAfterWrite( _allowWriteAfterWrite ) {}
        };
        typedef vector<BufferSource>::type BufferSourceVec;

    protected:
        TextureSources      mTextureSources;
        TextureSources      mUavSources;
        BufferSourceVec     mBufferSources;
        CompositorNodeDef   *mParentNodeDef;

    public:
        /// Name of the HlmsComputeJob to run.
        IdString mJobName;
        IdString mCameraName;

        CompositorPassComputeDef( CompositorNodeDef *parentNodeDef,
                                  CompositorTargetDef *parentTargetDef ) :
            CompositorPassDef( PASS_COMPUTE, parentTargetDef ),
            mParentNodeDef( parentNodeDef )
        {
        }

        /** Indicates the pass to change the texture units to use the specified texture sources.
            @See ComputeTextureSource for params
        */
        void addTextureSource( uint32 texUnitIdx, const String &textureName, uint32 mrtIndex );

        void addUavSource( uint32 texUnitIdx, const String &textureName, uint32 mrtIndex,
                           ResourceAccess::ResourceAccess access, int32 textureArrayIndex,
                           int32 mipmapLevel, PixelFormat pixelFormat, bool allowWriteAfterWrite );

//        void addTexBuffer( uint32 slotIdx, const String &bufferName,
//                           size_t offset=0, size_t sizeBytes=0 );
        void addUavBuffer( uint32 slotIdx, const String &bufferName,
                           ResourceAccess::ResourceAccess access, size_t offset=0,
                           size_t sizeBytes=0, bool allowWriteAfterWrite=false );

        const TextureSources& getTextureSources(void) const     { return mTextureSources; }
        const TextureSources& getUavSources(void) const         { return mUavSources; }
        const BufferSourceVec& getBufferSources(void) const     { return mBufferSources; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
