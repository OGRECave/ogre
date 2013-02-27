/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
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