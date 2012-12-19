#version 150

in vec4 vertex;
uniform mat4 worldViewProj;

out vec2 uv;

void main()                    
{
	gl_Position = worldViewProj * vertex;
	
	vec2 inPos = sign(vertex.xy);
	
	uv = (vec2(inPos.x, -inPos.y) + 1.0) * 0.5;
}

