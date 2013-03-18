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