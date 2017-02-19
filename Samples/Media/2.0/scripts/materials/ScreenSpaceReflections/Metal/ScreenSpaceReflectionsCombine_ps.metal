// Based on http://roar11.com/2015/07/screen-space-glossy-reflections/
#include <metal_stdlib>
using namespace metal;

#define INLINE inline
#define PARAMS_ARG_DECL , constant Params &p
#define PARAMS_ARG , p

#define TEXTURES_ARG_DECL , texture2d<float> prevFrame, sampler trilinearSampler
#define TEXTURES_ARG , prevFrame, trilinearSampler

struct PS_INPUT
{
	float2 uv0;
	float3 cameraDir;
};

struct Params
{
	float4 depthBufferRes;// dimensions of the z-buffer. .w = max( .x, .y )

	//float maxDistance;	// Maximum camera-space distance to trace before returning a miss.

	float fadeStart;		// determines where to start screen edge fading of effect
	//float fadeEnd;		// determines where to end screen edge fading of effect
	float invFadeRange;	//1.0 / (fadeEnd - fadeStart)

	float2 projectionParams;

	float4x4 textureSpaceToViewSpace;
	float3x3 invViewMatCubemap;
};

/*INLINE float specularPowerToConeAngle( float specularPower )
{
	// based on phong distribution model
	if( specularPower >= exp2(11) )
		return 0.0f;

	const float xi = 0.244f;
	float exponent = 1.0f / (specularPower + 1.0f);
	return acos( pow( xi, exponent ) );
}*/

INLINE float roughnessToConeAngle( float roughness )
{
	//This formula was eye-balled. Not done scientifically at all. It's good and fast. Deal with it.
	//(at least for for roughness < 0.6 which is the range we really care)
	return smoothstep( 0.0, 1.0, 1 - 1 / (1 + roughness * roughness) );
}

INLINE float isoscelesTriangleOpposite( float adjacentLength, float tanConeTheta )
{
	// simple trig and algebra - soh, cah, toa - tan(theta) = opp/adj, opp = tan(theta) * adj,
	// then multiply * 2.0f for isosceles triangle base
	//return 2.0f * tan(coneTheta) * adjacentLength;
	return 2.0f * tanConeTheta * adjacentLength;
}

INLINE float isoscelesTriangleInRadius( float a, float h )
{
	float a2 = a * a;
	float fh2 = 4.0f * h * h;
	return (a * (sqrt(a2 + fh2) - a)) / (4.0f * h);
}

INLINE float4 coneSampleWeightedColor( float2 samplePos, float mipChannel, float gloss TEXTURES_ARG_DECL )
{
	float3 sampleColor = prevFrame.sample( trilinearSampler, samplePos, level(mipChannel) ).xyz;
	return float4( sampleColor * gloss, gloss );
}

/*INLINE float glossToSpecularPower( float gloss )
{
	return exp2(10.0 * gloss + 1.0);
}*/

/*INLINE float3 viewSpacePositionFromDepth( float2 ssPosXY, float zDepth PARAMS_ARG_DECL )
{
	float4 result = p.textureSpaceToViewSpace * float4( ssPosXY, zDepth, 1.0 );
	result.xyz /= result.w;
	return result.xyz;
}*/

fragment float4 main_metal
(
	PS_INPUT inPs		[[stage_in]],
	float4 gl_FragCoord	[[position]],

	texture2d<float, access::read> depthTexture					[[texture(0)]],
	#if !USE_MSAA
		texture2d<float, access::read> gBuf_normals				[[texture(1)]],
		texture2d<float, access::read> gBuf_shadowRoughness		[[texture(2)]],
	#else
		texture2d_ms<float, access::read> gBuf_normals			[[texture(1)]],
		texture2d_ms<float, access::read> gBuf_shadowRoughness	[[texture(2)]],
	#endif
	texture2d<float> prevFrame									[[texture(3)]],
	#if HQ
		texture2d<float, access::read> rayTraceBuffer				[[texture(4)]],
	#else
		texture2d<float, access::sample> rayTraceBuffer				[[texture(4)]],
	#endif
	//uniform samplerCube globalCubemap							[[texture(5)]],

	sampler trilinearSampler									[[sampler(3)]],

	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
	float4 fragColour;

	//float depth = depthTexture.read( uint2( gl_FragCoord.xy ), 0 ).x;
	//float3 rayOriginVS = inPs.cameraDir.xyz * linearizeDepth( depth );


#if HQ
	float4 raySS = rayTraceBuffer.read( uint2( gl_FragCoord.xy ), 0 ).xyzw;
#else
	float4 raySS = rayTraceBuffer.sample( trilinearSampler, inPs.uv0.xy ).xyzw;
#endif

	//Do not decode roughness, keep it in [0; 1] range.
	float roughness = gBuf_shadowRoughness.read( uint2( gl_FragCoord.xy ), 0 ).y;
	float gloss = 1.0f - roughness;
	/*float specularPower = glossToSpecularPower( gloss );

	float coneTheta = specularPowerToConeAngle( specularPower ) * 0.5f;
	//float tanConeTheta = tan( roughnessToConeAngle( roughness ) * 0.5f );*/
	// convert to cone angle (maximum extent of the specular lobe aperture)
	// only want half the full cone angle since we're slicing the isosceles
	// triangle in half to get a right triangle
	float tanConeTheta = roughnessToConeAngle( roughness );

	// P1 = positionSS, P2 = hitPixel, adjacent length = ||P2 - P1||
	float2 positionSS = inPs.uv0.xy;
	float2 deltaP = raySS.xy - positionSS.xy;
	float adjacentLength = length( deltaP );
	float2 adjacentUnit = normalize( deltaP );

	float4 totalColor = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float remainingAlpha = 1.0f;
	float glossMult = gloss;
	// cone-tracing using an isosceles triangle to approximate a cone in screen space
	for( int i = 0; i < 14; ++i )
	{
		// intersection length is the adjacent side, get the opposite side using trig
		float oppositeLength = isoscelesTriangleOpposite( adjacentLength, tanConeTheta );

		// calculate in-radius of the isosceles triangle
		float incircleSize = isoscelesTriangleInRadius( oppositeLength, adjacentLength );

		// get the sample position in screen space
		float2 samplePos = positionSS.xy + adjacentUnit * (adjacentLength - incircleSize);

		//convert the in-radius into screen size then check what power N to
		//raise 2 to reach it - that power N becomes mip level to sample from
		float mipChannel = log2( incircleSize * p.depthBufferRes.w );

		/**	Read color and accumulate it using trilinear filtering and weight it.
			Uses pre-convolved image (color buffer) and glossiness to weigh color contributions.
			Visibility is accumulated in the alpha channel.
			Break if visibility is 100% or greater (>= 1.0f).
		*/
		float4 newColor = coneSampleWeightedColor( samplePos, mipChannel, glossMult TEXTURES_ARG );

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
	float2 boundary = abs( raySS.xy - float2(0.5f, 0.5f) ) * 2.0f;
	float fadeOnBorder = 1.0f - saturate( (boundary.x - p.fadeStart) * p.invFadeRange );
	fadeOnBorder *= 1.0f - saturate( (boundary.y - p.fadeStart) * p.invFadeRange );
	fadeOnBorder = smoothstep( 0.0f, 1.0f, fadeOnBorder );
	float fadeOnDistance = raySS.z;
	//float3 hitPointVS = viewSpacePositionFromDepth( raySS.xy, raySS.z PARAMS_ARG );
	//float fadeOnDistance = distance( hitPointVS, rayOriginVS ) / p.maxDistance;
	//fadeOnDistance = 1.0f - saturate( fadeOnDistance );

	// ray tracing steps stores rdotv in w component - always > 0 due to check at start of this method
	float fadeOnPerpendicular = saturate( raySS.w * 4.0f );
	float fadeOnRoughness = saturate( gloss * 4.0f );
	float totalFade =	fadeOnBorder * fadeOnDistance * fadeOnPerpendicular *
						fadeOnRoughness * ( 1.0f - saturate(remainingAlpha) );

	/*float3 nNormal = gBuf_normals.read( uint2( gl_FragCoord.xy ), 0 ).xyz * 2.0 - 1.0;
	float3 viewDir = normalize( float3( inPs.cameraDir.xy, -inPs.cameraDir.z ) );
	float3 reflDir = viewDir - 2.0 * dot( viewDir, nNormal ) * nNormal;
	float3 globalCubemapColour = textureLod( globalCubemap, p.invViewMatCubemap * reflDir,
											 roughness * 12.0 ).xyz;

	float3 fallbackColor = globalCubemapColour;

	fragColour = float4( lerp( fallbackColor, totalColor.rgb, totalFade ), 1.0f );*/

	//TODO: Perform the fallback blending SSR -> Local Cubemap -> Global Cubemap here.
	fragColour = float4( totalColor.rgb, totalFade );

	//fragColour = float4( fadeOnDistance, fadeOnDistance, fadeOnDistance, 1 );
	//fragColour = float4( totalColor.xyz, 1 );

	return fragColour;
}
