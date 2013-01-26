#version 150

in vec3 oViewPos;
    
out vec4 oColor;
    
uniform float cFarDistance;

void main()
{
    float depth = length(oViewPos) / cFarDistance;
    oColor = vec4(depth, depth, depth, 1.0);
}
