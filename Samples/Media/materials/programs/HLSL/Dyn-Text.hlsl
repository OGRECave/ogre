Texture2D g_textures[2];  
SamplerState SampleType
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

cbuffer MatrixBuffer
{
	matrix worldViewProj;
};

// Aplication to Vertex 
struct a2v
{
    float4 position : POSITION;
    float2 texCoord	: TEXCOORD0;
};

// Vertex to Pixel
struct v2p
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

// Vertex Shader
v2p main_vs( a2v input )
{
	v2p output;
	
	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;
	
	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(worldViewProj, input.position);
	output.texCoord = input.texCoord;
	
	return output;
}

// Pixel Shader
float4 main_ps( v2p input ) : SV_Target
{
	float4 l_color0;
	float4 l_color1;
		
	l_color0 = g_textures[0].Sample(SampleType, input.texCoord);
 	
	l_color1 = g_textures[1].Sample(SampleType, input.texCoord);
	l_color1 = float4( l_color1.rrr, 1.0f);
 	
	return l_color0 * l_color1;
	//return l_color1;
	
	/*float4 l_color0;
	l_color0 = g_frostTexture.Sample(SampleType, input.texCoord);
	
	int3 screenspace = int3( input.position.x, input.position.y, 1 );

	float4 vSample = g_thawTexture.Load( screenspace );
	vSample = float4(vSample.rrr, 1.0f);
	
	return l_color0 * vSample;*/
}