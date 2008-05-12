uniform sampler2D inRTT;
uniform vec2 texelSize;

varying vec2 uv;

void main(void)
{
    vec4 accum = vec4(0.0, 0.0, 0.0, 0.0);

    // Get colour from source
    accum += texture2D(inRTT, uv + texelSize * vec2(-1.0, -1.0));
    accum += texture2D(inRTT, uv + texelSize * vec2( 0.0, -1.0));
    accum += texture2D(inRTT, uv + texelSize * vec2( 1.0, -1.0));
    accum += texture2D(inRTT, uv + texelSize * vec2(-1.0,  0.0));
    accum += texture2D(inRTT, uv + texelSize * vec2( 0.0,  0.0));
    accum += texture2D(inRTT, uv + texelSize * vec2( 1.0,  0.0));
    accum += texture2D(inRTT, uv + texelSize * vec2(-1.0,  1.0));
    accum += texture2D(inRTT, uv + texelSize * vec2( 0.0,  1.0));
    accum += texture2D(inRTT, uv + texelSize * vec2( 1.0,  1.0));
    
	// take average of 9 samples
	accum *= 0.1111111111111111;

	gl_FragColor = accum;

}
