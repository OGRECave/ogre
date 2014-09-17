/**
*	Modified by: Juan Camilo Acosta Arango (ja0335 )
*	Date: 14-04-2013
*	Note: This shaders are based one my study over the 
* 	Eat3D course, "Shader Production - Writing Custom Shaders with CGFX"
*	http://eat3d.com/shaders_intro
**/

/////////////
// GLOBALS //
/////////////
Texture2D g_DiffuseTxt : register( t0 ); // diffuse
SamplerState g_samLinear : register( s0 );

cbuffer cbVertexBuffer
{
    matrix g_WorldViewprojMatrix;
	matrix g_WorldMatrix;
	matrix g_ViewMatrix;
	matrix g_ProjectionMatrix;
};

cbuffer cbTessellationBuffer
{
    float g_tessellationAmount;
};
//////////////
// TYPEDEFS //
//////////////

// application to vertex
struct a2v
{
    float4 position 		: POSITION;
	float2 texCoord  		: TEXCOORD0;
};

// vertex to hull
struct v2h
{
    float4 position 		: POSITION;
	float2 texCoord		   	: TEXCOORD0;
};

// hull to domain
struct h2d
{
    float4 position 		: SV_POSITION;
    float2 texCoord 		: TEXCOORD0;
};

// domain to pixel
struct d2p
{
    float4 position 		: SV_POSITION;
    float2 texCoord     	: TEXCOORD0;
};

struct v2p
{
    float4 position 		: SV_POSITION;
    float2 texCoord     	: TEXCOORD0;
};

struct ConstantOutputType
{
    float edges[3]  : SV_TessFactor;
    float inside 	: SV_InsideTessFactor;
};


//===============================================================
// PER-LIGHT
v2h simple_tessellation_vs(a2v In)
{
	v2h Out;
	//v2p Out;
	
	Out.position 		= In.position;//mul(g_WorldViewprojMatrix, In.position);
	Out.texCoord 		= In.texCoord;
	
	return Out;
}

////////////////////////////////////////////////////////////////////////////////
// Patch Constant Function
////////////////////////////////////////////////////////////////////////////////
ConstantOutputType ColorPatchConstantFunction(InputPatch<v2h, 3> inputPatch, uint patchId : SV_PrimitiveID)
{    
    ConstantOutputType output;
    // Set the tessellation factors for the three edges of the triangle.
    output.edges[0] = g_tessellationAmount;
    output.edges[1] = g_tessellationAmount;
    output.edges[2] = g_tessellationAmount;

    // Set the tessellation factor for tessallating inside the triangle.
    output.inside = g_tessellationAmount;

    return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ColorPatchConstantFunction")]
h2d simple_tessellation_hs(InputPatch<v2h, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    h2d output;
	
    output.position = patch[pointId].position;	
	output.texCoord = patch[pointId].texCoord;

    return output;
}

[domain("tri")]
d2p simple_tessellation_ds(ConstantOutputType input, float3 barycentricCoordinates : SV_DomainLocation, const OutputPatch<h2d, 3> patch)
{
    float3 vertexPosition;
    d2p output;
 

    // Determine the position of the new vertex.
    vertexPosition = barycentricCoordinates.x * patch[0].position.xyz + 
					 barycentricCoordinates.y * patch[1].position.xyz + 
					 barycentricCoordinates.z * patch[2].position.xyz;
    
	
	output.texCoord = barycentricCoordinates.x * patch[0].texCoord + 
                      barycentricCoordinates.y * patch[1].texCoord + 
                      barycentricCoordinates.z * patch[2].texCoord;
	
	output.position = mul(g_WorldViewprojMatrix, float4(vertexPosition, 1.0f));

    return output;
}

float4 simple_tessellation_ps(d2p In) : SV_TARGET
{
	float4 outColor;
	outColor.rgb = g_DiffuseTxt.Sample( g_samLinear, In.texCoord ).rgb;
	outColor.a = 1.0f;
	
	return outColor;
}