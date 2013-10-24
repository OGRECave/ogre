#version 100

precision mediump int;
precision mediump float;

uniform vec4 vertex;
uniform mat4 worldViewProj;
varying vec2 uv;
attribute vec2 uv0;

void main()
{
    gl_Position = worldViewProj * vertex;
    uv = uv0;
}
