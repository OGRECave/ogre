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

#define BIAS 0

in vec4 position;
in vec3 normal;
in vec4 uv0;

uniform mat4 worldViewProj;
uniform vec4 lightPosition;
uniform vec3 lightDiffuse;
#if FOG
uniform vec2 fogParams;		// x = fog start, y = fog distance
#endif

#if DEPTH_SHADOWCASTER
uniform vec4 depthRange; // x = min, y = max, z = range, w = 1/range
out float depth;
#endif

#if DEPTH_SHADOWRECEIVER
uniform vec4 depthRange0; // x = min, y = max, z = range, w = 1/range
uniform vec4 depthRange1; // x = min, y = max, z = range, w = 1/range
uniform vec4 depthRange2; // x = min, y = max, z = range, w = 1/range
uniform mat4 texWorldViewProjMatrix0;
uniform mat4 texWorldViewProjMatrix1;
uniform mat4 texWorldViewProjMatrix2;
out vec4 lightSpacePos0;
out vec4 lightSpacePos1;
out vec4 lightSpacePos2;
#endif

#if !SHADOWCASTER
out vec3 col;
#endif

out vec3 diffuseUV;

void main()
{
    // project position to the screen
    gl_Position = worldViewProj * position;

#if !SHADOWCASTER
	// Get object space light direction
	vec3 lightDir = normalize(lightPosition.xyz - (position.xyz * lightPosition.w).xyz);
	col = lightDiffuse.xyz * max(dot(lightDir, normal.xyz), 0.0);
#  if FOG
    diffuseUV.z = linearFog(gl_Position.z, fogParams.x, fogParams.y);
#  endif

#endif

    // pass through other texcoords exactly as they were received
    diffuseUV.xy = uv0.xy;

#if DEPTH_SHADOWCASTER
	depth = (BIAS + gl_Position.z - depthRange.x) * depthRange.w;
#endif

#if DEPTH_SHADOWRECEIVER
	// Calculate the position of vertex in light space
	lightSpacePos0 = texWorldViewProjMatrix0 * position;
	lightSpacePos1 = texWorldViewProjMatrix1 * position;
	lightSpacePos2 = texWorldViewProjMatrix2 * position;

	// make linear
	lightSpacePos0.z = (lightSpacePos0.z - depthRange0.x) * depthRange0.w;
	lightSpacePos1.z = (lightSpacePos1.z - depthRange1.x) * depthRange1.w;
	lightSpacePos2.z = (lightSpacePos2.z - depthRange2.x) * depthRange2.w;

	// pass cam depth
	diffuseUV.z = gl_Position.z;
#endif
}
