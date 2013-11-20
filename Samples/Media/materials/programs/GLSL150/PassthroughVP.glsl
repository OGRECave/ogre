#version 150

in vec4 vertex;

void main()
{
    // Transform the vertex (ModelViewProj matrix).
    gl_Position = vertex;
}
