#version 150

in vec4 vertex;
    
out vec3 oViewPos;
   
uniform mat4 cWorldViewProj;
uniform mat4 cWorldView;

void main()
{
    gl_Position = cWorldViewProj * vertex;
    oViewPos = (cWorldView * vertex).xyz;
}
