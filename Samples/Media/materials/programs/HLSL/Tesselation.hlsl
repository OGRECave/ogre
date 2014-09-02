// Sample of DirectX11 Tessellation Tutorial from www.xtunt.com

// This allows us to compile the shader with a #define to choose
// the different partition modes for the hull shader.

// See the hull shader: [partitioning(BEZIER_HS_PARTITION)]
// This sample demonstrates "integer", "fractional_even", and "fractional_odd"

#ifndef HS_PARTITION
#define HS_PARTITION "integer"
#endif

#define INPUT_PATCH_SIZE 3
#define OUTPUT_PATCH_SIZE 3

// Constant Buffers
cbuffer cbPerFrame : register( b0 )
{
    matrix g_mViewProjection;
    float  g_fTessellationFactor;
};

// Vertex shader
struct VS_CONTROL_POINT_INPUT
{
    float4 vPosition        : POSITION;
};

struct VS_CONTROL_POINT_OUTPUT
{
    float4 vPosition        : POSITION;
};

//Just a pass-through shader
VS_CONTROL_POINT_OUTPUT VS( VS_CONTROL_POINT_INPUT Input )
{
    VS_CONTROL_POINT_OUTPUT Output;

    Output.vPosition = Input.vPosition;

    return Output;
}

// Constant data function for the HS.  This is executed once per patch.
struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[3]          : SV_TessFactor;
    float Inside            : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
    float4 vPosition           : SV_POSITION;
};

HS_CONSTANT_DATA_OUTPUT ConstantHS( InputPatch<VS_CONTROL_POINT_OUTPUT, 3> ip,
                                          uint PatchID : SV_PrimitiveID )
{    
    HS_CONSTANT_DATA_OUTPUT Output;

    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = g_fTessellationFactor;
    Output.Inside = g_fTessellationFactor;

    return Output;
}

// The input to the hull shader comes from the vertex shader

// The output from the hull shader will go to the domain shader.
// The tessellation factor, topology, and partition mode will go to the fixed function
// tessellator stage to calculate the UVW and domain points.

[domain("tri")]						//Triangle domain for our shader
[partitioning(HS_PARTITION)]		//Partitioning type according to the GUI
[outputtopology("triangle_cw")]		//Where the generated triangles should face
[outputcontrolpoints(3)]			//Number of times this part of the hull shader will be called for each patch
[patchconstantfunc("ConstantHS")]	//The constant hull shader function
HS_OUTPUT HS( InputPatch<VS_CONTROL_POINT_OUTPUT, 3> p, 
                    uint i : SV_OutputControlPointID,
                    uint PatchID : SV_PrimitiveID )
{
    HS_OUTPUT Output;
    Output.vPosition = p[i].vPosition;
    return Output;
}

// Evaluation domain shader section
struct DS_OUTPUT
{
    float4 vPosition        : SV_POSITION;
};

//Domain Shader is invoked for each vertex created by the Tessellator
[domain("tri")]
DS_OUTPUT DS( HS_CONSTANT_DATA_OUTPUT input, 
                    float3 UVW : SV_DomainLocation,
                    const OutputPatch<HS_OUTPUT, 3> quad )
{
    DS_OUTPUT Output;

	//baricentric interpolation
	float3 finalPos = UVW.x * quad[0].vPosition + UVW.y * quad[1].vPosition + UVW.z * quad[2].vPosition;
    
    Output.vPosition = mul( float4(finalPos,1), g_mViewProjection );

    return Output;    
}

// Pixel shader section
float4 PS( DS_OUTPUT Input ) : SV_TARGET
{
    return float4(1, 0, 0, 1);
}

//  Pixel shader (used for wireframe overlay)
float4 SolidColorPS( DS_OUTPUT Input ) : SV_TARGET
{
    return float4( 1,1,1, 1 );
}