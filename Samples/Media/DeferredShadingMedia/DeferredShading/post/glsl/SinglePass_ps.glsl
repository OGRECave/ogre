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
	
	Post shader: Single pass
*/
uniform sampler2D tex0;
uniform sampler2D tex1;

varying vec2 texCoord;
varying vec3 projCoord;

uniform mat4 proj;

uniform vec3 ambientColor;
// Attributes of light 0
uniform vec4 lightPos0;
uniform vec3 lightDiffuseColor0;
uniform vec3 lightSpecularColor0;
// Attributes of light 1
uniform vec4 lightPos1;
uniform vec3 lightDiffuseColor1;
uniform vec3 lightSpecularColor1;

// Global parameters for lights
struct LightGlobal
{
	vec3 position;
	vec3 normal;
	vec3 viewDir;
};

// Current state of light
struct LightAccum
{
	vec3 light_diffuse;
	vec3 light_specular;
};

// Do lighting calculations for one light
void processLight(
	inout LightAccum accum,
	LightGlobal global,
	vec4 lightPos,
	vec3 lightDiffuseColor,
	vec3 lightSpecularColor)
{
    vec3 lightVec = vec3(lightPos) - global.position;
    vec3 lightDir = normalize(lightVec);
    accum.light_diffuse += max(0.0,dot(lightDir, global.normal)) * lightDiffuseColor;
    
    vec3 h = normalize(global.viewDir + lightDir);
    accum.light_specular += pow(dot(global.normal, h),32.0) * lightSpecularColor;
}

void main()
{
	vec4 a0 = texture2D(tex0, texCoord); // Attribute 0: Diffuse color+shininess
    vec4 a1 = texture2D(tex1, texCoord); // Attribute 1: Normal+depth
    
    // Clip fragment if depth is too far, so the skybox can be rendered on the background
    if(a1.w==0.0)
		discard;
      
    // Attributes
    vec3 colour = a0.rgb;
    float alpha = a0.a;		// Specularity
    float distance = a1.w;  // Distance from viewer -- is zero if no lighting wanted
    
    LightGlobal global;
    global.normal = a1.xyz;
	global.position = projCoord*distance;
    
    // Apply light
    LightAccum accum;
    accum.light_diffuse = vec3(0,0,0);
    accum.light_specular = vec3(0,0,0);
    global.viewDir = -normalize(global.position);
    
    processLight(accum, global, lightPos0, lightDiffuseColor0, lightSpecularColor0);
    processLight(accum, global, lightPos1, lightDiffuseColor1, lightSpecularColor1);
    
    // Calcalate total lighting for this fragment
    vec3 total_light_contrib;
	total_light_contrib = ambientColor+accum.light_diffuse+alpha*accum.light_specular;
	
	// Calculate colour of fragment
    gl_FragColor = vec4( total_light_contrib*colour ,0);
    
    // Calculate depth of fragment; GL requires a 2.0* here as the range is [-1, 1]
    gl_FragDepth = projCoord.z*proj[2][2] + proj[3][2]/(2.0*distance);
}