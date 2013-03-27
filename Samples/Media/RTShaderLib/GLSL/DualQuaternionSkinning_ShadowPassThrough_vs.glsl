#version 120

uniform mat4x4 viewProjectionMatrix;
attribute vec4 vertex;

void main()
{
	// view / projection
	gl_Position = viewProjectionMatrix * vertex;
	
	//Depth information
	gl_TexCoord[0].x = gl_Position.z;
	gl_TexCoord[0].y = gl_Position.w;
}

