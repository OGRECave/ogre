#version 150

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

/*
  Basic pass through geometry shader for GLSL.
  Assumes triangle input and output.
*/
void main()
{
    for (int i = 0; i < gl_in.length(); i++)
    {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
