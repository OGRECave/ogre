varying vec2 texCoord;
uniform mat4 worldViewProj;

attribute vec4 vertex;
attribute vec2 uv0;

void main()                    
{
	gl_Position = worldViewProj * vertex;
	texCoord = uv0;
}
