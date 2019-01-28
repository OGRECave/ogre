#version 120
varying vec2 oUv0;

uniform sampler2D mrt1;
	
void main()
{
    gl_FragColor = vec4(texture2D(mrt1, oUv0).rgb / 2.0 + 0.5, 1.0);
}
