
#define NUM_SHADOW_SAMPLES_1D 2.0
#define SHADOW_FILTER_SCALE 0.5

#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D

vec4 offsetSample(vec4 uv, vec2 offset, float invMapSize)
{
	return vec4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w);
}

float calcDepthShadowLod(sampler2D shadowMap, vec4 uv, float invShadowMapSize)
{
	// 4-sample PCF

	float shadow = 0.0;
	float offset = (NUM_SHADOW_SAMPLES_1D/2 - 0.5) * SHADOW_FILTER_SCALE;
	for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE)
	{
		for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE)
		{
			vec4 finalUv = offsetSample(uv, vec2(x, y), invShadowMapSize);
			finalUv.xy /= finalUv.w;
            float depth = texture2DLod( shadowMap, finalUv.xy, 0 ).x;
			if( depth >= 1 || depth >= uv.z )
				shadow += 1.0;
		}
	}

	shadow /= SHADOW_SAMPLES;

	return shadow;
}

float calcPSSMDepthShadow( sampler2D shadowMap0, sampler2D shadowMap1, sampler2D shadowMap2,
						   vec4 lsPos0, vec4 lsPos1, vec4 lsPos2,
						   float invShadowmapSize0, float invShadowmapSize1, float invShadowmapSize2,
						   vec4 pssmSplitPoints, float camDepth )
{

	float shadow;
	vec4 splitColour;
	// calculate shadow
	if (camDepth <= pssmSplitPoints.x)
	{
		splitColour = vec4(0.3, 0.0, 0, 0);
		shadow = calcDepthShadowLod(shadowMap0, lsPos0, invShadowmapSize0);
	}
	else if (camDepth <= pssmSplitPoints.y)
	{
		splitColour = vec4(0, 0.3, 0, 0);
		shadow = calcDepthShadowLod(shadowMap1, lsPos1, invShadowmapSize1);
	}
	else
	{
		splitColour = vec4(0.0, 0.0, 0.3, 0);
		shadow = calcDepthShadowLod(shadowMap2, lsPos2, invShadowmapSize2);
	}

	return shadow;
}

#define NUM_LIGHTS 8

uniform vec4 lightAmbient;
uniform vec4 lightPosition[NUM_LIGHTS];
uniform vec4 lightDiffuse[NUM_LIGHTS];

uniform float invShadowMapSize0;
uniform float invShadowMapSize1;
uniform float invShadowMapSize2;
#ifdef PSSM
	uniform vec4 pssmSplits;
#endif

uniform sampler2D diffuseMap;
uniform sampler2D shadowMap0;
uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;

void main()
{
    const vec2 psUv         = gl_TexCoord[0].xy;
    const vec3 psWorldPos   = gl_TexCoord[1].xyz;
    const vec3 psNorm       = gl_TexCoord[2].xyz;

    const vec4 psLightSpacePos0 = gl_TexCoord[3].xyzw;

    #ifdef PSSM
        const vec4 psLightSpacePos1 = gl_TexCoord[4].xyzw;
        const vec4 psLightSpacePos2 = gl_TexCoord[5].xyzw;
        const float psDepth         = gl_TexCoord[6].x;
    #endif

#ifdef PSSM
	float fShadow = calcPSSMDepthShadow( shadowMap0, shadowMap1, shadowMap2,
									psLightSpacePos0, psLightSpacePos1, psLightSpacePos2,
									invShadowMapSize0, invShadowMapSize1, invShadowMapSize2,
									pssmSplits, psDepth );
#else
	float fShadow = calcDepthShadowLod( shadowMap0, psLightSpacePos0, invShadowMapSize0 );
#endif

	vec3 negLightDir = lightPosition[0].xyz - (psWorldPos * lightPosition[0].w);
	negLightDir	= normalize( negLightDir );
	vec3 normal = normalize( psNorm );
    gl_FragColor = max( 0, dot( negLightDir, normal ) ) * lightDiffuse[0] * fShadow + lightAmbient;

	int i=1;
	for( i=1; i<NUM_LIGHTS; ++i )
	{
		vec3 negLightDir = lightPosition[i].xyz - (psWorldPos * lightPosition[i].w);
		negLightDir	= normalize( negLightDir );
        gl_FragColor += max( 0, dot( negLightDir, normal ) ) * lightDiffuse[i];
	}

    gl_FragColor *= texture2D( diffuseMap, psUv );

	/*if( inDepth <= pssmSplits.x)
        gl_FragColor.xyz = lerp( gl_FragColor.xyz, vec3( 0, 0, 1.0f ), 0.25f );
	else if( inDepth <= pssmSplits.y)
        gl_FragColor.xyz = lerp( gl_FragColor.xyz, vec3( 0, 1.0f, 0 ), 0.25f );
	else
        gl_FragColor.xyz = lerp( gl_FragColor.xyz, vec3( 1.0f, 0, 0 ), 0.25f );*/
}
