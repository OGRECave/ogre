/////////////
// GLOBALS //
/////////////

cbuffer TessellationBuffer
{
    float4 tessellationAmounts; //Tessellation factors: x=edge, y=inside, z=MinDistance, w=range
	float4 cameraPosition;
};

cbuffer MatrixBuffer
{
    matrix worldviewprojMatrix;
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
	//float4 cameraPosition;
	float  fDisplacementScale;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float3 position 		: POSITION;
    float3 normal 			: NORMAL;
    float2 texCoord     	: TEXCOORD0;
};

struct HullInputType
{
    float3 position 		: POSITION;
    float2 texCoord     	: TEXCOORD0;
    float3 normal 			: TEXCOORD1;
};

//The ConstantOutputType structure is what will be the output from the patch constant function.
struct ConstantOutputType
{
    float edges[3]  : SV_TessFactor;
    float inside 	: SV_InsideTessFactor;
	
	// Geometry cubic generated control points
    float3 f3B210    : POSITION3;
    float3 f3B120    : POSITION4;
    float3 f3B021    : POSITION5;
    float3 f3B012    : POSITION6;
    float3 f3B102    : POSITION7;
    float3 f3B201    : POSITION8;
    float3 f3B111    : CENTER;
    
    // Normal quadratic generated control points
    float3 f3N110    : NORMAL3;      
    float3 f3N011    : NORMAL4;
    float3 f3N101    : NORMAL5;
};

//The HullOutputType structure is what will be the output from the hull shader.
struct HullOutputType
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal 	: TEXCOORD1;
};

//The output of the domain shader goes to the pixel shader. This was previously in the vertex shader.
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal 	: TEXCOORD1;
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
//	float3 cameraPosition;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
HullInputType color_tessellation_vs(VertexInputType input)
{
    HullInputType output;
	
	// Pass the vertex position into the hull shader.
    output.position = input.position;
    output.normal = normalize( mul(input.normal, (float3x3)worldMatrix ) );
	output.texCoord = input.texCoord;
    
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
    
	
	// Now setup the PNTriangle control points...
	// Assign Positions
	float3 f3B003 = inputPatch[0].position;
	float3 f3B030 = inputPatch[1].position;
	float3 f3B300 = inputPatch[2].position;
	// And Normals
	float3 f3N002 = inputPatch[0].normal;
	float3 f3N020 = inputPatch[1].normal;
	float3 f3N200 = inputPatch[2].normal;
	
	// Compute the cubic geometry control points
	// Edge control points
	output.f3B210 = ( ( 2.0f * f3B003 ) + f3B030 - ( dot( ( f3B030 - f3B003 ), f3N002 ) * f3N002 ) ) / 3.0f;
	output.f3B120 = ( ( 2.0f * f3B030 ) + f3B003 - ( dot( ( f3B003 - f3B030 ), f3N020 ) * f3N020 ) ) / 3.0f;
	output.f3B021 = ( ( 2.0f * f3B030 ) + f3B300 - ( dot( ( f3B300 - f3B030 ), f3N020 ) * f3N020 ) ) / 3.0f;
	output.f3B012 = ( ( 2.0f * f3B300 ) + f3B030 - ( dot( ( f3B030 - f3B300 ), f3N200 ) * f3N200 ) ) / 3.0f;
	output.f3B102 = ( ( 2.0f * f3B300 ) + f3B003 - ( dot( ( f3B003 - f3B300 ), f3N200 ) * f3N200 ) ) / 3.0f;
	output.f3B201 = ( ( 2.0f * f3B003 ) + f3B300 - ( dot( ( f3B300 - f3B003 ), f3N002 ) * f3N002 ) ) / 3.0f;
	
	// Center control point
	float3 f3E = ( output.f3B210 + output.f3B120 + output.f3B021 + output.f3B012 + output.f3B102 + output.f3B201 ) / 6.0f;
	float3 f3V = ( f3B003 + f3B030 + f3B300 ) / 3.0f;
	output.f3B111 = f3E + ( ( f3E - f3V ) / 2.0f );
	
	 // Compute the quadratic normal control points, and rotate into world space
	float fV12 = 2.0f * dot( f3B030 - f3B003, f3N002 + f3N020 ) / dot( f3B030 - f3B003, f3B030 - f3B003 );
	output.f3N110 = normalize( f3N002 + f3N020 - fV12 * ( f3B030 - f3B003 ) );
	float fV23 = 2.0f * dot( f3B300 - f3B030, f3N020 + f3N200 ) / dot( f3B300 - f3B030, f3B300 - f3B030 );
	output.f3N011 = normalize( f3N020 + f3N200 - fV23 * ( f3B300 - f3B030 ) );
	float fV31 = 2.0f * dot( f3B003 - f3B300, f3N200 + f3N002 ) / dot( f3B003 - f3B300, f3B003 - f3B300 );
	output.f3N101 = normalize( f3N200 + f3N002 - fV31 * ( f3B003 - f3B300 ) );
	
	// Set the tessellation factor for tessallating inside the triangle.	
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
	output.normal = patch[pointId].normal;
	output.texCoord = patch[pointId].texCoord;

    return output;
}

//The inputs to the domain shader are the outputs from the hull shader and constant function.
////////////////////////////////////////////////////////////////////////////////
// Domain Shader
////////////////////////////////////////////////////////////////////////////////
[domain("tri")]
PixelInputType color_tessellation_ds(ConstantOutputType input, float3 barycentricCoordinates : SV_DomainLocation, const OutputPatch<HullOutputType, 3> patch)
{
    float3 vertexPosition;
    PixelInputType output;
	
	 // The barycentric coordinates
    float fU = barycentricCoordinates.x;
    float fV = barycentricCoordinates.y;
    float fW = barycentricCoordinates.z;

    // Precompute squares and squares * 3 
    float fUU = fU * fU;
    float fVV = fV * fV;
    float fWW = fW * fW;
    float fUU3 = fUU * 3.0f;
    float fVV3 = fVV * 3.0f;
    float fWW3 = fWW * 3.0f;
 

    // Compute position from cubic control points and barycentric coords
    vertexPosition = 	patch[0].position * fWW * fW +
						patch[1].position * fUU * fU +
						patch[2].position * fVV * fV +
						input.f3B210 * fWW3 * fU +
						input.f3B120 * fW * fUU3 +
						input.f3B201 * fWW3 * fV +
						input.f3B021 * fUU3 * fV +
						input.f3B102 * fW * fVV3 +
						input.f3B012 * fU * fVV3 +
						input.f3B111 * 6.0f * fW * fU * fV;
					 
	// Compute normal from quadratic control points and barycentric coords
    float3 f3Normal = 	patch[0].normal * fWW +
                        patch[1].normal * fUU +
                        patch[2].normal * fVV +
                        input.f3N110 * fW * fU +
                        input.f3N011 * fU * fV +
                        input.f3N101 * fW * fV;
					 
	// Normalize the interpolated normal    
    f3Normal = normalize( f3Normal );
    
	output.texCoord = patch[0].texCoord * fW + patch[1].texCoord * fU + patch[2].texCoord * fV;
		
	output.position = mul( worldviewprojMatrix, float4(vertexPosition, 1.0f) );
	output.normal = f3Normal;

    return output;
}

Texture2D g_baseTexture0 : register( t0 );
SamplerState g_samLinear0 : register( s0 );

Texture2D g_baseTexture1 : register( t1 );
SamplerState g_samLinear1 : register( s1 );

////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 color_tessellation_ps(PixelInputType input) : SV_TARGET
{
	float4 color;
	float lightIntensity;

	float4 texColor = g_baseTexture0.Sample( g_samLinear0, input.texCoord );
    
	lightIntensity =  saturate(dot(input.normal, lightDirection));
	
	color = saturate(surfaceDiffuseColour * lightIntensity);
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
    bumpNormal = texBumpMap.x * input.normal + texBumpMap.y * input.normal;
	
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