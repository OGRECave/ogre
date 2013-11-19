#version 150

in vec4 vertex;

// uniform mat4 WorldViewProj;

void main()
{
    // Transform the vertex (ModelViewProj matrix).
    // gl_Position = ftransform();
    // gl_Position = WorldViewProj * vertex;
    gl_Position = vertex;
}
