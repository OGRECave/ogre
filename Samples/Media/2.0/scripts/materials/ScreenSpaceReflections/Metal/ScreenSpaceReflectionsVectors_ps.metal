// By Morgan McGuire and Michael Mara at Williams College 2014
// Released as open source under the BSD 2-Clause License
// http://opensource.org/licenses/BSD-2-Clause
//
// Copyright (c) 2014, Morgan McGuire and Michael Mara
// All rights reserved.
//
// From McGuire and Mara, Efficient GPU Screen-Space Ray Tracing,
// Journal of Computer Graphics Techniques, 2014
//
// Small Custom adaptations by Matias N. Goldberg
//
// This software is open source under the "BSD 2-clause license":
//
// Redistribution and use in source and binary forms, with or
// without modification, are permitted provided that the following
// conditions are met:
//
// 1. Redistributions of source code must retain the above
// copyright notice, this list of conditions and the following
// disclaimer.
//
// 2. Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials provided
// with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.

#include <metal_stdlib>
using namespace metal;

#define inout thread
#define out thread
#define INLINE inline
#define PARAMS_ARG_DECL , constant Params &p
#define PARAMS_ARG , p
#define TEXTURES_ARG_DECL , texture2d<float, access::read> depthTexture
#define TEXTURES_ARG , depthTexture

struct PS_INPUT
{
	float2 uv0;
	float3 cameraDir;
};

struct Params
{
	float4 depthBufferRes;// dimensions of the z-buffer. .w = max( .x, .y )
	// .x = thickness to ascribe to each pixel in the depth buffer.
	// .yzw = Bias to increase the thickness as Z becomes larger to perform high quality reflections for
	//		  distant objects. See intersectsDepthBuffer and ScreenSpaceReflections::setupSSRValues
	float4 zThickness;
	float nearPlaneZ;		// the camera's near z plane

	float stride;		// Step in horizontal or vertical pixels between samples. This is a float
								// because integer math is slow on GPUs, but should be set to an integer >= 1.
	float maxSteps;		// Maximum number of iterations. Higher gives better images but may be slow.
	float maxDistance;	// Maximum camera-space distance to trace before returning a miss.

	float2 projectionParams;
	float2 invDepthBufferRes;

	float4x4 viewToTextureSpaceMatrix;
	float4x4 reprojectionMatrix;
	float reprojectionMaxDistanceError;
};

INLINE float distanceSquared( float2 a, float2 b )
{
	a -= b;
	return dot( a, a );
}

INLINE bool intersectsDepthBuffer( float z, float minZ, float maxZ PARAMS_ARG_DECL )
{
	/*
		Based on how far away from the camera the depth is,
		adding a bit of extra thickness can help improve some
		artifacts. Driving this value up too high can cause
		artifacts of its own.
	*/
	//Originally:
	//	depthThicknessDynamicBias = clamp( (z + biasStart) * biasRange, 0, 1 ) * biasAmount;
	//	= clamp( (z - biasStart) * biasRange * biasAmount, 0, biasAmount );
	//	= clamp( (z - biasStart) * biasRangeTimesAmount, 0, biasAmount );
	//	= clamp( (z * biasRangeTimesAmount + -biasStartTimesRangeTimesAmount), 0, biasAmount );
	//The goal is to preserver precision as biasRange may be too small,
	//and to allow fmad optimization.
	float depthThicknessDynamicBias = clamp( (z * p.zThickness.y + p.zThickness.z),
											 0.0, p.zThickness.w );
	z += p.zThickness.x + depthThicknessDynamicBias;
	return (maxZ >= z) && (minZ - p.zThickness.x - depthThicknessDynamicBias <= z);
}

INLINE void swap( inout float &a, inout float &b )
{
	float t = a;
	a = b;
	b = t;
}

INLINE float linearizeDepth( float fDepth PARAMS_ARG_DECL )
{
	return p.projectionParams.y / (fDepth - p.projectionParams.x);
}

#if !USE_MSAA
	INLINE float3 loadSubsample0( texture2d<float, access::read> tex, float2 iCoords )
	{
		return tex.read( uint2( iCoords ), 0 ).xyz;
	}
#else
	INLINE float3 loadSubsample0( texture2d_ms<float, access::read> tex, float2 iCoords )
	{
		return tex.read( uint2( iCoords ), 0 ).xyz;
	}
#endif

INLINE float linearDepthTexelFetch( texture2d<float, access::read> depthTex, uint2 hitPixel PARAMS_ARG_DECL )
{
	// Load returns 0 for any value accessed out of bounds
	return linearizeDepth( depthTex.read( hitPixel, 0 ).x PARAMS_ARG );
}

// Returns true if the ray hit something
INLINE bool traceScreenSpaceRay
(
	// Camera-space ray origin, which must be within the view volume
	float3 csOrig,
	// Unit length camera-space ray direction
	float3 csDir,
	// Number between 0 and 1 for how far to bump the ray in stride units
	// to conceal banding artifacts. Not needed if stride == 1.
	float jitter,
	// Pixel coordinates of the first intersection with the scene
	out float2 &hitPixel,
	// Camera space location of the ray hit
	out float3 &hitPoint
	PARAMS_ARG_DECL
	TEXTURES_ARG_DECL
)
{
	// Clip to the near plane
	float rayLength = ((csOrig.z + csDir.z * p.maxDistance) < p.nearPlaneZ) ?
			 (p.nearPlaneZ - csOrig.z) / csDir.z : p.maxDistance;
	float3 csEndPoint = csOrig + csDir * rayLength;

	// Project into homogeneous clip space
	//float4 H0 = float4( csOrig, 1.0f ) * p.viewToTextureSpaceMatrix;
	float4 H0 = p.viewToTextureSpaceMatrix * float4( csOrig, 1.0f );
	H0.xy *= p.depthBufferRes.xy;
	//float4 H1 = float4( csEndPoint, 1.0f ) * p.viewToTextureSpaceMatrix;
	float4 H1 = p.viewToTextureSpaceMatrix * float4( csEndPoint, 1.0f );
	H1.xy *= p.depthBufferRes.xy;
	float k0 = 1.0f / H0.w;
	float k1 = 1.0f / H1.w;

	// The interpolated homogeneous version of the camera-space points
	float3 Q0 = csOrig * k0;
	float3 Q1 = csEndPoint * k1;

	// Screen-space endpoints
	float2 P0 = H0.xy * k0;
	float2 P1 = H1.xy * k1;

	// If the line is degenerate, make it cover at least one pixel
	// to avoid handling zero-pixel extent as a special case later
	P1 += ( distanceSquared(P0, P1) < 0.0001f ) ? float2( 0.01f, 0.01f ) : float2( 0.0f, 0.0f );
	float2 delta = P1 - P0;

	// Permute so that the primary iteration is in x to collapse
	// all quadrant-specific DDA cases later
	bool permute = false;
	if( abs( delta.x ) < abs( delta.y ) )
	{
		// This is a more-vertical line
		permute = true;
		delta = delta.yx;
		P0 = P0.yx;
		P1 = P1.yx;
	}

	float stepDir = sign( delta.x );
	float invdx = stepDir / delta.x;

	// Track the derivatives of Q and k
	float3 dQ	= (Q1 - Q0) * invdx;
	float dk	= (k1 - k0) * invdx;
	float2 dP	= float2( stepDir, delta.y * invdx );

	// Scale derivatives by the desired pixel stride and then
	// offset the starting values by the jitter fraction
	dP *= p.stride;
	dQ *= p.stride;
	dk *= p.stride;

	P0 += dP * jitter;
	Q0 += dQ * jitter;
	k0 += dk * jitter;

	// Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
	float4 PQk	= float4( P0.xy, Q0.z, k0 );
	float4 dPQk	= float4( dP.xy, dQ.z, dk );
	float3 Q = Q0;

	// Adjust end condition for iteration direction
	float end = P1.x * stepDir;

	float stepCount = 0.0f;
	float prevZMaxEstimate = csOrig.z;
	float rayZMin = prevZMaxEstimate;
	float rayZMax = prevZMaxEstimate;
	float sceneZMax = rayZMax + 100.0f;
	for(;
		( (PQk.x * stepDir) <= end ) && (stepCount < p.maxSteps) &&
		!intersectsDepthBuffer( sceneZMax, rayZMin, rayZMax PARAMS_ARG ) &&
		sceneZMax != 0.0f;
		++stepCount )
	{
		rayZMin = prevZMaxEstimate;
		rayZMax = (dPQk.z * 0.5f + PQk.z) / (dPQk.w * 0.5f + PQk.w);
		prevZMaxEstimate = rayZMax;
		if( rayZMin > rayZMax )
			swap( rayZMin, rayZMax );

		hitPixel = permute ? PQk.yx : PQk.xy;
		// You may need hitPixel.y = depthBufferSize.y - hitPixel.y; here if your vertical axis
		// is different than ours in screen space
		sceneZMax = linearDepthTexelFetch( depthTexture, uint2( hitPixel ) PARAMS_ARG );

		PQk += dPQk;
	}

	// Advance Q based on the number of steps
	Q.xy += dQ.xy * stepCount;
	hitPoint = Q * (1.0f / PQk.w);
	return intersectsDepthBuffer( sceneZMax, rayZMin, rayZMax PARAMS_ARG );
}

#if HQ
	#define HQ_MULT
#else
	#define HQ_MULT * 2.0
#endif

fragment float4 main_metal
(
	PS_INPUT inPs		[[stage_in]],
	float4 gl_FragCoord [[position]],

	texture2d<float, access::read> depthTexture				[[texture(0)]],
	#if !USE_MSAA
		texture2d<float, access::read> gBuf_normals			[[texture(1)]],
	#else
		texture2d_ms<float, access::read> gBuf_normals		[[texture(1)]],
	#endif
	texture2d<float, access::read> prevFrameDepthTexture	[[texture(2)]],

	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
	float4 fragColour;

	float3 normalVS = normalize( loadSubsample0( gBuf_normals, gl_FragCoord.xy HQ_MULT ).xyz * 2.0 - 1.0 );
	normalVS.z = -normalVS.z; //Normal should be left handed.
	//if( !any(normalVS) )
	if( normalVS.x == 0 && normalVS.y == 0 && normalVS.z == 0 )
	{
		fragColour = float4( 0.0f, 0.0f, 0.0f, 0.0f );
		return fragColour;
	}

	float depth = depthTexture.read( uint2( gl_FragCoord.xy HQ_MULT ), 0 ).x;
	float3 rayOriginVS = inPs.cameraDir.xyz * linearizeDepth( depth PARAMS_ARG );

	/** Since position is reconstructed in view space, just normalize it to get the
		vector from the eye to the position and then reflect that around the normal to
		get the ray direction to trace.
	*/
	float3 toPositionVS = normalize( rayOriginVS );
	float3 rayDirectionVS = normalize( reflect( toPositionVS, normalVS ) );

	// output rDotV to the alpha channel for use in determining how much to fade the ray
	float rDotV = dot( rayDirectionVS, toPositionVS );

	// out parameters
	float2 hitPixel = float2( 0.0f, 0.0f );
	float3 hitPoint = float3( 0.0f, 0.0f, 0.0f );

	float jitter = p.stride > 1.0f ? float(int(gl_FragCoord.x + gl_FragCoord.y) & 1) * 0.5f : 0.0f;

	// perform ray tracing - true if hit found, false otherwise
	bool intersection = traceScreenSpaceRay( rayOriginVS, rayDirectionVS, jitter,
											 hitPixel, hitPoint PARAMS_ARG TEXTURES_ARG );

	// Got this from Killing Floor 2
	// https://sakibsaikia.github.io/2016/12/26/Screen-Space-Reflection-in-Killing-Floor-2.html
	// This will check the direction of the normal of the reflection sample with the
	// direction of the reflection vector, and if they are pointing in the same direction,
	// it will drown out those reflections since backward facing pixels are not available
	// for screen space reflection. Attenuate reflections for angles between 90 degrees
	// and 100 degrees, and drop all contribution beyond the (-100,100)  degree range
	float3 normalsAtRefl = normalize( loadSubsample0( gBuf_normals, hitPixel.xy ).xyz * 2.0 - 1.0 );
	normalsAtRefl.z = -normalsAtRefl.z;
	//* 0.25 because later we'll multiply by 4
	rDotV *= smoothstep(-0.17, 0.0, dot( normalVS, -normalsAtRefl ) ) * 0.25;

	//Reflections lag one frame behind, so we need to reproject based on previous camera position
	//to know where to sample the colour. If the depth differences are too big then we don't use
	//that reflection.
	float currDepth = depthTexture.read( uint2( hitPixel.xy ), 0 ).x;
	hitPixel *= p.invDepthBufferRes.xy; //Transform hit pixel from pixel position to UVs
	float4 reprojectedPos = p.reprojectionMatrix * float4( hitPixel.xy, currDepth, 1.0 );
	reprojectedPos.xyz /= reprojectedPos.w;
	float prevDepth = prevFrameDepthTexture.read( uint2(reprojectedPos.xy * p.depthBufferRes.xy), 0 ).x;
	hitPixel.xy = reprojectedPos.xy;
	bool reprojFailed = (linearizeDepth( reprojectedPos.z PARAMS_ARG ) -
						 linearizeDepth( prevDepth PARAMS_ARG )) > p.reprojectionMaxDistanceError;

	intersection = intersection
					&& hitPixel.x <= 1.0f
					&& hitPixel.x >= 0.0f
					&& hitPixel.y <= 1.0f
					&& hitPixel.y >= 0.0f
					&& !reprojFailed;

	float fadeOnDistance = distance( hitPoint, rayOriginVS ) / p.maxDistance;
	//fadeOnDistance = 1.0f - saturate( fadeOnDistance );
	fadeOnDistance = 1.0f - fadeOnDistance;

	fragColour = intersection ? float4( hitPixel.xy, fadeOnDistance, rDotV ) : float4( 0, 0, 0, 0 );
	return fragColour;
}
