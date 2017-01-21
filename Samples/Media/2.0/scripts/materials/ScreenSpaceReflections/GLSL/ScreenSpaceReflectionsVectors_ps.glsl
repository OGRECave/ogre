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

#version 330

uniform sampler2D depthTexture;
uniform sampler2D gBuf_normals;
uniform sampler2D prevFrame;

#define float2 vec2
#define float3 vec3
#define float4 vec4

#define int2 ivec2
#define int3 ivec3

#define float4x4 mat4

#define lerp mix

#define saturate(x) clamp( (x), 0.0, 1.0 )

#define INLINE

out vec4 fragColour;

in block
{
	vec2 uv0;
	vec3 cameraDir;
} inPs;

//struct Params
//{
uniform	float4 p_depthBufferRes;// dimensions of the z-buffer. .w = max( .x, .y )
uniform	float p_zThickness;		// thickness to ascribe to each pixel in the depth buffer
uniform	float p_nearPlaneZ;		// the camera's near z plane

uniform	float p_stride;		// Step in horizontal or vertical pixels between samples. This is a float
							// because integer math is slow on GPUs, but should be set to an integer >= 1.
uniform	float p_maxSteps;		// Maximum number of iterations. Higher gives better images but may be slow.
uniform	float p_maxDistance;	// Maximum camera-space distance to trace before returning a miss.
uniform	float p_strideZCutoff;	// More distant pixels are smaller in screen space. This value tells at what point to
								// start relaxing the stride to give higher quality reflections for objects far from
								// the camera.

#if 0
	uniform	float p_fadeStart;		// determines where to start screen edge fading of effect
	//uniform	float p_fadeEnd;		// determines where to end screen edge fading of effect
	uniform	float p_invFadeRange;	//1.0 / (p_fadeEnd - p_fadeStart)
#endif

uniform	float2 p_projectionParams;
uniform	float2 p_invDepthBufferRes;

uniform	float4x4 p_viewToTextureSpaceMatrix;
//};

//uniform Params p;

INLINE float distanceSquared( float2 a, float2 b )
{
	a -= b;
	return dot( a, a );
}

INLINE bool intersectsDepthBuffer( float z, float minZ, float maxZ )
{
	/*
		Based on how far away from the camera the depth is,
		adding a bit of extra thickness can help improve some
		artifacts. Driving this value up too high can cause
		artifacts of its own.
	*/
	float depthScale = min( 1.0f, z * p_strideZCutoff );
	z += p_zThickness + lerp( 0.0f, 2.0f, depthScale );
	return (maxZ >= z) && (minZ - p_zThickness <= z);
}

INLINE void swap( inout float a, inout float b )
{
	float t = a;
	a = b;
	b = t;
}

INLINE float linearizeDepth( float fDepth )
{
	return p_projectionParams.y / (fDepth - p_projectionParams.x);
}

INLINE float linearDepthTexelFetch( sampler2D depthTex, int2 hitPixel )
{
	// Load returns 0 for any value accessed out of bounds
	return linearizeDepth( texelFetch( depthTex, int2( hitPixel ), 0 ).x );
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
	out float2 hitPixel,
	// Camera space location of the ray hit
	out float3 hitPoint
)
{
	// Clip to the near plane
	float rayLength = ((csOrig.z + csDir.z * p_maxDistance) < p_nearPlaneZ) ?
			 (p_nearPlaneZ - csOrig.z) / csDir.z : p_maxDistance;
	float3 csEndPoint = csOrig + csDir * rayLength;

	// Project into homogeneous clip space
	//float4 H0 = mul( float4( csOrig, 1.0f ), p_viewToTextureSpaceMatrix );
	float4 H0 = p_viewToTextureSpaceMatrix * float4( csOrig, 1.0f );
	H0.xy *= p_depthBufferRes.xy;
	//float4 H1 = mul( float4( csEndPoint, 1.0f ), p_viewToTextureSpaceMatrix );
	float4 H1 = p_viewToTextureSpaceMatrix * float4( csEndPoint, 1.0f );
	H1.xy *= p_depthBufferRes.xy;
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
	float strideScale	= 1.0f - min( 1.0f, csOrig.z * p_strideZCutoff );
	float stride		= 1.0f + strideScale * p_stride;
	dP *= stride;
	dQ *= stride;
	dk *= stride;

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
		( (PQk.x * stepDir) <= end ) && (stepCount < p_maxSteps) &&
		!intersectsDepthBuffer( sceneZMax, rayZMin, rayZMax ) &&
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
		sceneZMax = linearDepthTexelFetch( depthTexture, int2( hitPixel ) );

		PQk += dPQk;
	}

	// Advance Q based on the number of steps
	Q.xy += dQ.xy * stepCount;
	hitPoint = Q * (1.0f / PQk.w);
	return intersectsDepthBuffer( sceneZMax, rayZMin, rayZMax );
}

#if 0
INLINE float specularPowerToConeAngle( float specularPower )
{
	// based on phong distribution model
	if( specularPower >= exp2(11) )
		return 0.0f;

	const float xi = 0.244f;
	float exponent = 1.0f / (specularPower + 1.0f);
	return acos( pow( xi, exponent ) );
}

INLINE float isoscelesTriangleOpposite( float adjacentLength, float coneTheta )
{
	// simple trig and algebra - soh, cah, toa - tan(theta) = opp/adj, opp = tan(theta) * adj,
	// then multiply * 2.0f for isosceles triangle base
	return 2.0f * tan(coneTheta) * adjacentLength;
}

INLINE float isoscelesTriangleInRadius( float a, float h )
{
	float a2 = a * a;
	float fh2 = 4.0f * h * h;
	return (a * (sqrt(a2 + fh2) - a)) / (4.0f * h);
}

INLINE float4 coneSampleWeightedColor( float2 samplePos, float mipChannel, float gloss )
{
	float3 sampleColor = textureLod( prevFrame, samplePos, mipChannel ).xyz;
	return float4( sampleColor * gloss, gloss );
}

INLINE float glossToSpecularPower( float gloss )
{
	return exp2(10.0 * gloss + 1.0);
}
#endif

in float4 gl_FragCoord;

void main()
{
	float3 normalVS = texelFetch( gBuf_normals, int2( gl_FragCoord.xy ), 0 ).xyz;
	normalVS.z = -normalVS.z; //Normal should be left handed.
	//if( !any(normalVS) )
	if( normalVS.x == 0 && normalVS.y == 0 && normalVS.z == 0 )
	{
		fragColour = vec4( 0.0f, 0.0f, 0.0f, 0.0f );
		return;
	}

	float depth = texelFetch( depthTexture, int2( gl_FragCoord.xy ), 0 ).x;
	float3 rayOriginVS = inPs.cameraDir.xyz * linearizeDepth( depth );

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

	float jitter = p_stride > 1.0f ? float(int(gl_FragCoord.x + gl_FragCoord.y) & 1) * 0.5f : 0.0f;

	// perform ray tracing - true if hit found, false otherwise
	bool intersection = traceScreenSpaceRay( rayOriginVS, rayDirectionVS, jitter, hitPixel, hitPoint );

	// move hit pixel from pixel position to UVs
	hitPixel *= p_invDepthBufferRes.xy;
	if( hitPixel.x > 1.0f || hitPixel.x < 0.0f || hitPixel.y > 1.0f || hitPixel.y < 0.0f )
		intersection = false;

	float fadeOnDistance = distance( hitPoint, rayOriginVS ) / p_maxDistance;
	//fadeOnDistance = 1.0f - saturate( fadeOnDistance );
	fadeOnDistance = 1.0f - fadeOnDistance;

	//depth = texelFetch( depthTexture, int2( hitPixel ), 0 ).x;

	fragColour = intersection ? float4( hitPixel.xy, /*depth*/fadeOnDistance, rDotV ) : float4( 0, 0, 0, 0 );

#if 0
	//-----------------------------------
	//CONE TRACE SECTION (single pass)
	//-----------------------------------

	if( !intersection )
	{
		hitPixel.xy = float2( 0.0, 0.0 );
		rDotV = 0;
	}

	float roughness = texelFetch( gBuf_shadowRoughness, int2( gl_FragCoord.xy ), 0 ).y;
	//float roughness = 0.5;
	float gloss = 1.0f - roughness;
	float specularPower = glossToSpecularPower( gloss );

	// convert to cone angle (maximum extent of the specular lobe aperture)
	// only want half the full cone angle since we're slicing the isosceles triangle in half to get a right triangle
	float coneTheta = specularPowerToConeAngle( specularPower ) * 0.5f;

	// P1 = positionSS, P2 = hitPixel, adjacent length = ||P2 - P1||
	float2 positionSS = inPs.uv0.xy;
	float2 deltaP = hitPixel.xy - positionSS.xy;
	float adjacentLength = length( deltaP );
	float2 adjacentUnit = normalize( deltaP );

	float4 totalColor = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float remainingAlpha = 1.0f;
	float glossMult = gloss;
	// cone-tracing using an isosceles triangle to approximate a cone in screen space
	for( int i = 0; i < 14; ++i )
	{
		// intersection length is the adjacent side, get the opposite side using trig
		float oppositeLength = isoscelesTriangleOpposite( adjacentLength, coneTheta );

		// calculate in-radius of the isosceles triangle
		float incircleSize = isoscelesTriangleInRadius( oppositeLength, adjacentLength );

		// get the sample position in screen space
		float2 samplePos = positionSS.xy + adjacentUnit * (adjacentLength - incircleSize);

		//convert the in-radius into screen size then check what power N to
		//raise 2 to reach it - that power N becomes mip level to sample from
		float mipChannel = log2( incircleSize * p_depthBufferRes.w );

		/**	Read color and accumulate it using trilinear filtering and weight it.
			Uses pre-convolved image (color buffer) and glossiness to weigh color contributions.
			Visibility is accumulated in the alpha channel.
			Break if visibility is 100% or greater (>= 1.0f).
		*/
		float4 newColor = coneSampleWeightedColor( samplePos, mipChannel, glossMult );

		remainingAlpha -= newColor.a;
		if( remainingAlpha < 0.0f )
			newColor.xyz *= ( 1.0f - abs(remainingAlpha) );
		totalColor += newColor;

		if( totalColor.a >= 1.0f )
			break;

		// subtract the diameter of the incircle to get the adjacent side of the next level on the cone
		// (isoscelesTriangleNextAdjacent( adjacentLength, incircleRadius ))
		adjacentLength = adjacentLength - (incircleSize * 2.0f);
		glossMult *= gloss;
	}

	// fade rays close to screen edge
	float2 boundary = abs( hitPixel.xy - float2(0.5f, 0.5f) ) * 2.0f;
	float fadeOnBorder = 1.0f - saturate( (boundary.x - p_fadeStart) * p_invFadeRange );
	fadeOnBorder *= 1.0f - saturate( (boundary.y - p_fadeStart) * p_invFadeRange );
	fadeOnBorder = smoothstep( 0.0f, 1.0f, fadeOnBorder );
	//float3 rayHitPositionVS = viewSpacePositionFromDepth( hitPixel.xy, raySS.z );
	float fadeOnDistance = distance( hitPoint /*rayHitPositionVS*/, rayOriginVS ) / p_maxDistance;
	fadeOnDistance = 1.0f - saturate( fadeOnDistance );
	// ray tracing steps stores rdotv in w component - always > 0 due to check at start of this method
	float fadeOnPerpendicular = saturate( rDotV * 4.0f );
	float fadeOnRoughness = saturate( gloss * 4.0f );
	float totalFade =	fadeOnBorder * fadeOnDistance * fadeOnPerpendicular *
						fadeOnRoughness * ( 1.0f - saturate(remainingAlpha) );

	float3 fallbackColor = float3( 0, 0, 0 );

	fragColour = float4( lerp( fallbackColor, totalColor.rgb, totalFade ), 1.0f );
	//fragColour = float4( fadeOnDistance, fadeOnDistance, fadeOnDistance, 1 );
	//fragColour = float4( totalColor.xyz, 1 );
#endif
}
