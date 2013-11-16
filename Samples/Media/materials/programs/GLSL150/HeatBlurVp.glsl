#version 150

uniform vec4 position;
out vec2 uv;

uniform mat4 worldViewProj;
in vec4 vertex;
in vec2 uv0;

void main()
{
    gl_Position = worldViewProj * vertex;
    uv  = uv0;
}
