#version 100

precision mediump int;
precision mediump float;

attribute vec4 uv0;
attribute vec4 vertex;
varying vec4 oUv;

uniform mat4 worldViewProj;

void main(void)
{
	gl_Position = worldViewProj * vertex;
	oUv = uv0;
}
