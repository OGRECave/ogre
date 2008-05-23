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
sampler Tex0: register(s0);
sampler Tex1: register(s1);

float4x4 worldView;

// Attributes of light
float4 lightDiffuseColor;
float4 lightSpecularColor;
float4 lightFalloff;

float4 main(float2 texCoord: TEXCOORD0, float3 projCoord: TEXCOORD1) : COLOR 
{
	float4 a0 = tex2D(Tex0, texCoord); // Attribute 0: Diffuse color+shininess
	float4 a1 = tex2D(Tex1, texCoord); // Attribute 1: Normal+depth

	// Attributes
	float3 colour = a0.rgb;
	float alpha = a0.a;		// Specularity
	float distance = a1.w;  // Distance from viewer (w)
	float3 normal = a1.xyz;

	// Calculate position of texel in view space
	float3 position = projCoord*distance;

	// Extract position in view space from worldView matrix
	float3 lightPos = float3(worldView[0][3],worldView[1][3],worldView[2][3]);

	// Calculate light direction and distance
	float3 lightVec = lightPos - position;
	float len_sq = dot(lightVec, lightVec);
	float len = sqrt(len_sq);
	float3 lightDir = lightVec/len;

	// Calculate diffuse colour
	float3 total_light_contrib;
	total_light_contrib = max(0.0,dot(lightDir, normal)) * lightDiffuseColor;

#if IS_SPECULAR
	// Calculate specular component
	float3 viewDir = -normalize(position);
	float3 h = normalize(viewDir + lightDir);
	float3 light_specular = pow(dot(normal, h),32.0) * lightSpecularColor;

	total_light_contrib += alpha * light_specular;
#endif

#if IS_ATTENUATED
	// Calculate attenuation
	float attenuation = dot(lightFalloff, float3(1.0, len, len_sq));
	total_light_contrib /= attenuation;
#endif
	return float4(total_light_contrib*colour, 0.0);
}
// Debugging only
//return lightDiffuseColor;
//return lightDiffuseColor/attenuation;
//return float4(abs(position-position2),0);
//return length(lightPos-position)/1000.0;
//return float4(abs(lightPos-lightPos2),0);
//return float4(lightPos-position,0);
//return float4(-global.position/1000.0f,0);
//return float4(global.position/1000.0,0);
///return float4(lightPos[0], lightPos[1], -lightPos[2], 0)/1000.0f;
//return lightDiffuseColor*a0.xyzw/2;

