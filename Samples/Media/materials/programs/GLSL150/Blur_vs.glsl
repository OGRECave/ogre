#version 150

in vec4 vertex;
out vec2 texCoord[5];

void main()
{
	vec2 inPos = sign(vertex.xy);
	gl_Position = vec4(inPos.xy, 0.0, 1.0);

	vec2 uv = (vec2(inPos.x, -inPos.y) + 1.0)*0.5;
	texCoord[0] = uv;
	
	const float size = 0.01;
	texCoord[1] = uv + vec2(0.0, 1.0)*size;
	texCoord[2] = uv + vec2(0.0, 2.0)*size;
	texCoord[3] = uv + vec2(0.0, -1.0)*size;
	texCoord[4] = uv + vec2(0.0, -2.0)*size;
}
