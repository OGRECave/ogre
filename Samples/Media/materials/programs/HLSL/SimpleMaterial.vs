/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldViewprojMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position 		: POSITION;
    float3 normal 			: NORMAL;
    float2 texCoord     	: TEXCOORD0;
};

struct PixelInputType
{
    float4 position 		: SV_POSITION;
    float2 texCoord			: TEXCOORD0;
    float3 normal  		   	: TEXCOORD1;
};


PixelInputType color_vs(VertexInputType input)
{
	PixelInputType output;
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(worldViewprojMatrix, input.position);
	output.normal = input.normal;
	output.texCoord = input.texCoord;
		
    return output;
}