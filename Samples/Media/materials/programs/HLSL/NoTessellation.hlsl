/////////////
// GLOBALS //
/////////////
cbuffer cbPerFrame
{
    matrix worldViewprojMatrix;
    matrix worldMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position 		: POSITION;
    float3 normal 			: NORMAL;
    float3 tangent 			: TANGENT;
    float2 tex0				: TEXCOORD0;
};

struct PixelInputType
{
    float4 position 		: SV_POSITION;
    float3 tangent 			: TANGENT;
    float2 tex0				: TEXCOORD0;
    float4 worldPosition   	: TEXCOORD1;
    float3 binormal 		: TEXCOORD2;
    float3 normal 			: TEXCOORD3;
};

cbuffer LightBuffer
{
    float4 lightAmbientColor;
	float4 lightDiffuseColor;
    float4 lightSpecularColor;
    float3 lightDirection;
	float4 lightPosition;
	float4 lightAttenuation;
	float4 surfaceDiffuseColour;
	float4 surfaceSpecularColour;
	float  surfaceShininess;
	float3 cameraPosition;
};

PixelInputType color_vs(VertexInputType input) //: SV_Position
{
	PixelInputType output;
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position 	 = mul(worldViewprojMatrix, input.position);
	output.worldPosition = mul(worldMatrix, input.position);
    
    // Store the input color for the pixel shader to use.
    output.tex0 = input.tex0;
	
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);
	
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	output.tangent = normalize(output.tangent);
	
	output.binormal = cross(input.tangent, input.normal);
	output.binormal = mul(output.binormal, (float3x3)worldMatrix);
	output.binormal = normalize(output.binormal);
    
    return output;
}

Texture2D shaderTextures[3];
SamplerState SampleType;

float4 color_ps(PixelInputType input) : SV_TARGET
{
	float4 texDiffuseColor;
	float4 texBumpMap;
	float4 texSpecularColor;
    float3 bumpNormal;
    float3 lightIntensity;
	float3 diffuseContribution;
    float4 color;
	float3 cameraDirection;
	float3 halfVector;
	float3 specular;
	float3 specularContribution;
	half lightDistance;
	half iluminationLightAttenuation;
	half calculatedLightAttenuation;
	
	texDiffuseColor  = shaderTextures[0].Sample(SampleType, input.tex0);
	texBumpMap 		 = shaderTextures[1].Sample(SampleType, input.tex0);
	texSpecularColor = shaderTextures[2].Sample(SampleType, input.tex0);
	
	// Expand the range of the normal value from (0, +1) to (-1, +1).
    texBumpMap = (texBumpMap * 2.0f) - 1.0f;
	
	// Calculate the normal from the data in the bump map.
    bumpNormal = input.normal + texBumpMap.x * input.tangent + texBumpMap.y * input.binormal;
	
	 // Normalize the resulting bump normal.
    bumpNormal = normalize(bumpNormal);
		
	 // Calculate the amount of light on this pixel based on the bump map normal value.
    lightIntensity = max(dot(bumpNormal, lightDirection), 0);
	
	//Ilumination Light Attenuation
	lightDistance = length( lightPosition.xyz - input.worldPosition.xyz) / lightAttenuation.r;
	iluminationLightAttenuation = lightDistance * lightDistance; // quadratic falloff
	calculatedLightAttenuation = 1.0 - iluminationLightAttenuation;
	
	cameraDirection = normalize(cameraPosition - input.worldPosition.xyz);
	halfVector = normalize( lightDirection + cameraDirection);
	specular = pow(max(dot(bumpNormal, halfVector), 0), surfaceShininess);
	
	// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
	diffuseContribution  = (lightIntensity * lightDiffuseColor.rgb * texDiffuseColor.rgb);// * surfaceDiffuseColour.rgb);
	specularContribution = (specular * lightSpecularColor.rgb * texSpecularColor.rgb * surfaceSpecularColour.rgb);
	float3 lightContributtion = (diffuseContribution + specularContribution) * calculatedLightAttenuation;
	
     // Combine the final bump light color with the texture color.
	color = float4(lightContributtion, texDiffuseColor.a);
	
    return color;
}