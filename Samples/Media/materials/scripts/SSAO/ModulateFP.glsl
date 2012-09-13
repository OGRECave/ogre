#version 120
varying vec2 uv;

uniform sampler2D scene;
uniform sampler2D occlusion;
	
void main()
{
    gl_FragColor = vec4((texture2D(scene, uv) * texture2D(occlusion, uv)).rgb, 1.0);
}
