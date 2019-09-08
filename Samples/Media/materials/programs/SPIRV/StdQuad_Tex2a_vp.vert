#version 430

layout(location = 1) uniform mat4 worldViewProj;

layout(location = 0) in vec4 vertex;
layout(location = 8) in vec2 uv0;

layout(location = 0) out vec2 oUv0;

void main()
{
    // uniforms are not yet passed correctly
    // however here worldViewProj would be identity anyway
    gl_Position = /*worldViewProj */ vertex;
    oUv0 = uv0;
}
