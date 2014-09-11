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

void SGX_TriplanarTexturing(in vec4 diffuse, in vec3 normal, in vec4 position, in sampler2D texFromX, in sampler2D texFromY, in sampler2D texFromZ, in vec3 parameters, out vec4 cOut) {

	vec3 blendWeights = abs(normalize(normal));
	blendWeights = blendWeights - vec3(parameters.yyy);
	blendWeights = vec3(pow(max(blendWeights.x, 0), parameters.z), pow(max(blendWeights.y, 0), parameters.z), pow(max(blendWeights.z, 0), parameters.z));
	float sum = blendWeights.x + blendWeights.y + blendWeights.z;
	blendWeights =  blendWeights/vec3(sum, sum, sum);
	// Move the planar mapping a bit according to the normal length to avoid bad looking skirts.
	float nLength = length(normal - 1.0);
	vec2 coord1 = (position.yz + nLength) * parameters.x;
	vec2 coord2 = (position.zx + nLength) * parameters.x;
	vec2 coord3 = (position.xy + nLength) * parameters.x;
	
	vec4 col1 = texture2D(texFromX, coord1);
	vec4 col2 = texture2D(texFromY, coord2);
	vec4 col3 = texture2D(texFromZ, coord3);
	cOut = diffuse * vec4(col1.xyz * blendWeights.x +
		col2.xyz * blendWeights.y +
		col3.xyz * blendWeights.z, 1);
}