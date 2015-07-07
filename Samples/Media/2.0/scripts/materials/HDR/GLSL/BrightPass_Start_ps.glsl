#version 330

out vec3 fragColour;

in block
{
	vec2 uv0;
} inPs;

uniform sampler2D rt0;
uniform sampler2D lumRt;

vec3 toSRGB( vec3 x )
{
	return sqrt( x );
}

//.x = 'low' threshold. All colour below this value is 0.
//.y = 1 / (low - high). All colour above 'high' is at full brightness.
//Colour between low and high is faded.
uniform vec2 brightThreshold;

void main()
{
	//Perform a high-pass filter on the image
	float fInvLumAvg = texture( lumRt, float( 0.0, 0.0 ) ).x;

	vec3 vSample = texture( rt0, inPs.uv0 ).xyz;
	vSample.xyz *= fInvLumAvg;
	
	vec3 w = clamp( (vSample.xyz - brightThreshold.xxx) * brightThreshold.yyy, 0, 1 );
	vSample.xyz *= w * w * (3.0 - 2.0 * w);

	//We use RGB10 format which doesn't natively support sRGB, so we use a cheap approximation (gamma = 2)
	//This preserves a lot of quality. Don't forget to convert it back when sampling from the texture.
	fragColour = toSRGB( vSample * 0.0625 );
}
