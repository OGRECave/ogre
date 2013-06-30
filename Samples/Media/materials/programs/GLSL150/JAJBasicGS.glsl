#version 150

// Sample trivial GLSL geometry shader.

uniform mat4 WorldViewProj;

layout(triangles) in;
layout(triangle_strip, max_vertices = 6) out;
 
void main(void) {
    for (int i = 0; i < gl_in.length(); ++i) {
        gl_Position = WorldViewProj * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();

    for (int i = 0; i < gl_in.length(); ++i) {
        const vec4 offset = vec4(50,50,50,0);
        gl_Position = WorldViewProj * (gl_in[i].gl_Position + offset);
        EmitVertex();
    }
    EndPrimitive();
}
