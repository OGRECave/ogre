#version 120
varying vec2 uv;

uniform sampler2D mrt2;
	
void main()
{
    gl_FragColor = vec4(texture2D(mrt2, uv).rgb * vec3(0.1, 0.1, -0.01), 1.0);
}
