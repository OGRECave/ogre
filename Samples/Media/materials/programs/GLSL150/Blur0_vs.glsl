#version 150

out vec2 texCoord[5];
in vec4 vertex;

uniform mat4 worldViewProj;
in vec2 uv0;

void main()                    
{
	gl_Position = worldViewProj * vertex;
	
	texCoord[0]  = uv0;
	
	const float size = 0.01;
	texCoord[1] = texCoord[0] + vec2(1.0, 0.0)*size;
	texCoord[2] = texCoord[0] + vec2(2.0, 0.0)*size;
	texCoord[3] = texCoord[0] + vec2(-1.0, 0.0)*size;
	texCoord[4] = texCoord[0] + vec2(-2.0, 0.0)*size;
}
