
Texture2D		myTexture : register(t0);
SamplerState	mySampler : register(s0);

float4 main( float2 uv : TEXCOORD0 ) : SV_Target
{
	return myTexture.Sample( mySampler, uv ).xyzw;
}
