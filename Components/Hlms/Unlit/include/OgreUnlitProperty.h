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
#ifndef _OgreUnlitProperty_H_
#define _OgreUnlitProperty_H_

#include "OgreHlmsUnlitPrerequisites.h"

#include "OgreIdString.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{

	struct _OgreHlmsUnlitExport UnlitProperty
    {
        static const IdString HwGammaRead;
        static const IdString HwGammaWrite;
        static const IdString SignedIntTex;
        static const IdString MaterialsPerBuffer;
        static const IdString AnimationMatricesPerBuffer; //TODO: Seems dead
        static const IdString TextureMatrix;
        static const IdString ExponentialShadowMaps;
        static const IdString HasPlanarReflections;

        static const IdString TexMatrixCount;
        static const IdString TexMatrixCount0;
        static const IdString TexMatrixCount1;
        static const IdString TexMatrixCount2;
        static const IdString TexMatrixCount3;
        static const IdString TexMatrixCount4;
        static const IdString TexMatrixCount5;
        static const IdString TexMatrixCount6;
        static const IdString TexMatrixCount7;

        /// Whether uses material's colour.
        static const IdString Diffuse;

        /// Number of texture arrays actually baked.
        static const IdString NumArrayTextures;
        static const IdString NumTextures;

        /// Number of diffuse maps.
        static const IdString DiffuseMap;

        //static const IdString DiffuseMap0;
        //static const IdString DiffuseMap0Array;

        /// UV source # assigned to each texture.
        static const IdString UvDiffuse0;
        static const IdString UvDiffuse1;
        static const IdString UvDiffuse2;
        static const IdString UvDiffuse3;
        static const IdString UvDiffuse4;
        static const IdString UvDiffuse5;
        static const IdString UvDiffuse6;
        static const IdString UvDiffuse7;
        static const IdString UvDiffuse8;
        static const IdString UvDiffuse9;
        static const IdString UvDiffuse10;
        static const IdString UvDiffuse11;
        static const IdString UvDiffuse12;
        static const IdString UvDiffuse13;
        static const IdString UvDiffuse14;
        static const IdString UvDiffuse15;

        static const IdString UvDiffuseSwizzle0;
        static const IdString UvDiffuseSwizzle1;
        static const IdString UvDiffuseSwizzle2;
        static const IdString UvDiffuseSwizzle3;
        static const IdString UvDiffuseSwizzle4;
        static const IdString UvDiffuseSwizzle5;
        static const IdString UvDiffuseSwizzle6;
        static const IdString UvDiffuseSwizzle7;
        static const IdString UvDiffuseSwizzle8;
        static const IdString UvDiffuseSwizzle9;
        static const IdString UvDiffuseSwizzle10;
        static const IdString UvDiffuseSwizzle11;
        static const IdString UvDiffuseSwizzle12;
        static const IdString UvDiffuseSwizzle13;
        static const IdString UvDiffuseSwizzle14;
        static const IdString UvDiffuseSwizzle15;

        static const IdString BlendModeIndex0;
        static const IdString BlendModeIndex1;
        static const IdString BlendModeIndex2;
        static const IdString BlendModeIndex3;
        static const IdString BlendModeIndex4;
        static const IdString BlendModeIndex5;
        static const IdString BlendModeIndex6;
        static const IdString BlendModeIndex7;
        static const IdString BlendModeIndex8;
        static const IdString BlendModeIndex9;
        static const IdString BlendModeIndex10;
        static const IdString BlendModeIndex11;
        static const IdString BlendModeIndex12;
        static const IdString BlendModeIndex13;
        static const IdString BlendModeIndex14;
        static const IdString BlendModeIndex15;

        static const IdString OutUvCount;
        static const IdString OutUvHalfCount;

        struct DiffuseMapPtr
        {
            IdString const *uvSource;
            IdString const *uvSourceSwizzle;
            IdString const *blendModeIndex;
        };

        static const DiffuseMapPtr DiffuseMapPtrs[NUM_UNLIT_TEXTURE_TYPES];
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
