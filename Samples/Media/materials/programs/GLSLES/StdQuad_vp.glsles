#version 100

precision mediump int;
precision mediump float;

uniform mat4 worldViewProj;
attribute vec4 vertex;
attribute vec2 uv0;
varying vec2 uv;

void main()
{
    gl_Position = worldViewProj * vertex;
  	
	uv = uv0;
}
