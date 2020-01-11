//---------------------------------------------------------------------------
//These materials/shaders are part of the NEW InstanceManager implementation
//Written by Matias N. Goldberg ("dark_sylinc")
//---------------------------------------------------------------------------
#version 300 es
precision mediump int;
precision mediump float;

//Vertex input
in vec4 vertex;
in vec4 blendIndices;
in vec4 blendWeights;

//Parameters
uniform mat4 viewProjMatrix;
//uniform mat4x3 worldMatrix3x4Array[80];
#ifdef ST_DUAL_QUATERNION
uniform vec4 worldDualQuaternion2x4Array[240];
#else
uniform vec4 worldMatrix3x4Array[240]; //240 = 80*3
#endif

//Output
out vec2 depth;

vec3 calculateBlendPosition(vec3 position, mat2x4 blendDQ)
{
	vec3 blendPosition = position + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, position) + blendDQ[0].x*position);
	vec3 trans = 2.0*(blendDQ[0].x*blendDQ[1].yzw - blendDQ[1].x*blendDQ[0].yzw + cross(blendDQ[0].yzw, blendDQ[1].yzw));
	blendPosition += trans;

	return blendPosition;
}

//---------------------------------------------
//Main Vertex Shader
//---------------------------------------------
void main(void)
{
vec4 worldPos;

#ifdef ST_DUAL_QUATERNION
	int idx = int(blendIndices[0]) * 2;
	mat2x4 blendDQ;
	blendDQ[0] = worldDualQuaternion2x4Array[idx];
	blendDQ[1] = worldDualQuaternion2x4Array[idx + 1];
#ifdef BONE_TWO_WEIGHTS
	int idx2 = int(blendIndices[1]) * 2;
	mat2x4 blendDQ2;
 	blendDQ2[0] = worldDualQuaternion2x4Array[idx2];
	blendDQ2[1] = worldDualQuaternion2x4Array[idx2 + 1];

	//Accurate antipodality handling. For speed increase, remove the following line
	if (dot(blendDQ[0], blendDQ2[0]) < 0.0) blendDQ2 *= -1.0;
	
	//Blend the dual quaternions based on the weights
	blendDQ *= blendWeights.x;
	blendDQ += blendWeights.y*blendDQ2;
	//Normalize the resultant dual quaternion
	blendDQ /= length(blendDQ[0]);
#endif
	worldPos = vec4(calculateBlendPosition(vertex.xyz, blendDQ), 1.0);
#else
	mat4 worldMatrix;
	int idx = int(blendIndices[0]) * 3;
	worldMatrix[0] = worldMatrix3x4Array[idx];
	worldMatrix[1] = worldMatrix3x4Array[idx + 1];
	worldMatrix[2] = worldMatrix3x4Array[idx + 2];
	worldMatrix[3] = vec4( 0, 0, 0, 1 );

	worldPos		= vertex * worldMatrix;
#endif

	//Transform the position
	gl_Position			= viewProjMatrix * worldPos;
}
