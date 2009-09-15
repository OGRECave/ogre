/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#ifndef __TERRAINVERTEXPROGRAM_H__
#define __TERRAINVERTEXPROGRAM_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgreString.h"

namespace Ogre {

    /*
    Static class containing the source to vertex programs used to 
    morph the terrain LOD levels. The programs are generated from 
    the following Cg:
    @code 	
    // No fog morphing terrain
    void terrain_vp(
	    float4 position : POSITION,
	    float2 uv1   	: TEXCOORD0,
	    float2 uv2	 	: TEXCOORD1,
	    float delta     : BLENDWEIGHT,

	    out float4 oPosition : POSITION,
	    out float2 oUv1		 : TEXCOORD0,
	    out float2 oUv2		 : TEXCOORD1,
	    out float4 colour    : COLOR,
	    uniform float4x4 worldViewProj,
	    uniform float morphFactor
	    )
    {
	    // Apply morph
	    position.y = position.y + (delta.x * morphFactor);
	    // world / view / projection
	    oPosition = mul(worldViewProj, position);
	    // Main texture coords
	    oUv1 = uv1;
	    // Detail texture coords
	    oUv2 = uv2;
	    // Full bright (no lighting)
	    colour = float4(1,1,1,1);
    }


    // Linear fogged morphing terrain
    void terrain_vp_linear(
	    float4 position : POSITION,
	    float2 uv1   	: TEXCOORD0,
	    float2 uv2	 	: TEXCOORD1,
	    float delta     : BLENDWEIGHT,

	    out float4 oPosition : POSITION,
	    out float2 oUv1		 : TEXCOORD0,
	    out float2 oUv2		 : TEXCOORD1,
	    out float4 colour    : COLOR,
	    out float fog		 : FOG,
	    uniform float4x4 worldViewProj,
	    uniform float morphFactor
	    )
    {
	    // Apply morph
	    position.y = position.y + (delta.x * morphFactor);
	    // world / view / projection
	    oPosition = mul(worldViewProj, position);
	    // Main texture coords
	    oUv1 = uv1;
	    // Detail texture coords
	    oUv2 = uv2;
	    // Full bright (no lighting)
	    colour = float4(1,1,1,1);
	    // Fog 
	    // f = end - camz / end - start
	    // when start / end has been set, fog value is distance
	    fog = oPosition.z;
    }


    // Exp fogged morphing terrain
    void terrain_vp_exp(
	    float4 position : POSITION,
	    float2 uv1   	: TEXCOORD0,
	    float2 uv2	 	: TEXCOORD1,
	    float delta     : BLENDWEIGHT,

	    out float4 oPosition : POSITION,
	    out float2 oUv1		 : TEXCOORD0,
	    out float2 oUv2		 : TEXCOORD1,
	    out float4 colour    : COLOR,
	    out float fog		 : FOG,
	    uniform float4x4 worldViewProj,
	    uniform float morphFactor,
	    uniform float fogDensity)
    {
	    // Apply morph
	    position.y = position.y + (delta.x * morphFactor);
	    // world / view / projection
	    oPosition = mul(worldViewProj, position);
	    // Main texture coords
	    oUv1 = uv1;
	    // Detail texture coords
	    oUv2 = uv2;
	    // Full bright (no lighting)
	    colour = float4(1,1,1,1);
	    // Fog 
	    // f = 1 / e ^ (camz x density)
	    // note pow = exp(src1 * log(src0)).
	    fog = 1 / (exp((oPosition.z * fogDensity) * log(2.718281828)));
    }


    // Exp2 fogged morphing terrain
    void terrain_vp_exp2(
	    float4 position : POSITION,
	    float2 uv1   	: TEXCOORD0,
	    float2 uv2	 	: TEXCOORD1,
	    float delta     : BLENDWEIGHT,

	    out float4 oPosition : POSITION,
	    out float2 oUv1		 : TEXCOORD0,
	    out float2 oUv2		 : TEXCOORD1,
	    out float4 colour    : COLOR,
	    out float fog		 : FOG,
	    uniform float4x4 worldViewProj,
	    uniform float morphFactor,
	    uniform float fogDensity)
    {
	    // Apply morph
	    position.y = position.y + (delta.x * morphFactor);
	    // world / view / projection
	    oPosition = mul(worldViewProj, position);
	    // Main texture coords
	    oUv1 = uv1;
	    // Detail texture coords
	    oUv2 = uv2;
	    // Full bright (no lighting)
	    colour = float4(1,1,1,1);
	    // Fog 
	    // f = 1 / e ^ (camz x density)^2
	    // note pow = exp(src1 * log(src0)).
	    float src1 = oPosition.z * 0.002;
	    fog = 1 / (exp((src1*src1) * log(2.718281828f)));
    }

	// Shadow receiver vertex program
    void terrain_shadow_receiver_vp(
	    float4 position : POSITION,
	    float2 uv1   	: TEXCOORD0,
	    float2 uv2	 	: TEXCOORD1,
	    float delta     : BLENDWEIGHT,

	    out float4 oPosition : POSITION,
	    out float2 oUv1		 : TEXCOORD0,
	    out float4 colour    : COLOR,
	    uniform float4x4 worldViewProj,
		uniform float4x4 world,
	    uniform float4x4 textureViewProj,
	    uniform float morphFactor
	    )
    {
	    // Apply morph
	    position.y = position.y + (delta.x * morphFactor);
	    // world / view / projection
	    oPosition = mul(worldViewProj, position);
		
	    // Main texture coords
		float4 worldpos = mul(world, position);
	    float4 projuv = mul(textureViewProj, worldpos);
		oUv1.xy = projuv.xy / projuv.w;
	    // Full bright (no lighting)
	    colour = float4(1,1,1,1);
    }

    @endcode
    */
class TerrainVertexProgram : public ResourceAlloc
    {
    private:
        static String mNoFogArbvp1;
        static String mLinearFogArbvp1;
        static String mExpFogArbvp1;
        static String mExp2FogArbvp1;
		static String mShadowReceiverArbvp1;

        static String mNoFogVs_1_1;
        static String mLinearFogVs_1_1;
        static String mExpFogVs_1_1;
        static String mExp2FogVs_1_1;
		static String mShadowReceiverVs_1_1;

    public:
        /// General purpose method to get any of the program sources
        static const String& getProgramSource(FogMode fogMode, 
			const String syntax, bool shadowReceiver = false);


    };
}

#endif
