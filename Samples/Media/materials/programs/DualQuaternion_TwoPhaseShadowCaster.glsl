#version 120

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, vec4 dualQuaternions[48]);
vec3 calculateBlendPosition(vec3 position, mat2x4 blendDQ);

uniform vec4 worldDualQuaternion2x4Array[48];
uniform mat4x3 scaleM[24];
uniform mat4x4 viewProjectionMatrix;
uniform vec4   ambient;

attribute vec4 vertex;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

void main()
{	
	//First phase - applies scaling and shearing:
	mat4x3 blendS = blendWeights.x*scaleM[int(blendIndices.x)];
	blendS += blendWeights.y*scaleM[int(blendIndices.y)];	
	mat3x4 blendF = transpose(blendS);

	vec3 pass1_position = vertex * blendF;

	//Second phase
	mat2x4 blendDQ = blendTwoWeightsAntipod(blendWeights, blendIndices, worldDualQuaternion2x4Array);

	float len = length(blendDQ[0]);
	blendDQ /= len;

	vec3 blendPosition = calculateBlendPosition(pass1_position, blendDQ);
	
	gl_Position =  viewProjectionMatrix * vec4(blendPosition, 1.0);

	gl_FrontColor = ambient;			
}

