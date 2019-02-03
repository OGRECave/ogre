//-------------------------------
//BlurH_ps20.glsl
// Horizontal Gaussian-Blur pass
//-------------------------------

uniform sampler2D Blur0;
varying vec2 texCoord;

vec2 pos[11];
//We use the Normal-gauss distribution formula
//f(x) being the formula, we used f(0.5)-f(-0.5); f(1.5)-f(0.5)...
float samples[11];

void main()
{
	pos[0] = vec2(-5.0, 0.0);
	pos[1] = vec2(-4.0, 0.0);
	pos[2] = vec2(-3.0, 0.0);
	pos[3] = vec2(-2.0, 0.0);
	pos[4] = vec2(-1.0, 0.0);
	pos[5] = vec2(0.0, 0.0);
	pos[6] = vec2(1.0, 0.0);
	pos[7] = vec2(2.0, 0.0);
	pos[8] = vec2(3.0, 0.0);
	pos[9] = vec2(4.0, 0.0);
	pos[10] = vec2(5.0, 0.0);

	//We use the Normal-gauss distribution formula
	//f(x) being the formula, we used f(0.5)-f(-0.5); f(1.5)-f(0.5)...
	//stddev=2.0
	samples[0] = 0.01222447;
	samples[1] = 0.02783468;
	samples[2] = 0.06559061;
	samples[3] = 0.12097757;
	samples[4] = 0.17466632;
	samples[5] = 0.19741265;
	samples[6] = 0.17466632;
	samples[7] = 0.12097757;
	samples[8] = 0.06559061;
	samples[9] = 0.02783468;
	samples[10] = 0.01222447;
	
    vec4 retVal;
    vec4 sum;
    int i = 0;

    sum = vec4( 0 );
    for( i=0;i < 11; i++ )
    {
        sum += texture2D( Blur0, texCoord + (pos[i] * 0.0100000) ) * samples[i];
    }

    gl_FragColor = sum;
}
