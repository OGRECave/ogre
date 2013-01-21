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
	
	Post shader: Generic fullscreen quad
*/
#version 150

in vec4 vertex;

out vec2 oUv0;
out vec3 oRay;

uniform vec3 farCorner;
uniform float flip;

void main()
{
	// Clean up inaccuracies
	vec2 Pos = sign(vertex.xy);

	// Image-space
    oUv0 = (vec2(Pos.x, -Pos.y) + 1.0) * 0.5;

	// This ray will be interpolated and will be the ray from the camera
	// to the far clip plane, per pixel
	oRay = farCorner * vec3(Pos, 1);

	gl_Position = vec4(Pos, 0, 1);
	gl_Position.y *= flip;
}
