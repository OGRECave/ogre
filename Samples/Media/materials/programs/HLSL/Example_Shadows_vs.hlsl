
void main_vs( in float4 inPos	: POSITION,
			in float3 inNorm	: NORMAL,
			in float2 inUv		: TEXCOORD0,

			out float4 outPos		: POSITION
			
#ifdef DEPTH_SHADOWCASTER
		,	out float outDepth		: TEXCOORD0
        ,   uniform float shadowConstantBias
#else
        ,	out float2 outUv		: TEXCOORD0
        ,	out float3 outWorldPos	: TEXCOORD1
        ,	out float3 outNorm		: TEXCOORD2
			

        ,	out float4 outLightSpacePos0	: TEXCOORD3
    #ifdef PSSM
        ,	out float outDepth				: TEXCOORD4
		,	out float4 outLightSpacePos1	: TEXCOORD5
        ,	out float4 outLightSpacePos2	: TEXCOORD6
    #endif
#endif
			
		,	uniform float4x4 worldViewProj
			
#ifndef DEPTH_SHADOWCASTER
		,	uniform float4x4 world
        ,	uniform float4x4 texViewProjMatrix0
#endif

        ,	uniform float4 depthRange0

#ifdef PSSM
		,	uniform float4x4 texViewProjMatrix1
		,	uniform float4x4 texViewProjMatrix2
		,	uniform float4 depthRange1
		,	uniform float4 depthRange2
#endif
			)
{
	outPos		= mul( worldViewProj, inPos );
#ifdef DEPTH_SHADOWCASTER
    outDepth	= (outPos.z - depthRange0.x + shadowConstantBias) * depthRange0.w;

    //We can't make the depth buffer linear without Z out in the fragment shader;
    //however we can use a cheap approximation ("pseudo linear depth")
    //see http://yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
	outPos.z	= outPos.z * (outPos.w * depthRange0.w);
#else
	outWorldPos	= mul( world, inPos );
	outNorm		= mul( (float3x3)world, inNorm );
	outUv		= inUv;

		// Calculate the position of vertex in light space to do shadows
		float4 shadowWorldPos = float4( outWorldPos, 1.0f );
		outLightSpacePos0 = mul( texViewProjMatrix0, shadowWorldPos );
		// make linear
		outLightSpacePos0.z = (outLightSpacePos0.z - depthRange0.x) * depthRange0.w;

    #ifdef PSSM
		outLightSpacePos1	= mul( texViewProjMatrix1, shadowWorldPos );
		outLightSpacePos1.z	= (outLightSpacePos1.z - depthRange1.x) * depthRange1.w;
		outLightSpacePos2	= mul( texViewProjMatrix2, shadowWorldPos );
		outLightSpacePos2.z	= (outLightSpacePos2.z - depthRange2.x) * depthRange2.w;
		
		outDepth = outPos.z;
	#endif
#endif
}
