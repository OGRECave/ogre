struct VS_INPUT
{
	float4 Position	:	SV_POSITION;
	float3 Normal	:	NORMAL;
	float2 uv0		:	TEXCOORD0;
};

struct PS_INPUT
{
#ifdef DEPTH_SHADOWCASTER
	float3 unused	:	TEXCOORD0;
	float depth		:	TEXCOORD1;
#else
	float2 uv0		:	TEXCOORD0;
	float3 Normal	:	TEXCOORD1;
	float3 vPos		:	TEXCOORD2;
	
	#ifdef DEPTH_SHADOWRECEIVER
		float4 lightSpacePos	:	TEXCOORD3;
	#endif
#endif
};

struct VS_OUTPUT
{
	float4 Position	:	SV_POSITION;
	PS_INPUT	ps;
};

#define SHADOW_BIAS 0

//---------------------------------------------
//Main Vertex Shader
//---------------------------------------------
VS_OUTPUT main_vs( VS_INPUT input,
				   uniform float4x4 viewProjMatrix,
				   uniform float4x4 worldMatrix

#if defined( DEPTH_SHADOWCASTER ) || defined( DEPTH_SHADOWRECEIVER )
				,  uniform float4 depthRange
#endif
#ifdef DEPTH_SHADOWRECEIVER
				,  uniform float4x4 texViewProjMatrix
#endif
				   )
{
	VS_OUTPUT output;
	
	float4 worldPos  = mul( worldMatrix, input.Position );
	float3 worldNorm = mul( (float3x3)(worldMatrix), input.Normal );

	//Transform the position
	output.Position		= mul( viewProjMatrix, worldPos );
	
#ifdef DEPTH_SHADOWCASTER
	output.ps.unused	= float3( 0 );
	output.ps.depth		= (output.Position.z - depthRange.x + SHADOW_BIAS) * depthRange.w;
#else
	output.ps.uv0		= input.uv0;
	
	//Pass Normal and position for Blinn Phong lighting
	output.ps.Normal	= worldNorm;
	output.ps.vPos		= worldPos.xyz;

	#ifdef DEPTH_SHADOWRECEIVER
		// Calculate the position of vertex in light space to do shadows
		output.ps.lightSpacePos = mul( texViewProjMatrix, worldPos );
		// make linear
		output.ps.lightSpacePos.z = (output.ps.lightSpacePos.z - depthRange.x) * depthRange.w;
	#endif
#endif

	return output;
}
