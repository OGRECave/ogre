RWTexture2D<uint> testTexture			: register(u1);

struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};

float4 main( PS_INPUT inPs, float4 gl_FragCoord : SV_Position ) : SV_Target
{
	int2 fragPos = int2( gl_FragCoord.xy );
	
	uint packedVal = testTexture[fragPos];
	
	float4 fragColour;
	fragColour.x = (packedVal >> 24u);
	fragColour.y = (packedVal >> 16u) & 0xFF;
	fragColour.z = (packedVal >>  8u) & 0xFF;
	fragColour.w = packedVal & 0xFF;
	return fragColour * 0.00392157f; // 1/255
}
