varying vec2 uv;
uniform mat4 worldViewProj;
attribute vec2 uv0;

void main()                    
{
	gl_Position = worldViewProj * gl_Vertex;
	
	uv = uv0;
}

