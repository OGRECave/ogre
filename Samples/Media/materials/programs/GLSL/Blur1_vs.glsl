varying vec2 texCoord[5];
attribute vec2 uv0;
uniform mat4 worldViewProj;

void main()                    
{
	gl_Position = worldViewProj * gl_Vertex;
	
	texCoord[0]  = uv0;
	
	const float size = 0.01;
	texCoord[1] = texCoord[0] + vec2(0.0, 1.0)*size;
	texCoord[2] = texCoord[0] + vec2(0.0, 2.0)*size;
	texCoord[3] = texCoord[0] + vec2(0.0, -1.0)*size;
	texCoord[4] = texCoord[0] + vec2(0.0, -2.0)*size;
}
