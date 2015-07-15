#version 330

//-------------------------------
// Vertical Gaussian-Blur pass
//-------------------------------

uniform sampler2D Blur0;
out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

float offsets[11]  = float[11]
(
	-5.0, -4.0, -3.0, -2.0, -1.0,
	 0.0,
	 1.0,  2.0,  3.0,  4.0,  5.0
);

//We use the Normal-gauss distribution formula
//f(x) being the formula, we used f(0.5)-f(-0.5); f(1.5)-f(0.5)...
float samples[11] = float[11]
(//stddev=2.0
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
);

void main()
{
    vec4 retVal;

	vec2 uv = inPs.uv0.xy;
	uv.y += offsets[0] * 0.01;
	vec4 sum = texture( Blur0, uv ) * samples[0];

	for( int i=1; i<11; ++i )
    {
		uv = inPs.uv0.xy;
		uv.y += offsets[i] * 0.01;
		sum += texture( Blur0, uv ) * samples[i];
    }

    fragColour = sum;
}
