#define NUM_SHADOW_SAMPLES_1D 2.0
#define SHADOW_FILTER_SCALE 0.5

#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D

float4 offsetSample(float4 uv, float2 offset, float invMapSize)
{
	return float4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w);
}

float calcDepthShadowLod(sampler2D shadowMap, float4 uv, float invShadowMapSize)
{
	// 4-sample PCF
	
	float shadow = 0.0;
	float offset = (NUM_SHADOW_SAMPLES_1D/2 - 0.5) * SHADOW_FILTER_SCALE;
	for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE)
		for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE)
		{
			float4 finalUv = offsetSample(uv, float2(x, y), invShadowMapSize);
			finalUv.xy /= finalUv.w;
			float depth = tex2Dlod(shadowMap, finalUv.xyyy ).x;
			if (depth >= 1 || depth >= uv.z)
				shadow += 1.0;
		}

	shadow /= SHADOW_SAMPLES;

	return shadow;
}

float calcPSSMDepthShadow( sampler2D shadowMap0, sampler2D shadowMap1, sampler2D shadowMap2, 
						   float4 lsPos0, float4 lsPos1, float4 lsPos2,
						   float invShadowmapSize0, float invShadowmapSize1, float invShadowmapSize2,
						   float4 pssmSplitPoints, float camDepth)
{

	float shadow;
	float4 splitColour;
	// calculate shadow
	if (camDepth <= pssmSplitPoints.x)
	{
		splitColour = float4(0.3, 0.0, 0, 0);
		shadow = calcDepthShadowLod(shadowMap0, lsPos0, invShadowmapSize0);
	}
	else if (camDepth <= pssmSplitPoints.y)
	{
		splitColour = float4(0, 0.3, 0, 0);
		shadow = calcDepthShadowLod(shadowMap1, lsPos1, invShadowmapSize1);
	}
	else
	{
		splitColour = float4(0.0, 0.0, 0.3, 0);
		shadow = calcDepthShadowLod(shadowMap2, lsPos2, invShadowmapSize2);
	}

	return shadow;
}

#define NUM_LIGHTS 8

void main_ps(	in float2 uv : TEXCOORD0,
				in float3 objPos : TEXCOORD1,
				in float3 normal : TEXCOORD2,
				out float4 outColour : COLOR0,

				uniform float4 lightAmbient,
				uniform float4 lightPosition[NUM_LIGHTS],
				uniform float4 lightDiffuse[NUM_LIGHTS],
				
                uniform sampler2D diffuseMap : register(s0),
                uniform sampler2D shadowMap0 : register(s1),

                uniform float invShadowMapSize0,

                in float4 psLightSpacePos0	: TEXCOORD3
    #ifdef PSSM
            ,   in float psDepth			: TEXCOORD4,
                in float4 psLightSpacePos1	: TEXCOORD5,
                in float4 psLightSpacePos2	: TEXCOORD6,

                uniform sampler2D shadowMap1 : register(s2),
                uniform sampler2D shadowMap2 : register(s3),

                uniform float invShadowMapSize1,
                uniform float invShadowMapSize2,
                uniform float4 pssmSplits
    #endif
				)
{
#ifdef PSSM
    float fShadow = calcPSSMDepthShadow( shadowMap0, shadowMap1, shadowMap2,
                                    psLightSpacePos0, psLightSpacePos1, psLightSpacePos2,
                                    invShadowMapSize0, invShadowMapSize1, invShadowMapSize2,
                                    pssmSplits, psDepth );
#else
    float fShadow = calcDepthShadowLod( shadowMap0, psLightSpacePos0, invShadowMapSize0 );
#endif

	float3 negLightDir = lightPosition[0].xyz - (objPos * lightPosition[0].w);
	negLightDir	= normalize( negLightDir );
	normal		= normalize( normal );
	outColour = max( 0, dot( negLightDir, normal ) ) * lightDiffuse[0] * fShadow + lightAmbient;
	
	int i=1;
	for( i=1; i<NUM_LIGHTS; ++i )
	{
		float3 negLightDir = lightPosition[i].xyz - (objPos * lightPosition[i].w);
		negLightDir	= normalize( negLightDir );
		outColour += max( 0, dot( negLightDir, normal ) ) * lightDiffuse[i];
	}

	outColour *= tex2D( diffuseMap, uv );

	/*if( inDepth <= pssmSplits.x)
		outColour.xyz = lerp( outColour.xyz, float3( 0, 0, 1.0f ), 0.25f );
	else if( inDepth <= pssmSplits.y)
		outColour.xyz = lerp( outColour.xyz, float3( 0, 1.0f, 0 ), 0.25f );
	else
		outColour.xyz = lerp( outColour.xyz, float3( 1.0f, 0, 0 ), 0.25f );*/
}
