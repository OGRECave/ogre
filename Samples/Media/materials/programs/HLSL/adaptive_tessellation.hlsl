/////////////
// GLOBALS //
/////////////
cbuffer cbPerFrame : register( b0 )
{
    matrix worldViewprojMatrix;
};

cbuffer TessellationBuffer
{
    float4 tessellationAmounts; //Tessellation factors: x=edge, y=inside, z=MinDistance, w=range
	float4 cameraPosition;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType2
{
    float3 position 		: POSITION;
    float2 texCoord     	: TEXCOORD0;
    float4 color 			: COLOR;
};
//The HullInputType structure is the same as the output structure from the vertex shader.

struct HullInputType
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color 	: COLOR;
};

//The ConstantOutputType structure is what will be the output from the patch constant function.

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
};

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

//--------------------------------------------------------------------------------------
// Returns a distance adaptive tessellation scale factor (0.0f -> 1.0f) 
//--------------------------------------------------------------------------------------
float GetDistanceAdaptiveScaleFactor(    
                                    float3 f3Eye,           // Position of the camera/eye
                                    float3 f3EdgePosition0, // Position of the first control point of the given patch edge
                                    float3 f3EdgePosition1, // Position of the second control point of the given patch edge
                                    float fMinDistance,     // Minimum distance that maximum tessellation factors should be applied at
                                    float fRange            // Range beyond the minimum distance where tessellation will scale down to the minimum scaling factor    
                                    )
{
    float3 f3MidPoint = ( f3EdgePosition0 + f3EdgePosition1 ) * 0.5f;

    float fDistance = distance( f3MidPoint, f3Eye ) - fMinDistance;
        
    float fScale = 1.0f - saturate( fDistance / fRange );

    return fScale;
}

////////////////////////////////////////////////////////////////////////////////
// Patch Constant Function
////////////////////////////////////////////////////////////////////////////////
ConstantOutputType ColorPatchConstantFunction(InputPatch<HullInputType, 3> inputPatch, uint patchId : SV_PrimitiveID)
{    
    ConstantOutputType output;
	float adaptativeScaleFactor;

    // Set the tessellation factors for the three edges of the triangle.
    /*output.edges[0] = tessellationAmounts.x;
    output.edges[1] = tessellationAmounts.x;
    output.edges[2] = tessellationAmounts.x;*/
	
	output.edges[0] = output.edges[1] = output.edges[2] = tessellationAmounts.x;
	
	adaptativeScaleFactor = GetDistanceAdaptiveScaleFactor( cameraPosition.xyz, inputPatch[2].position, inputPatch[0].position, tessellationAmounts.z, tessellationAmounts.w );
	output.edges[0] = lerp( 1.0f, output.edges[0], adaptativeScaleFactor);
	
	adaptativeScaleFactor = GetDistanceAdaptiveScaleFactor( cameraPosition.xyz, inputPatch[0].position, inputPatch[1].position, tessellationAmounts.z, tessellationAmounts.w );
	output.edges[1] = lerp( 1.0f, output.edges[1], adaptativeScaleFactor);	
	
	adaptativeScaleFactor = GetDistanceAdaptiveScaleFactor( cameraPosition.xyz, inputPatch[1].position, inputPatch[2].position, tessellationAmounts.z, tessellationAmounts.w );
	output.edges[2] = lerp( 1.0f, output.edges[2], adaptativeScaleFactor);
    
	// Set the tessellation factor for tessallating inside the triangle.
    //output.inside = tessellationAmounts.y;
	
	output.inside = (output.edges[0] + output.edges[1] + output.edges[2]) / 3.0f;

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

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////

Texture2D g_baseTexture0 : register( t0 );
SamplerState g_samLinear0 : register( s0 );

Texture2D g_baseTexture1 : register( t1 );
SamplerState g_samLinear1 : register( s1 );

float4 color_tessellation_ps(PixelInputType2 input) : SV_TARGET
{
	float4 color;

	float4 texColor = g_baseTexture0.Sample( g_samLinear0, input.texCoord );
  
	color = saturate(surfaceDiffuseColour);
	color = color * texColor * lightDiffuseColor;
		
	return color;
}

/*
Texture2D shaderTextures[2];
SamplerState SampleType;

float4 color_tessellation_ps(PixelInputType input) : SV_TARGET
{
	float4 texDiffuseColor;
	float4 texBumpMap;
    float3 bumpNormal;
    float3 lightIntensity;
	float3 diffuseContribution;
    float4 color;
	half lightDistance;
	half iluminationLightAttenuation;
	half calculatedLightAttenuation;
	
	texDiffuseColor  = shaderTextures[0].Sample(SampleType, input.texCoord);
	texBumpMap 		 = shaderTextures[1].Sample(SampleType, input.texCoord);
	
	// Expand the range of the normal value from (0, +1) to (-1, +1).
    texBumpMap = (texBumpMap * 2.0f) - 1.0f;
	
	// Calculate the normal from the data in the bump map.
    bumpNormal = texBumpMap.x + texBumpMap.y ;
	
	 // Normalize the resulting bump normal.
    bumpNormal = normalize(bumpNormal);
		
	 // Calculate the amount of light on this pixel based on the bump map normal value.
    lightIntensity = max(dot(bumpNormal, lightDirection), 0);
	
	//Ilumination Light Attenuation
	lightDistance = length( lightPosition.xyz) / lightAttenuation.r;
	iluminationLightAttenuation = lightDistance * lightDistance; // quadratic falloff
	calculatedLightAttenuation = 1.0 - iluminationLightAttenuation;
	
	// Determine the final diffuse color based on the diffuse color and the amount of light intensity.
	diffuseContribution  = (lightIntensity * lightDiffuseColor.rgb * texDiffuseColor.rgb);// * surfaceDiffuseColour.rgb);
	float3 lightContributtion = (diffuseContribution) * calculatedLightAttenuation;
	
     // Combine the final bump light color with the texture color.
	color = float4(lightContributtion, texDiffuseColor.a);
	
    return color;
}*/