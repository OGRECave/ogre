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

#include "OgreUnlitProperty.h"


namespace Ogre
{
    const IdString UnlitProperty::HwGammaRead       = IdString( "hw_gamma_read" );
    const IdString UnlitProperty::HwGammaWrite      = IdString( "hw_gamma_write" );
    const IdString UnlitProperty::SignedIntTex      = IdString( "signed_int_textures" );
    const IdString UnlitProperty::MaterialsPerBuffer= IdString( "materials_per_buffer" );
    const IdString UnlitProperty::AnimationMatricesPerBuffer = IdString( "animation_matrices_per_buffer" );
    const IdString UnlitProperty::TextureMatrix     = IdString( "texture_matrix" );
    const IdString UnlitProperty::ExponentialShadowMaps = IdString( "exponential_shadow_maps" );
    const IdString UnlitProperty::HasPlanarReflections  = IdString( "has_planar_reflections" );

    const IdString UnlitProperty::TexMatrixCount        = IdString( "hlms_texture_matrix_count" );
    const IdString UnlitProperty::TexMatrixCount0       = IdString( "hlms_texture_matrix_count0" );
    const IdString UnlitProperty::TexMatrixCount1       = IdString( "hlms_texture_matrix_count1" );
    const IdString UnlitProperty::TexMatrixCount2       = IdString( "hlms_texture_matrix_count2" );
    const IdString UnlitProperty::TexMatrixCount3       = IdString( "hlms_texture_matrix_count3" );
    const IdString UnlitProperty::TexMatrixCount4       = IdString( "hlms_texture_matrix_count4" );
    const IdString UnlitProperty::TexMatrixCount5       = IdString( "hlms_texture_matrix_count5" );
    const IdString UnlitProperty::TexMatrixCount6       = IdString( "hlms_texture_matrix_count6" );
    const IdString UnlitProperty::TexMatrixCount7       = IdString( "hlms_texture_matrix_count7" );

    const IdString UnlitProperty::Diffuse               = IdString( "diffuse" );

    const IdString UnlitProperty::NumArrayTextures      = IdString( "num_array_textures" );
    const IdString UnlitProperty::NumTextures           = IdString( "num_textures" );

    const IdString UnlitProperty::DiffuseMap            = IdString( "diffuse_map" );
    //const IdString UnlitProperty::DiffuseMap0           = IdString( "diffuse_map0" );
    //const IdString UnlitProperty::DiffuseMap0Array      = IdString( "diffuse_map0_array" );

    const IdString UnlitProperty::UvDiffuse0            = IdString( "uv_diffuse0" );
    const IdString UnlitProperty::UvDiffuse1            = IdString( "uv_diffuse1" );
    const IdString UnlitProperty::UvDiffuse2            = IdString( "uv_diffuse2" );
    const IdString UnlitProperty::UvDiffuse3            = IdString( "uv_diffuse3" );
    const IdString UnlitProperty::UvDiffuse4            = IdString( "uv_diffuse4" );
    const IdString UnlitProperty::UvDiffuse5            = IdString( "uv_diffuse5" );
    const IdString UnlitProperty::UvDiffuse6            = IdString( "uv_diffuse6" );
    const IdString UnlitProperty::UvDiffuse7            = IdString( "uv_diffuse7" );
    const IdString UnlitProperty::UvDiffuse8            = IdString( "uv_diffuse8" );
    const IdString UnlitProperty::UvDiffuse9            = IdString( "uv_diffuse9" );
    const IdString UnlitProperty::UvDiffuse10           = IdString( "uv_diffuse10" );
    const IdString UnlitProperty::UvDiffuse11           = IdString( "uv_diffuse11" );
    const IdString UnlitProperty::UvDiffuse12           = IdString( "uv_diffuse12" );
    const IdString UnlitProperty::UvDiffuse13           = IdString( "uv_diffuse13" );
    const IdString UnlitProperty::UvDiffuse14           = IdString( "uv_diffuse14" );
    const IdString UnlitProperty::UvDiffuse15           = IdString( "uv_diffuse15" );

    const IdString UnlitProperty::UvDiffuseSwizzle0     = IdString( "uv_diffuse_swizzle0" );
    const IdString UnlitProperty::UvDiffuseSwizzle1     = IdString( "uv_diffuse_swizzle1" );
    const IdString UnlitProperty::UvDiffuseSwizzle2     = IdString( "uv_diffuse_swizzle2" );
    const IdString UnlitProperty::UvDiffuseSwizzle3     = IdString( "uv_diffuse_swizzle3" );
    const IdString UnlitProperty::UvDiffuseSwizzle4     = IdString( "uv_diffuse_swizzle4" );
    const IdString UnlitProperty::UvDiffuseSwizzle5     = IdString( "uv_diffuse_swizzle5" );
    const IdString UnlitProperty::UvDiffuseSwizzle6     = IdString( "uv_diffuse_swizzle6" );
    const IdString UnlitProperty::UvDiffuseSwizzle7     = IdString( "uv_diffuse_swizzle7" );
    const IdString UnlitProperty::UvDiffuseSwizzle8     = IdString( "uv_diffuse_swizzle8" );
    const IdString UnlitProperty::UvDiffuseSwizzle9     = IdString( "uv_diffuse_swizzle9" );
    const IdString UnlitProperty::UvDiffuseSwizzle10    = IdString( "uv_diffuse_swizzle10" );
    const IdString UnlitProperty::UvDiffuseSwizzle11    = IdString( "uv_diffuse_swizzle11" );
    const IdString UnlitProperty::UvDiffuseSwizzle12    = IdString( "uv_diffuse_swizzle12" );
    const IdString UnlitProperty::UvDiffuseSwizzle13    = IdString( "uv_diffuse_swizzle13" );
    const IdString UnlitProperty::UvDiffuseSwizzle14    = IdString( "uv_diffuse_swizzle14" );
    const IdString UnlitProperty::UvDiffuseSwizzle15    = IdString( "uv_diffuse_swizzle15" );

    const IdString UnlitProperty::BlendModeIndex0       = IdString( "blend_mode_idx0" );
    const IdString UnlitProperty::BlendModeIndex1       = IdString( "blend_mode_idx1" );
    const IdString UnlitProperty::BlendModeIndex2       = IdString( "blend_mode_idx2" );
    const IdString UnlitProperty::BlendModeIndex3       = IdString( "blend_mode_idx3" );
    const IdString UnlitProperty::BlendModeIndex4       = IdString( "blend_mode_idx4" );
    const IdString UnlitProperty::BlendModeIndex5       = IdString( "blend_mode_idx5" );
    const IdString UnlitProperty::BlendModeIndex6       = IdString( "blend_mode_idx6" );
    const IdString UnlitProperty::BlendModeIndex7       = IdString( "blend_mode_idx7" );
    const IdString UnlitProperty::BlendModeIndex8       = IdString( "blend_mode_idx8" );
    const IdString UnlitProperty::BlendModeIndex9       = IdString( "blend_mode_idx9" );
    const IdString UnlitProperty::BlendModeIndex10      = IdString( "blend_mode_idx10" );
    const IdString UnlitProperty::BlendModeIndex11      = IdString( "blend_mode_idx11" );
    const IdString UnlitProperty::BlendModeIndex12      = IdString( "blend_mode_idx12" );
    const IdString UnlitProperty::BlendModeIndex13      = IdString( "blend_mode_idx13" );
    const IdString UnlitProperty::BlendModeIndex14      = IdString( "blend_mode_idx14" );
    const IdString UnlitProperty::BlendModeIndex15      = IdString( "blend_mode_idx15" );

    const IdString UnlitProperty::OutUvCount            = IdString( "out_uv_count" );
    const IdString UnlitProperty::OutUvHalfCount        = IdString( "out_uv_half_count" );
    /*
    //There are out_uv_half_count of this (half of out_uv_count, rounded up)
    const IdString UnlitProperty::OutUvCount0           = IdString( "out_uv_half_count0" );

    //There are out_uv_count of these
    const IdString UnlitProperty::OutUv0TextureMatrix   = IdString( "out_uv0_out_uv" );
    const IdString UnlitProperty::OutUv0TextureMatrix   = IdString( "out_uv0_texture_matrix" );
    const IdString UnlitProperty::OutUv0TexUnit         = IdString( "out_uv0_tex_unit" );
    const IdString UnlitProperty::OutUv0Swizzle         = IdString( "out_uv0_swizzle" );
    const IdString UnlitProperty::OutUv0SourceUv        = IdString( "out_uv0_source_uv" );*/



    const UnlitProperty::DiffuseMapPtr UnlitProperty::DiffuseMapPtrs[NUM_UNLIT_TEXTURE_TYPES] =
    {
        { &UnlitProperty::UvDiffuse0, &UnlitProperty::UvDiffuseSwizzle0, &UnlitProperty::BlendModeIndex0 },
        { &UnlitProperty::UvDiffuse1, &UnlitProperty::UvDiffuseSwizzle1, &UnlitProperty::BlendModeIndex1 },
        { &UnlitProperty::UvDiffuse2, &UnlitProperty::UvDiffuseSwizzle2, &UnlitProperty::BlendModeIndex2 },
        { &UnlitProperty::UvDiffuse3, &UnlitProperty::UvDiffuseSwizzle3, &UnlitProperty::BlendModeIndex3 },
        { &UnlitProperty::UvDiffuse4, &UnlitProperty::UvDiffuseSwizzle4, &UnlitProperty::BlendModeIndex4 },
        { &UnlitProperty::UvDiffuse5, &UnlitProperty::UvDiffuseSwizzle5, &UnlitProperty::BlendModeIndex5 },
        { &UnlitProperty::UvDiffuse6, &UnlitProperty::UvDiffuseSwizzle6, &UnlitProperty::BlendModeIndex6 },
        { &UnlitProperty::UvDiffuse7, &UnlitProperty::UvDiffuseSwizzle7, &UnlitProperty::BlendModeIndex7 },
        { &UnlitProperty::UvDiffuse8, &UnlitProperty::UvDiffuseSwizzle8, &UnlitProperty::BlendModeIndex8 },
        { &UnlitProperty::UvDiffuse9, &UnlitProperty::UvDiffuseSwizzle9, &UnlitProperty::BlendModeIndex9 },
        { &UnlitProperty::UvDiffuse10, &UnlitProperty::UvDiffuseSwizzle10, &UnlitProperty::BlendModeIndex10 },
        { &UnlitProperty::UvDiffuse11, &UnlitProperty::UvDiffuseSwizzle11, &UnlitProperty::BlendModeIndex11 },
        { &UnlitProperty::UvDiffuse12, &UnlitProperty::UvDiffuseSwizzle12, &UnlitProperty::BlendModeIndex12 },
        { &UnlitProperty::UvDiffuse13, &UnlitProperty::UvDiffuseSwizzle13, &UnlitProperty::BlendModeIndex13 },
        { &UnlitProperty::UvDiffuse14, &UnlitProperty::UvDiffuseSwizzle14, &UnlitProperty::BlendModeIndex14 },
        { &UnlitProperty::UvDiffuse15, &UnlitProperty::UvDiffuseSwizzle15, &UnlitProperty::BlendModeIndex15 },
    };

 }
