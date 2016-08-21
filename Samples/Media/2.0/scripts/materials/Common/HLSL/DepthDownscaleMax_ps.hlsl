struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;
};

Texture2D<float> depthTexture	: register(t0);

float main
(
	PS_INPUT inPs,
	float4 gl_FragCoord : SV_Position
) : SV_Depth
{
	float fDepth0 = depthTexture.Load( int3( int2(gl_FragCoord.xy * 2.0), 0 ) ).x;
	float fDepth1 = depthTexture.Load( int3( int2(gl_FragCoord.xy * 2.0) + int2( 0, 1 ), 0 ) ).x;
	float fDepth2 = depthTexture.Load( int3( int2(gl_FragCoord.xy * 2.0) + int2( 1, 0 ), 0 ) ).x;
	float fDepth3 = depthTexture.Load( int3( int2(gl_FragCoord.xy * 2.0) + int2( 1, 1 ), 0 ) ).x;
	
	//return depthTexture.Load( int3( int2(gl_FragCoord.xy * 2.0), 0 ) ).x;
	return max( max( fDepth0, fDepth1 ), max( fDepth2, fDepth3 ) );
}
