
//See Hable_John_Uncharted2_HDRLighting.pptx
//See http://filmicgames.com/archives/75
//See https://expf.wordpress.com/2010/05/04/reinhards_tone_mapping_operator/
/*static const float A = 0.15f;
static const float B = 0.50f;
static const float C = 0.10f;
static const float D = 0.20f;
static const float E = 0.02f;
static const float F = 0.30f;
static const float W = 11.2f;*/
static const float A = 0.22f;
static const float B = 0.3f;
static const float C = 0.10f;
static const float D = 0.20f;
static const float E = 0.01f;
static const float F = 0.30f;
static const float W = 11.2f;

float3 FilmicTonemap( float3 x )
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}
float FilmicTonemap( float x )
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

float3 fromSRGB( float3 x )
{
	return x * x;
}

Texture2D<float4> rt0		: register(t0);
Texture2D<float> lumRt		: register(t1);
Texture2D<float3> bloomRt	: register(t2);

SamplerState samplerPoint	: register(s0);
SamplerState samplerBilinear: register(s2);

float4 main
(
	in float2 uv : TEXCOORD0
) : SV_Target
{
	float fInvLumAvg = lumRt.Sample( samplerPoint, float2( 0.0, 0.0 ) ).x;

	float4 vSample = rt0.Sample( samplerPoint, uv );

	vSample.xyz *= fInvLumAvg;
	vSample.xyz	+= fromSRGB( bloomRt.Sample( samplerBilinear, uv ).xyz ) * 16.0;
	vSample.xyz  = FilmicTonemap( vSample.xyz ) / FilmicTonemap( W );
	//vSample.xyz  = vSample.xyz / (1 + vSample.xyz); //Reinhard Simple
	vSample.xyz  = ( vSample.xyz - 0.5f ) * 1.25f + 0.5f + 0.11f;

	return vSample;
}
