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

#ifndef __CompositorPassUavDef_H__
#define __CompositorPassUavDef_H__

#include "OgreHeaderPrefix.h"

#include "../OgreCompositorPassDef.h"
#include "OgreCommon.h"
#include "OgreTexture.h"

namespace Ogre
{
    class CompositorNodeDef;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    class _OgreExport CompositorPassUavDef : public CompositorPassDef
    {
    public:
        struct TextureSource
        {
            /// Index of texture unit state to change
            uint32      uavSlot;
            /// Name of the texture (can come from input channel, local textures, or global ones)
            /// Not used if externalTextureName is not empty
            IdString    textureName;
            /// Name of the external texture, if empty, textureName is used.
            String      externalTextureName;
            /// Index in case of MRT. Ignored if textureSource isn't mrt
            uint32      mrtIndex;

            TextureAccess   access;
            int32           mipmapLevel;
            PixelFormat     pixelFormat;

            TextureSource( uint32 _uavSlot, IdString _textureName,
                           const String &_externalTextureName,
                           uint32 _mrtIndex, TextureAccess _access,
                           int32 _mipmapLevel, PixelFormat _pixelFormat ) :
                uavSlot( _uavSlot ), textureName( _textureName ),
                externalTextureName( _externalTextureName ), mrtIndex( _mrtIndex ),
                access( _access ), mipmapLevel( _mipmapLevel ), pixelFormat( _pixelFormat ) {}
        };
        typedef vector<TextureSource>::type TextureSources;

    protected:
        TextureSources      mTextureSources;
        CompositorNodeDef   *mParentNodeDef;
        //bool                mNeedsFlush;

    public:
        bool    mKeepPreviousUavs;
        /// Max value (0xff) means don't alter it.
        uint8   mStartingSlot;

        CompositorPassUavDef( CompositorNodeDef *parentNodeDef, uint32 rtIndex ) :
            CompositorPassDef( PASS_UAV, rtIndex ),
            mParentNodeDef( parentNodeDef ),
            //mNeedsFlush( false ),
            mKeepPreviousUavs( true ),
            mStartingSlot( 0xFF )
        {
        }

        /** Indicates the pass to change the UAV slots to use the specified texture sources.
            @See RenderSystem::queueBindUAV for params.
        @param isExternal
            True if the texture is a random texture that needs to be loaded via
            TextureManager::getByName; false if it's an RTT controlled by the CompositorManager
            (i.e. a global texture, an input texture, or a local texture)
        @param textureName
            Name of the texture. When empty, it will clear the slots.
        */
        void setUav( uint32 slot, bool isExternal, const String &textureName, uint32 mrtIndex,
                     TextureAccess access, int32 mipmapLevel, PixelFormat pixelFormat );

        const TextureSources& getTextureSources(void) const     { return mTextureSources; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
