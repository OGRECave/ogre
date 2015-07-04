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

uniform float fMiddleGray;

//.x = 'low' threshold. All colour below this value is 0
//.y = 'high' threshold. All colour below this value starts to fade to 0.
//.z = 1 / (y - x)
uniform vec3 brightThreshold;

void main()
{
	//Perform a high-pass filter on the image
	float fInvLumAvg = texture( lumRt, float( 0.0 ).xx ).x;

	vec3 vSample = texture( rt0, inPs.uv0 ).xyz;
	vSample.xyz *= fMiddleGray * fInvLumAvg * 1024.0;
	
	vec3 loweredSample = smoothstep( float( 0.0 ).xxx, brightThreshold.yyy,
										(vSample.xyz - brightThreshold.xxx) * brightThreshold.zzz );
										
	vSample.xyz = (vSample.x >= brightThreshold.y ||
				   vSample.z >= brightThreshold.y ||
				   vSample.z >= brightThreshold.y ) ? vSample.xyz : loweredSample.xyz;

	//Filter negative values (extrapolation bellow brightThreshold.x)
	vSample.xyz	 = max( vSample.xyz, 0.0 );

	//We use RGB10 format which doesn't natively support sRGB, so we use a cheap approximation (gamma = 2)
	//This preserves a lot of quality. Don't forget to convert it back when sampling from the texture.
	fragColour = toSRGB( vSample * 0.0625 );
}
