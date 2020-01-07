//---------------------------------------------------------------------------
//These materials/shaders are part of the NEW InstanceManager implementation
//Written by Matias N. Goldberg ("dark_sylinc")
//---------------------------------------------------------------------------
#version 300 es
precision mediump int;
precision mediump float;

//Vertex input
in vec4 vertex;
in mat3x4 uv1;

//Parameters
uniform mat4 viewProjMatrix;

//---------------------------------------------
//Main Vertex Shader
//---------------------------------------------
void main(void)
{
	mat3x4 worldMatrix = uv1;
	vec3 worldPos		= vertex * worldMatrix;

	//Transform the position
	gl_Position			= viewProjMatrix * vec4(worldPos, 1);
}
