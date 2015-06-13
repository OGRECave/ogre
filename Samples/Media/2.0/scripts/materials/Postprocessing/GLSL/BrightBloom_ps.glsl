#version 330

//-------------------------------
// High-pass filter for obtaining lumminance
// We use an aproximation formula that is pretty fast:
//   f(x) = ( -3 * ( x - 1 )^2 + 1 ) * 2
//   Color += Grayscale( f(Color) ) + 0.6
//-------------------------------

uniform sampler2D RT;

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

void main()
{
	vec4 tex = texture( RT, inPs.uv0 );
	tex -= 1.0;
	vec4 bright4 = -6.0 * tex * tex + 2.0; //vec4 bright4= ( -3 * tex * tex + 1 ) * 2;
	float bright = dot( bright4.xyz, vec3( 0.333333, 0.333333, 0.333333 ) );
	tex += (bright + 0.6);

    fragColour = tex;
}
