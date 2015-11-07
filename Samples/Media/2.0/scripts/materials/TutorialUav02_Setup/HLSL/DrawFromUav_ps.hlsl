Texture2D<float4> testTexture		: register(u1);

float4 main( float4 gl_FragCoord : SV_Position ) : SV_Target
{
	int2 fragPos = int2( gl_FragCoord.xy );
	return testTexture[fragPos];
}
