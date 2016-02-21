#version 150

in vec4 vertex;
in vec4 uv0;

out vec2 texCoord;
uniform mat4 WorldViewProj;

void main()
{
    texCoord = uv0.xy;
    gl_Position = WorldViewProj * vertex;
}
