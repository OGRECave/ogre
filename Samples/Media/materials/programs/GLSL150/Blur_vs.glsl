#version 150

in vec4 vertex;
out vec2 texCoord[5];

uniform mat4 worldViewProj;
in vec2 uv0;

void main()
{
	gl_Position = worldViewProj * vertex;

	texCoord[0] = uv0;
	
	const float size = 0.01;
	texCoord[1] = uv0 + vec2(0.0, 1.0)*size;
	texCoord[2] = uv0 + vec2(0.0, 2.0)*size;
	texCoord[3] = uv0 + vec2(0.0, -1.0)*size;
	texCoord[4] = uv0 + vec2(0.0, -2.0)*size;
}
