/******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/
/** Deferred shading framework
	// W.J. :wumpus: van der Laan 2005 //
	
	Post shader: Multipass, ambient (base) pass
*/
uniform sampler2D tex0;
uniform sampler2D tex1;

varying vec2 texCoord;
varying vec3 projCoord;

uniform mat4 proj;

uniform vec3 ambientColor;

void main()
{
	vec4 a0 = texture2D(tex0, texCoord); // Attribute 0: Diffuse color+shininess
	vec4 a1 = texture2D(tex1, texCoord); // Attribute 1: Normal+depth

	// Clip fragment if depth is too far, so the skybox can be rendered on the background
	if(a1.w==0.0)
		discard;

	// Calculate ambient colour of fragment
	gl_FragColor = vec4( ambientColor*a0.rgb ,0);

	// Calculate depth of fragment; GL requires a 2.0* here as the range is [-1, 1]
	// Also, see again how matrix is transposed ([3][2] instead of [2][3])
	gl_FragDepth = projCoord.z*proj[2][2] + proj[3][2]/(2.0*a1.w);
}
