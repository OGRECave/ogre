varying vec2 texCoord[5];
uniform mat4 worldViewProj;
attribute vec2 uv0;

void main()                    
{
	gl_Position = worlViewProj * gl_Vertex;
	
	texCoord[0]  = uv0;
	
	const float size = 0.01;
	texCoord[1] = texCoord[0] + vec2(1.0, 0.0)*size;
	texCoord[2] = texCoord[0] + vec2(2.0, 0.0)*size;
	texCoord[3] = texCoord[0] + vec2(-1.0, 0.0)*size;
	texCoord[4] = texCoord[0] + vec2(-2.0, 0.0)*size;
}
