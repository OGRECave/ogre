#version 400 core

in vec4 vertex;

//Just a pass-through shader
void main()
{
    gl_Position = vertex;
}
