uniform sampler2D inRTT;

uniform vec4 sampleOffsets[15];
uniform vec4 sampleWeights[15];

void main(void)
{
	vec2 uv = vec2( gl_TexCoord[0] );
    vec4 accum = vec4(0.0, 0.0, 0.0, 1.0);
	vec2 sampleUV;
    
    for( int i = 0; i < 15; i++ )
    {
        // Sample from adjacent points, 7 each side and central
        sampleUV = uv + sampleOffsets[i].xy;
        accum += sampleWeights[i] * texture2D(inRTT, sampleUV);
    }
    
    gl_FragColor = accum;
	
}
