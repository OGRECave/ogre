#version 150

uniform mat4 WorldViewProj;

uniform vec4 origColour;
uniform vec4 cloneColour;

out vec4 colour;

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

void main()
{
    // Pass-through!
    for (int i = 0; i < gl_in.length(); i++){
        gl_Position = WorldViewProj * gl_in[i].gl_Position;
        colour = origColour;
        EmitVertex();
    }
    EndPrimitive();

    // New piece of geometry!  We just swizzle the x and y terms.
    for (int i = 0; i < gl_in.length(); i++){
        gl_Position = WorldViewProj * gl_in[i].gl_Position;
        gl_Position.xy = gl_Position.yx;
        colour = cloneColour;
        EmitVertex();
    }
    EndPrimitive();
}
