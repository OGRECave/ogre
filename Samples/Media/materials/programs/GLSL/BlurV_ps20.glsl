// Note, this won't work on ATI which is why it's not used
// the issue is with the array initializers
// no card supports GL_3DL_array_objects but it does work on nvidia, not on ATI
//#extension GL_3DL_array_objects : enable

//-------------------------------
//BlurV_ps20.glsl
// Vertical Gaussian-Blur pass
//-------------------------------

uniform sampler2D Blur0;
vec2 pos[11]  = vec2[11](
	vec2(0.0, -5.0),
	vec2(0.0, -4.0),
	vec2(0.0, -3.0),
	vec2(0.0, -2.0),
	vec2(0.0, -1.0),
	vec2(0.0, 0.0),
	vec2(0.0, 1.0),
	vec2(0.0, 2.0),
	vec2(0.0, 3.0),
	vec2(0.0, 4.0),
	vec2(0.0, 5.0)
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
    
    vec4 sum;
    vec2 texcoord = vec2(gl_TexCoord[0]);
    int i = 0;

    sum = vec4( 0 );
    for( ;i < 11; i++ )
    {
        sum += texture2D( Blur0, texcoord + (pos[i] * 0.0100000) ) * samples[i];
    }

    gl_FragColor = sum;
}
