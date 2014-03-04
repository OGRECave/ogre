#version 150
/* Copyright Torus Knot Software Ltd 2000-2014

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
*/

in vec3 diffuseUV;
uniform sampler2D diffuseMap;
uniform vec3 materialAmbient;

#if !SHADOWCASTER
in vec3 col;
#endif
#if DEPTH_SHADOWCASTER
in float depth;
#endif

#if DEPTH_SHADOWRECEIVER
in vec4 lightSpacePos0;
in vec4 lightSpacePos1;
in vec4 lightSpacePos2;
uniform sampler2D shadowMap0;
uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;
uniform float inverseShadowmapSize0;
uniform float inverseShadowmapSize1;
uniform float inverseShadowmapSize2;
uniform vec4 pssmSplitPoints;
#endif
	
#if SHADOWCASTER
uniform vec3 shadowColour;
#endif
#if FOG
uniform vec3 fogColour;
#endif

out vec4 fragColour;

// Number of samples in one dimension (square for total samples)
#define NUM_SHADOW_SAMPLES_1D 2.0
#define SHADOW_FILTER_SCALE 1

#define SHADOW_SAMPLES NUM_SHADOW_SAMPLES_1D*NUM_SHADOW_SAMPLES_1D

vec4 offsetSample(vec4 uv, vec2 offset, float invMapSize)
{
	return vec4(uv.xy + offset * invMapSize * uv.w, uv.z, uv.w);
}

float calcDepthShadow(sampler2D shadowMap, vec4 uv, float invShadowMapSize)
{
	// 4-sample PCF
	
	float shadow = 0.0;
	float offset = (NUM_SHADOW_SAMPLES_1D/2 - 0.5) * SHADOW_FILTER_SCALE;
	for (float y = -offset; y <= offset; y += SHADOW_FILTER_SCALE)
		for (float x = -offset; x <= offset; x += SHADOW_FILTER_SCALE)
		{
			float depth = textureProj(shadowMap, offsetSample(uv, vec2(x, y), invShadowMapSize)).x;
			if (depth >= 1 || depth >= uv.z)
				shadow += 1.0;
		}

	shadow /= SHADOW_SAMPLES;

	return shadow;
}

float calcPSSMDepthShadow(sampler2D shadowMap0, sampler2D shadowMap1, sampler2D shadowMap2, 
						   vec4 lsPos0, vec4 lsPos1, vec4 lsPos2,
						   float invShadowmapSize0, float invShadowmapSize1, float invShadowmapSize2,
						   vec4 pssmSplitPoints, float camDepth)
{

	float shadow;
	vec4 splitColour;
	// calculate shadow
	if (camDepth <= pssmSplitPoints.y)
	{
		splitColour = vec4(0.3, 0.0, 0, 0);
		shadow = calcDepthShadow(shadowMap0, lsPos0, invShadowmapSize0);
	}
	else if (camDepth <= pssmSplitPoints.z)
	{
		splitColour = vec4(0, 0.3, 0, 0);
		shadow = calcDepthShadow(shadowMap1, lsPos1, invShadowmapSize1);
	}
	else
	{
		splitColour = vec4(0.0, 0.0, 0.3, 0);
		shadow = calcDepthShadow(shadowMap2, lsPos2, invShadowmapSize2);
	}

	return shadow;
}

void main()
{
    // look up the diffuse map layer
    vec4 texDiffuse = texture(diffuseMap, diffuseUV.xy);
    
#if SHADOWCASTER
#  if DEPTH_SHADOWCASTER
	// early-out with depth (we still include alpha for those cards that support it)
	fragColour = vec4(depth, depth, depth, 1);
#  else
	fragColour = vec4(shadowColour.xyz, texDiffuse.a);
#  endif

#else
    // compute the ambient contribution (pulled from the diffuse map)
    vec3 vAmbient = texDiffuse.xyz * materialAmbient.xyz;
    vec3 vColor3 = texDiffuse.rgb * col.rgb;

#  if DEPTH_SHADOWRECEIVER
	float camDepth = diffuseUV.z;
	float shadow = calcPSSMDepthShadow(shadowMap0, shadowMap1, shadowMap2, 
		lightSpacePos0, lightSpacePos1, lightSpacePos2,
		inverseShadowmapSize0, inverseShadowmapSize1, inverseShadowmapSize2,
		pssmSplitPoints, camDepth);
	vColor3 *= shadow;
#  endif

    fragColour = vec4(vColor3 + vAmbient, texDiffuse.a);
    
#  if FOG
    // if fog is active, interpolate between the unfogged color and the fog color
    // based on vertex shader fog value
    fragColour.rgb = mix(vColor.rgb, fogColour, diffuseUV.z).rgb;
#  endif

#endif
}
