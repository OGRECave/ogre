uniform mat4 WorldViewProj;
attribute vec4 vertex;


void main()
{
    // Transform the vertex (ModelViewProj matrix).
    gl_Position = WorldViewProj*vertex;
}
