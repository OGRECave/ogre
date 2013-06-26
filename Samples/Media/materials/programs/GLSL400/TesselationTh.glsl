#version 400 core

// GLSL Control shader

uniform float g_fTessellationFactor;

layout (vertices = 3) out;
void main()
{
    gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = g_fTessellationFactor;
    gl_TessLevelInner[0] = g_fTessellationFactor;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}

