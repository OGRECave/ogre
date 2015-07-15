
Texture2D<float>	myTexture : register(t0);
SamplerState		mySampler : register(s0);

float main( float2 uv : TEXCOORD0 ) : SV_Target
{
	return myTexture.Sample( mySampler, uv ).x;
}
