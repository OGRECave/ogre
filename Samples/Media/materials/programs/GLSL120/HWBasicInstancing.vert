//---------------------------------------------------------------------------
//These materials/shaders are part of the NEW InstanceManager implementation
//Written by Matias N. Goldberg ("dark_sylinc")
//---------------------------------------------------------------------------
#version 120

//Vertex input
attribute vec4 vertex;
attribute mat3x4 uv1;

//Parameters
uniform mat4 viewProjMatrix;

//---------------------------------------------
//Main Vertex Shader
//---------------------------------------------
void main(void)
{
	mat3x4 worldMatrix = uv1;
	vec4 worldPos		= vec4(vertex * worldMatrix, 1);

	//Transform the position
	gl_Position			= viewProjMatrix * worldPos;
}
