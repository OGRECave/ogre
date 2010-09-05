//------------------------------------------------------------------------------------------
//Copyright (C) 2010 Matias N. Goldberg (dark_sylinc)
//------------------------------------------------------------------------------------------
//#version 120
attribute vec4 vertex;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec4 uv0;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

//uniform mat4x3 worldMatrix3x4Array[80];
uniform vec4 worldMatrix3x4Array[240]; //240 = 80*3

uniform mat4 viewProjMatrix;

//---------------------------------------------
//Main Vertex Shader
//---------------------------------------------
void main(void)
{
	vec4 worldPos = vec4( 0 );

	mat4 worldMatrix;
	int idx = int(blendIndices[0]) * 3;
	worldMatrix[0] = worldMatrix3x4Array[idx];
	worldMatrix[1] = worldMatrix3x4Array[idx + 1];
	worldMatrix[2] = worldMatrix3x4Array[idx + 2];
	worldMatrix[3] = vec4( 0, 0, 0, 1 );
	
	worldPos = vertex * worldMatrix;
	/*int idx = int(blendIndices[0]);
	worldPos	= vec4( (worldMatrix3x4Array[idx]* vertex).xyz, 1.0 );*/

	//Transform the position
	gl_Position		= viewProjMatrix * worldPos;
	gl_TexCoord[0]	= uv0;
}
