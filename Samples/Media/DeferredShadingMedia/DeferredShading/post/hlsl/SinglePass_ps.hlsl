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
sampler Tex0: register(s0);
sampler Tex1: register(s1);

float4x4 proj;

float4 ambientColor;
// Attributes of light 0
float4 lightPos0;
float4 lightDiffuseColor0;
float4 lightSpecularColor0;
// Attributes of light 1
float4 lightPos1;
float4 lightDiffuseColor1;
float4 lightSpecularColor1;

// Global parameters for lights
struct LightGlobal
{
	float3 position;
	float3 normal;
	float3 viewDir;
};

// Current state of light
struct LightAccum
{
	float3 light_diffuse;
	float3 light_specular;
};

// Do lighting calculations for one light
void processLight(
	inout LightAccum accum,
	LightGlobal global,
	float4 lightPos,
	float4 lightDiffuseColor,
	float4 lightSpecularColor)
{
    float3 lightVec = lightPos - global.position;
    float3 lightDir = normalize(lightVec);
    accum.light_diffuse += max(0,dot(lightDir, global.normal)) * lightDiffuseColor;
    
    float3 h = normalize(global.viewDir + lightDir);
    accum.light_specular += pow(dot(global.normal, h),32) * lightSpecularColor;
}

struct POUTPUT
{
	float4 colour: COLOR;
	float depth: DEPTH;
};

POUTPUT main(float2 texCoord: TEXCOORD0, float3 projCoord: TEXCOORD1)
{
	POUTPUT o;
	
    float4 a0 = tex2D(Tex0, texCoord); // Attribute 0: Diffuse color+shininess
    float4 a1 = tex2D(Tex1, texCoord); // Attribute 1: Normal+depth
    
    LightGlobal global; 
    
    // Clip fragment if depth is too far, so the skybox can be rendered on the background
    clip(a1.w-0.001);
      
    // Attributes
    float3 colour = a0.rgb;
    float alpha = a0.a;		// Specularity
    float distance = a1.w;  // Distance from viewer -- is zero if no lighting wanted
    //global.normal = normalize(a1.xyz); // normalizing done already
    global.normal = a1.xyz;

    // Acquire view space position via inverse projection transformation
    //global.position = mul(invProj, float4(projCoord, 0, 1))*distance;
    // Acquire view space position via inverse projection transformation
    //float4 tpos;
    //tpos = float4(projCoord, distance, 1.0);
    //tpos = mul(invProj, tpos);
    //tpos = tpos / tpos.w;
    //global.position = tpos;
    //global.position = float3(
	//	invProj[0][0], // X vector component from X
	//	invProj[1][1], // Y vector component from Y
	//	invProj[2][3]  // Z vector component from W
	//)*projCoord*distance;
	global.position = projCoord*distance;
    
    // Apply light
    LightAccum accum;
    accum.light_diffuse = float3(0,0,0);
    accum.light_specular = float3(0,0,0);
    global.viewDir = -normalize(global.position);
    
    processLight(accum, global, lightPos0, lightDiffuseColor0, lightSpecularColor0);
    processLight(accum, global, lightPos1, lightDiffuseColor1, lightSpecularColor1);
    
    // Calcalate total lighting for this fragment
    float3 total_light_contrib;
	total_light_contrib = ambientColor+accum.light_diffuse+alpha*accum.light_specular;
	
    o.colour = float4( total_light_contrib*colour ,0);
    // Depth buffer value
    // Transfering depth makes it possible to render particle effects and other transparent
    // things unaffected by light in the postprocessing phase.
    o.depth = projCoord.z*proj[2][2] + proj[2][3]/distance;
    return o;
}

