#version 330

out vec3 fragColour;

in block
{
	vec2 uv0;
} inPs;

uniform sampler2D Blur0;

vec3 fromSRGB( vec3 x )
{
	return x * x;
}
vec3 toSRGB( vec3 x )
{
	return sqrt( x );
}

uniform vec4 invTex0Size;

#define NUM_SAMPLES 11

void main()
{
	vec2 uv = inPs.uv0.xy;

	uv.y -= invTex0Size.y * ((NUM_SAMPLES-1) * 0.5);
	vec3 sum = fromSRGB( texture( Blur0, uv ).xyz );

	for( int i=1; i<NUM_SAMPLES; ++i )
	{
		uv.y += invTex0Size.y;
		sum += fromSRGB( texture( Blur0, uv ).xyz );
	}

	fragColour = toSRGB( sum / NUM_SAMPLES );
}
