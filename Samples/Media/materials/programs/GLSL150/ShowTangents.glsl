#version 150

in vec4 position;
in vec3 tangent;

out vec4 oUv0;

uniform mat4 worldViewProj;

void main()
{
	gl_Position = worldViewProj * position;
	oUv0 = vec4(tangent, 1.0);
}
