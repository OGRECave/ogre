uniform sampler2D rt0;
uniform sampler2D rt1;
uniform sampler2D rt2;
uniform sampler2D rt3;

varying vec2 oUv0;

void main(void)
{
    
    //gl_FragColor = texture2D(rt0, uv);
    gl_FragColor = texture2D(rt1, oUv0);
    //gl_FragColor = texture2D(rt2, uv);
    //gl_FragColor = texture2D(rt3, uv);
	
}

