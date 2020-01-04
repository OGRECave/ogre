attribute vec4 vertex;
    
varying vec3 oViewPos;
   
uniform mat4 cWorldViewProj;
uniform mat4 cWorldView;

void main()
{
    gl_Position = cWorldViewProj * vertex;
    oViewPos = (cWorldView * vertex).xyz;
}
