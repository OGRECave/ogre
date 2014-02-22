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

#ifndef __CompositorPassQuadDef_H__
#define __CompositorPassQuadDef_H__

#include "OgreHeaderPrefix.h"

#include "../OgreCompositorPassDef.h"
#include "OgreCommon.h"

namespace Ogre
{
    class CompositorNodeDef;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    class _OgreExport CompositorPassQuadDef : public CompositorPassDef
    {
    public:
        struct QuadTextureSource
        {
            /// Index of texture unit state to change
            size_t      texUnitIdx;
            /// Name of the texture (can come from input channel, local textures, or global ones)
            IdString    textureName;
            /// Index in case of MRT. Ignored if textureSource isn't mrt
            size_t      mrtIndex;

            QuadTextureSource( size_t _texUnitIdx, IdString _textureName, size_t _mrtIndex ) :
                texUnitIdx( _texUnitIdx ), textureName( _textureName ), mrtIndex( _mrtIndex ) {}
        };
        typedef vector<QuadTextureSource>::type TextureSources;

    protected:
        TextureSources      mTextureSources;
        CompositorNodeDef   *mParentNodeDef;

    public:
        enum FrustumCorners
        {
            NO_CORNERS,
            VIEW_SPACE_CORNERS,
            WORLD_SPACE_CORNERS,
        };

        /** Whether to use a full screen quad or triangle. (default: false). Note that you may not
            always get the triangle (for example, if you ask for WORLD_SPACE_CORNERS)
        */
        bool    mUseQuad;

        /** When true, the user is telling Ogre this pass just performs a custom FSAA resolve filter.
            Hence we should skip this pass for those APIs that don't support explicit resolving
            TODO: Not really implemented yet!!!
        @remarks
            @See TextureDefinitionBase::TextureDefinition::fsaaExplicitResolve
        */
        bool    mIsResolve;

        String  mMaterialName;

        /** Type of frustum corners to pass in the quad normals.
            mCameraName contains which camera's frustum to pass
        */
        FrustumCorners  mFrustumCorners;
        IdString        mCameraName;

        CompositorPassQuadDef( CompositorNodeDef *parentNodeDef, uint32 rtIndex ) :
            CompositorPassDef( PASS_QUAD, rtIndex ),
            mParentNodeDef( parentNodeDef ),
            mUseQuad( false ),
            mIsResolve( false ),
            mFrustumCorners( NO_CORNERS )
        {
        }

        /** Indicates the pass to change the texture units to use the specified texture sources.
            @See QuadTextureSource for params
        */
        void addQuadTextureSource( size_t texUnitIdx, const String &textureName, size_t mrtIndex );

        const TextureSources& getTextureSources(void) const     { return mTextureSources; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
