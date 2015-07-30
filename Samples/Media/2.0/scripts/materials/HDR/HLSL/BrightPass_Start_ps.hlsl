//------------------------------------------------------------------
//Bloom
//------------------------------------------------------------------

Texture2D<float3> rt0		: register(t0);
Texture2D<float> lumRt		: register(t1);

SamplerState samplerBilinear: register(s0);
SamplerState samplerPoint	: register(s1);

float3 toSRGB( float3 x )
{
	return sqrt( x );
}

float3 main
(
	in float2 uv : TEXCOORD0,
	
	//.x = 'low' threshold. All colour below this value is 0.
	//.y = 1 / (low - high). All colour above 'high' is at full brightness.
	//Colour between low and high is faded.
	uniform float2 brightThreshold
) : SV_Target
{
	//Perform a high-pass filter on the image
	float fInvLumAvg = lumRt.Sample( samplerPoint, float( 0.0 ).xx ).x;

	float3 vSample = rt0.Sample( samplerBilinear, uv ).xyz;
	vSample.xyz *= fInvLumAvg;
	
	float3 w = saturate( (vSample.xyz - brightThreshold.xxx) * brightThreshold.yyy );
	vSample.xyz *= w * w * (3.0 - 2.0 * w);

	//We use RGB10 format which doesn't natively support sRGB, so we use a cheap approximation (gamma = 2)
	//This preserves a lot of quality. Don't forget to convert it back when sampling from the texture.
	return toSRGB( vSample * 0.0625 );
}
