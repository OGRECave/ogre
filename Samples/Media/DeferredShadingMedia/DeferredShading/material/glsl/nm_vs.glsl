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
	
	Material shader: Normal mapped
*/
varying vec3 normal;
varying vec3 tangent_;
varying vec3 binormal;
varying vec2 texCoord0;
varying float depth;
attribute vec3 tangent;
void main()                    
{
   vec4 projPos = gl_ModelViewProjectionMatrix * gl_Vertex;

   gl_Position = projPos;
   depth = projPos.w;
   texCoord0 = vec2(gl_MultiTexCoord0);
   
   normal = vec3(gl_ModelViewMatrix*vec4(gl_Normal,0));
   tangent_ = vec3(gl_ModelViewMatrix*vec4(tangent,0));
   binormal = cross(normal, tangent_);

}
