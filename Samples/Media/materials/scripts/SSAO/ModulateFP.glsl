#version 120
varying vec2 uv;

uniform sampler2D ssao;
uniform sampler2D scene;
	
void main()
{
    gl_FragColor = vec4((texture2D(scene, uv) * texture2D(ssao, uv)).rgb, 1.0);
}
