/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

//-----------------------------------------------------------------------------
// Program Name: SGXLib_NormalMapLighting
// Program Desc: Normal map lighting functions.
// Program Type: Vertex/Pixel shader
// Language: GLSL
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void SGX_FetchNormal(in sampler2D s,
				   in vec2 uv,
				   out vec3 vOut)
{
	vOut = 2.0 * texture2D(s, uv).xyz - 1.0;
}

//-----------------------------------------------------------------------------
void SGX_CalculateTBN(in vec3 vNormal,
					  in vec4 vTangent,
					  out mat3 TBN)
{
	vec3 vBinormal = cross(vNormal, vTangent.xyz) * vTangent.w;

	// direction: from tangent space to world
	TBN = mtxFromCols(vTangent.xyz, vBinormal, vNormal);
}

//-----------------------------------------------------------------------------
void SGX_Generate_Parallax_Texcoord(in sampler2D normalHeightMap,
						in vec2 texCoord,
						in vec3 viewPos,
						in float heightScale,
						in mat3 TBN,
						out vec2 newTexCoord)
{
	//Calculate eye direction
	vec3 eyeVec = mul(-viewPos, TBN);
	eyeVec = normalize(eyeVec);
#ifndef TERRAIN_PARALLAX_MAPPING
	eyeVec.y = -eyeVec.y; //Inverse y
#endif

	newTexCoord = texCoord;

#ifndef POM_LAYER_COUNT
	//Simple parallax mapping
	float height = 1.0f - texture2D(normalHeightMap, newTexCoord).a;

	#ifndef TERRAIN_PARALLAX_MAPPING
		vec2 p = eyeVec.xy / eyeVec.z * (height * heightScale);
	#else
	    vec2 p = eyeVec.xy * (height * heightScale);
	#endif

    newTexCoord = newTexCoord - p;
#else
	// parallax occlusion mapping
#ifdef POM_MAX_DISTANCE
	if (abs(viewPos.z) > POM_MAX_DISTANCE)
		return;
#endif

	//Configure steep mapping layering.
	float layerDepth = 1.0 / float(POM_LAYER_COUNT);
	float currentLayerDepth = 0.0;
	vec2 parallaxShift = (eyeVec.xy) * heightScale;
	vec2 deltaTexCoords = parallaxShift / float(POM_LAYER_COUNT);

	float currentDepthMapValue = 1.0f - texture2D(normalHeightMap, newTexCoord).a;

	//Loop through layers and break early if match found.
	for (int currentLayerId = 0; currentLayerId < POM_LAYER_COUNT; currentLayerId++)
	{
		// shift texture coordinates along direction of P
		newTexCoord -= deltaTexCoords;

		// get depthmap value at current texture coordinates
		currentDepthMapValue = 1.0f - texture2D(normalHeightMap, newTexCoord).a;

		//Break if layer height matched
		if (currentLayerDepth > currentDepthMapValue)
			break;

		// get depth of next layer
		currentLayerDepth += layerDepth;
	}

	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = newTexCoord + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = (1.0f - texture2D(normalHeightMap, prevTexCoords).a) - currentLayerDepth + layerDepth;

	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	newTexCoord = mix(newTexCoord, prevTexCoords, weight);
#endif
}