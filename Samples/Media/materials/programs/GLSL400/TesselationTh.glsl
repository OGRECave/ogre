#version 400 core

#extension GL_ARB_tessellation_shader: enable

// Sample of DirectX11 Tessellation Tutorial from www.xtunt.com

// This allows us to compile the shader with a #define to choose
// the different partition modes for the hull shader.

// See the hull shader: [partitioning(BEZIER_HS_PARTITION)]
// This sample demonstrates "integer", "fractional_even", and "fractional_odd"

// GLSL Control shader

uniform float g_fTessellationFactor;
                
layout (vertices = 3) out;
void main()
{
    gl_TessLevelOuter[0] = gl_TessLevelOuter[1] = gl_TessLevelOuter[2] = g_fTessellationFactor;
    gl_TessLevelInner[0] = g_fTessellationFactor;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}

