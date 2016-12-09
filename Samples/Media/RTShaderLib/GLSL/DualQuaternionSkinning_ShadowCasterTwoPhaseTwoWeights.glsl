#version 120

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, vec4 dualQuaternions[48]);
vec3 calculateBlendPosition(vec3 position, mat2x4 blendDQ);
vec3 calculateBlendNormal(vec3 normal, mat2x4 blendDQ);

mat3 adjointTransposeMatrix(mat3 M)
{
	mat3 atM;
	atM[0][0] = M[2][2] * M[1][1] - M[2][1] * M[1][2];
	atM[1][0] = M[2][1] * M[0][2] - M[0][1] * M[2][2];
	atM[2][0] = M[0][1] * M[1][2] - M[0][2] * M[1][1];

	atM[0][1] = M[2][0] * M[1][2] - M[2][2] * M[1][0];
	atM[1][1] = M[2][2] * M[0][0] - M[2][0] * M[0][2];
	atM[2][1] = M[0][2] * M[1][0] - M[0][0] * M[1][2];

	atM[0][2] = M[2][1] * M[1][0] - M[2][0] * M[1][1];
	atM[1][2] = M[0][1] * M[2][0] - M[2][1] * M[0][0];
	atM[2][2] = M[0][0] * M[1][1] - M[0][1] * M[1][0];

	return atM;
}

uniform vec4 worldDualQuaternion2x4Array[48];
uniform vec4 scaleM[72];
uniform mat4 viewProjectionMatrix;
uniform vec4   lightPos[2];
uniform vec4   lightDiffuseColour[2];
uniform vec4   ambient;

attribute vec4 vertex;
attribute vec3 normal;
attribute vec4 blendIndices;
attribute vec4 blendWeights;
attribute vec4 uv0;

varying vec4 colour;

void main()
{	
	//First phase - applies scaling and shearing:
	int blendIndicesX = int(blendIndices.x) * 3;
	int blendIndicesY = int(blendIndices.y) * 3;
	
	mat3x4 blendS = blendWeights.x*mat3x4(scaleM[blendIndicesX], 
		scaleM[blendIndicesX + 1], scaleM[blendIndicesX + 2]);
	
	blendS += blendWeights.y*mat3x4(scaleM[blendIndicesY], scaleM[blendIndicesY + 1], scaleM[blendIndicesY + 2]);

	vec3 pass1_position = vertex* blendS;

	//Second phase
	mat2x4 blendDQ = blendTwoWeightsAntipod(blendWeights, blendIndices, worldDualQuaternion2x4Array);

	blendDQ /= length(blendDQ[0]);

	vec3 blendPosition = calculateBlendPosition(pass1_position, blendDQ);
	
	gl_Position =  viewProjectionMatrix * vec4(blendPosition, 1.0);
	colour = ambient;			
}
