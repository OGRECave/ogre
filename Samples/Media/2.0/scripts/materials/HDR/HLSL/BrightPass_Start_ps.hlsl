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
	uniform float fMiddleGray,
	
	//.x = 'low' threshold. All colour below this value is 0
	//.y = 'high' threshold. All colour below this value starts to fade to 0.
	//.z = 1 / (y - x)
	uniform float3 brightThreshold
) : SV_Target
{
	//Perform a high-pass filter on the image
	float fInvLumAvg = lumRt.Sample( samplerPoint, float( 0.0 ).xx ).x;

	float3 vSample = rt0.Sample( samplerBilinear, uv ).xyz;
	vSample.xyz *= fMiddleGray * fInvLumAvg * 1024.0f;
	
	float3 loweredSample = smoothstep( float( 0.0 ).xxx, brightThreshold.yyy,
										(vSample.xyz - brightThreshold.xxx) * brightThreshold.zzz );
										
	vSample.xyz = vSample.xyz >= brightThreshold.yyy ? vSample.xyz : loweredSample.xyz;

	//Filter negative values (extrapolation bellow brightThreshold.x)
	vSample.xyz	 = max( vSample.xyz, 0.0f );

	//We use RGB10 format which doesn't natively support sRGB, so we use a cheap approximation (gamma = 2)
	//This preserves a lot of quality. Don't forget to convert it back when sampling from the texture.
	return toSRGB( vSample * 0.0625 );
}
