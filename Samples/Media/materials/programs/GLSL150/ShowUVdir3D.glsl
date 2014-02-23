#version 150

in vec4 oUv0;

out vec4 fragColour;

// Basic fragment program to display 3d uv
void main()
{
    vec3 normal = normalize(oUv0.xyz);
    fragColour = vec4(normal, 1.0);
}
