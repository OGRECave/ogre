#version 150

in vec4 oUv0;

out vec4 fragColour;

// Basic fragment program to display 3d uv
void main()
{
	vec3 n = normalize(oUv0.xyz);
	fragColour = vec4(n.x, n.y, n.z, 1.0);
}


