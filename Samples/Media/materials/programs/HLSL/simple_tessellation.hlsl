/////////////
// GLOBALS //
/////////////
cbuffer cbPerFrame : register( b0 )
{
    matrix worldViewprojMatrix;
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer TessellationBuffer
{
    float tessellationAmount;
    //float3 padding;
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

struct VertexInputType2
{
    float3 position 		: POSITION;
    float2 texCoord     	: TEXCOORD0;
    float4 color 			: COLOR;
};

struct HullInputType
{
    float3 position 		: POSITION;
    float2 texCoord     	: TEXCOORD0;
    float4 color 			: COLOR;
};

struct ConstantOutputType
{
    float edges[3]  : SV_TessFactor;
    float inside 	: SV_InsideTessFactor;
};

//The HullOutputType structure is what will be the output from the hull shader.

struct HullOutputType
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color 	: COLOR;
};

struct PixelInputType2
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR;
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

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
HullInputType color_tessellation_vs(VertexInputType2 input)
{
    HullInputType output;
	
	// Pass the vertex position into the hull shader.
    output.position = input.position;
    
	output.texCoord = input.texCoord;
    // Pass the input color into the hull shader.
    output.color = input.color;
    
    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Patch Constant Function
////////////////////////////////////////////////////////////////////////////////
ConstantOutputType ColorPatchConstantFunction(InputPatch<HullInputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{    
    ConstantOutputType output;


    // Set the tessellation factors for the three edges of the triangle.
    output.edges[0] = tessellationAmount;
    output.edges[1] = tessellationAmount;
    output.edges[2] = tessellationAmount;

    // Set the tessellation factor for tessallating inside the triangle.
    output.inside = tessellationAmount;

    return output;
}

//The inputs to the domain shader are the outputs from the hull shader and constant function.
////////////////////////////////////////////////////////////////////////////////
// Hull Shader
////////////////////////////////////////////////////////////////////////////////
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ColorPatchConstantFunction")]
HullOutputType color_tessellation_hs(InputPatch<HullInputType, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    HullOutputType output;


    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;
	
	output.texCoord = patch[pointId].texCoord;
    // Set the input color as the output color.
    output.color = patch[pointId].color;

    return output;
}

//The inputs to the domain shader are the outputs from the hull shader and constant function.
////////////////////////////////////////////////////////////////////////////////
// Domain Shader
////////////////////////////////////////////////////////////////////////////////
[domain("tri")]
PixelInputType2 color_tessellation_ds(ConstantOutputType input, float3 barycentricCoordinates : SV_DomainLocation, const OutputPatch<HullOutputType, 3> patch)
{
    float3 vertexPosition;
    PixelInputType2 output;
 

    // Determine the position of the new vertex.
    vertexPosition = barycentricCoordinates.x * patch[0].position + barycentricCoordinates.y * patch[1].position + barycentricCoordinates.z * patch[2].position;
    
	output.texCoord = barycentricCoordinates.x * patch[0].texCoord + 
                      barycentricCoordinates.y * patch[1].texCoord + 
                      barycentricCoordinates.z * patch[2].texCoord;
	
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    /*output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);*/
	output.position = mul( worldViewprojMatrix, float4(vertexPosition, 1.0f) );

    // Send the input color into the pixel shader.
    output.color = patch[0].color;

    return output;
}

Texture2D g_baseTexture : register( t0 );    // Base color texture
SamplerState g_samLinear : register( s0 );

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 color_tessellation_ps(PixelInputType2 input) : SV_TARGET
{
	float4 cBaseColor = g_baseTexture.Sample( g_samLinear, input.texCoord );
    return cBaseColor;
}