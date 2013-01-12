#version 120
varying vec2 uv;

uniform sampler2D mrt1;
	
void main()
{
    gl_FragColor = vec4(texture2D(mrt1, uv).rgb / 2.0 + 0.5, 1.0);
}
