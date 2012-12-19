varying vec2 texCoord[5];

void main()                    
{
	vec2 inPos = sign(gl_Vertex.xy);
	gl_Position = vec4(inPos.xy, 0.0, 1.0);
	
	texCoord[0]  = (vec2(inPos.x, -inPos.y) + 1.0)/2.0;
	
	const float size = 0.01;
	texCoord[1] = texCoord[0] + vec2(1.0, 0.0)*size;
	texCoord[2] = texCoord[0] + vec2(2.0, 0.0)*size;
	texCoord[3] = texCoord[0] + vec2(-1.0, 0.0)*size;
	texCoord[4] = texCoord[0] + vec2(-2.0, 0.0)*size;
}
