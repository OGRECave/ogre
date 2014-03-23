#version 150

in vec4 vertex;
in vec2 uv0;
uniform mat4 worldViewProj;

out vec2 oUv0;

void main()
{
    gl_Position = worldViewProj * vertex;
    oUv0 = uv0.xy;
}
