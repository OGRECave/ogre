#version 150

in vec4 vertex;

uniform mat4 WorldViewProj;

void main()
{
    //gl_Position = WorldViewProj * vertex;
    gl_Position = vertex;
}
