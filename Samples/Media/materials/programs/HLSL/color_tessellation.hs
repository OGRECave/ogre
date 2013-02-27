/////////////
// GLOBALS //
/////////////
cbuffer TessellationBuffer
{
    float4 tessellationAmounts; //Tessellation factors: x=edge, y=inside, z=MinDistance, w=range
	float4 cameraPosition;
};

//////////////
// TYPEDEFS //
//////////////

//The HullInputType structure is the same as the output structure from the vertex shader.

struct HullInputType
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal 	: TEXCOORD1;
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