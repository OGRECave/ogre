#version 400 core

uniform float g_fTessellationFactor;

// GLSL tessellation control shader.
layout (vertices = 3) out;
void main()
{
    gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = g_fTessellationFactor;
    gl_TessLevelInner[0] = g_fTessellationFactor;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}

