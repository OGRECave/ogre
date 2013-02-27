/////////////
// GLOBALS //
/////////////

cbuffer MatrixBuffer
{
    matrix worldviewprojMatrix;
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
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