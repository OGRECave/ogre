uniform vec4 ambient;
uniform mat4 worldViewProj;

attribute vec4 position;

/*
  Basic ambient lighting vertex program
*/
void main()
{
	gl_Position = worldViewProj * position;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor = ambient;
}


