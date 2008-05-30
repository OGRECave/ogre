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
	
	Post shader: Light geometry material
*/
varying vec2 texCoord;
varying vec3 projCoord;

uniform float vpWidth;
uniform float vpHeight;
uniform mat4 invProj;

void main()                    
{
	vec4 projPos = gl_ModelViewProjectionMatrix * gl_Vertex;
	projPos = projPos/projPos.w;

	// projPos is now in nonhomogeneous 2d space -- this makes sure no perspective interpolation is
	// done that could mess with our concept.
	//gl_Position = projPos;
	vec2 texSize = vec2(vpWidth, vpHeight);
	vec2 tcTemp = vec2(projPos[0], projPos[1])/2.0+0.5;
	tcTemp = (floor(tcTemp * texSize)+0.5)/texSize;

	//float3 position = mul(invProj, float4(projCoord, 0, 1))*distance; 
	// Acquire view space position via inverse projection transformation
	// Optimisation for perspective, symmetric view frustrums 
	// These interpolate over the frustrum plane for w=1

	projCoord = vec3(projPos[0], projPos[1], 1)*vec3(
		invProj[0][0], // X vector component from X
		invProj[1][1], // Y vector component from Y
		invProj[3][2]  // Z vector component from W
	);

	//projCoord = vec3(invProj*vec4(projPos[0], projPos[1], 0, 1));

	// Texture coordinate magic, this compensates for jitter
	texCoord = vec2(tcTemp[0], 1.0-tcTemp[1]);   
	gl_Position = vec4((tcTemp-0.5)*2.0, 0.0, 1.0);
}
