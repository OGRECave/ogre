// Based on http://roar11.com/2015/07/screen-space-glossy-reflections/

#version 330

uniform sampler2D depthTexture;
#if !USE_MSAA
	uniform sampler2D gBuf_normals;
	uniform sampler2D gBuf_shadowRoughness;
#else
	uniform sampler2DMS gBuf_normals;
	uniform sampler2DMS gBuf_shadowRoughness;
#endif
uniform sampler2D prevFrame;
uniform sampler2D rayTraceBuffer;
//uniform samplerCube globalCubemap;

#define float2 vec2
#define float3 vec3
#define float4 vec4

#define int2 ivec2
#define int3 ivec3

#define float3x3 mat3
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

#define p_depthBufferRes			depthBufferRes
#define p_fadeStart					fadeStart
#define p_invFadeRange				invFadeRange
#define p_projectionParams			projectionParams
#define p_textureSpaceToViewSpace	textureSpaceToViewSpace
#define p_invViewMatCubemap			invViewMatCubemap


//struct Params
//{
uniform	float4 p_depthBufferRes;// dimensions of the z-buffer. .w = max( .x, .y )

//uniform	float p_maxDistance;	// Maximum camera-space distance to trace before returning a miss.

uniform	float p_fadeStart;		// determines where to start screen edge fading of effect
//uniform	float p_fadeEnd;		// determines where to end screen edge fading of effect
uniform	float p_invFadeRange;	//1.0 / (p_fadeEnd - p_fadeStart)

uniform	float2 p_projectionParams;

uniform	float4x4 p_textureSpaceToViewSpace;
uniform	float3x3 p_invViewMatCubemap;
//};

//uniform Params p;

INLINE float linearizeDepth( float fDepth )
{
	return p_projectionParams.y / (fDepth - p_projectionParams.x);
}

INLINE float linearDepthTexelFetch( sampler2D depthTex, int2 hitPixel )
{
	// Load returns 0 for any value accessed out of bounds
	return linearizeDepth( texelFetch( depthTex, int2( hitPixel ), 0 ).x );
}

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

INLINE float4 coneSampleWeightedColor( float2 samplePos, float mipChannel, float gloss )
{
	float3 sampleColor = textureLod( prevFrame, samplePos, mipChannel ).xyz;
	return float4( sampleColor * gloss, gloss );
}

/*INLINE float glossToSpecularPower( float gloss )
{
	return exp2(10.0 * gloss + 1.0);
}*/

/*INLINE float3 viewSpacePositionFromDepth( float2 ssPosXY, float zDepth )
{
	float4 result = p_textureSpaceToViewSpace * float4( ssPosXY, zDepth, 1.0 );
	result.xyz /= result.w;
	return result.xyz;
}*/

in float4 gl_FragCoord;

void main()
{
	//float depth = texelFetch( depthTexture, int2( gl_FragCoord.xy ), 0 ).x;
	//float3 rayOriginVS = inPs.cameraDir.xyz * linearizeDepth( depth );

#if HQ
	float4 raySS = texelFetch( rayTraceBuffer, int2( gl_FragCoord.xy ), 0 ).xyzw;
#else
	float4 raySS = texture( rayTraceBuffer, inPs.uv0.xy ).xyzw;
#endif

	//Do not decode roughness, keep it in [0; 1] range.
	float roughness = texelFetch( gBuf_shadowRoughness, int2( gl_FragCoord.xy ), 0 ).y;
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
	float2 boundary = abs( raySS.xy - float2(0.5f, 0.5f) ) * 2.0f;
	float fadeOnBorder = 1.0f - saturate( (boundary.x - p_fadeStart) * p_invFadeRange );
	fadeOnBorder *= 1.0f - saturate( (boundary.y - p_fadeStart) * p_invFadeRange );
	fadeOnBorder = smoothstep( 0.0f, 1.0f, fadeOnBorder );
	float fadeOnDistance = raySS.z;
	//float3 hitPointVS = viewSpacePositionFromDepth( raySS.xy, raySS.z );
	//float fadeOnDistance = distance( hitPointVS, rayOriginVS ) / p_maxDistance;
	//fadeOnDistance = 1.0f - saturate( fadeOnDistance );

	// ray tracing steps stores rdotv in w component - always > 0 due to check at start of this method
	float fadeOnPerpendicular = saturate( raySS.w * 4.0f );
	float fadeOnRoughness = saturate( gloss * 4.0f );
	float totalFade =	fadeOnBorder * fadeOnDistance * fadeOnPerpendicular *
						fadeOnRoughness * ( 1.0f - saturate(remainingAlpha) );

	/*float3 nNormal = texelFetch( gBuf_normals, int2( gl_FragCoord.xy ), 0 ).xyz * 2.0 - 1.0;
	float3 viewDir = normalize( float3( inPs.cameraDir.xy, -inPs.cameraDir.z ) );
	float3 reflDir = viewDir - 2.0 * dot( viewDir, nNormal ) * nNormal;
	float3 globalCubemapColour = textureLod( globalCubemap, p_invViewMatCubemap * reflDir,
											 roughness * 12.0 ).xyz;

	float3 fallbackColor = globalCubemapColour;

	fragColour = float4( lerp( fallbackColor, totalColor.rgb, totalFade ), 1.0f );*/

	//TODO: Perform the fallback blending SSR -> Local Cubemap -> Global Cubemap here.
	fragColour = float4( totalColor.rgb, totalFade );

	//fragColour = float4( fadeOnDistance, fadeOnDistance, fadeOnDistance, 1 );
	//fragColour = float4( totalColor.xyz, 1 );
}
