#version 150

in vec4 vertex;
out vec2 uv;

uniform mat4 worldViewProj;
in vec2 uv0;

void main()
{
    gl_Position = worldViewProj * vertex;
    uv = uv0;
}
