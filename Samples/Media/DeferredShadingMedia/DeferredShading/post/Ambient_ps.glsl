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

#version 150

in vec2 oUv0;
in vec3 oRay;

out vec4 oColour;

uniform sampler2D Tex0;
uniform sampler2D Tex1;
uniform mat4 proj;
uniform vec4 ambientColor;
uniform float farClipDistance;

float finalDepth(vec4 p)
{
    // GL needs it in [0..1]
    return (p.z / p.w) * 0.5 + 0.5;
}

void main()
{
	vec4 a0 = texture(Tex0, oUv0); // Attribute 0: Diffuse color+shininess
	vec4 a1 = texture(Tex1, oUv0); // Attribute 1: Normal+depth

	// Clip fragment if depth is too close, so the skybox can be rendered on the background
	if((a1.w - 0.0001) < 0.0)
        discard;

	// Calculate ambient colour of fragment
	oColour = vec4(ambientColor * vec4(a1.rgb,0));

	// Calculate depth of fragment;
	vec3 viewPos = normalize(oRay) * farClipDistance * a1.w;
	vec4 projPos = proj * vec4(viewPos, 1);
	gl_FragDepth = finalDepth(projPos);
}
