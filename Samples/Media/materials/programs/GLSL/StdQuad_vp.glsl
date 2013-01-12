varying vec2 uv;
uniform mat4 worldViewProj;

void main()                    
{
	gl_Position = worldViewProj * gl_Vertex;
	
	vec2 inPos = sign(gl_Vertex.xy);
	
	uv = (vec2(inPos.x, -inPos.y) + 1.0)/2.0;
}

