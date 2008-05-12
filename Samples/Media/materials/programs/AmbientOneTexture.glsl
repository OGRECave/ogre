uniform vec4 ambient;

/*
  Basic ambient lighting vertex program
*/
void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor = ambient;
}


