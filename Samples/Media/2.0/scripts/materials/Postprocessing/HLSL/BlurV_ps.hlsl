//-------------------------------
// Vertical Gaussian-Blur pass
//-------------------------------

Texture2D Blur0				: register(t0);
SamplerState samplerState	: register(s0);

// Simple blur filter

//We use the Normal-gauss distribution formula
//f(x) being the formula, we used f(0.5)-f(-0.5); f(1.5)-f(0.5)...
static const float samples[11] =
{//stddev=2.0
	0.01222447,
	0.02783468,
	0.06559061,
	0.12097757,
	0.17466632,

	0.19741265,

	0.17466632,
	0.12097757,
	0.06559061,
	0.02783468,
	0.01222447
};

static const float offsets[11] =
{
	-5.0, -4.0, -3.0, -2.0, -1.0,
	 0.0,
	 1.0,  2.0,  3.0,  4.0,  5.0
};


struct PS_INPUT
{
	float2 uv0 : TEXCOORD0;
};


float4 main( PS_INPUT inPs ) : SV_Target
{
	float2 uv = inPs.uv0.xy;
	uv.y += offsets[0] * 0.01;

	float4 sum = Blur0.Sample( samplerState, uv ) * samples[0];

	for( int i=1; i<11; ++i)
	{
		uv = inPs.uv0.xy;
		uv.y += offsets[i] * 0.01;
		sum += Blur0.Sample( samplerState, uv ) * samples[i];
	}

	return sum;
}
