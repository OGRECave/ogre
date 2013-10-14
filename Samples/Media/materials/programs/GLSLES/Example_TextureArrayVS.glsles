#version 300 es
precision mediump int;
precision mediump float;

in vec4 uv0;
in vec4 vertex;
out vec4 oUv;

uniform mat4 worldViewProj;

void main(void)
{
	gl_Position = worldViewProj * vertex;
	oUv = uv0;
}
