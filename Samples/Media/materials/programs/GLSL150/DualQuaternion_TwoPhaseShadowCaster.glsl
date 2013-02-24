#version 150

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, vec4 dualQuaternions[24]);
vec3 calculateBlendPosition(vec3 position, mat2x4 blendDQ);

uniform vec4 worldDualQuaternion2x4Array[24];
uniform vec4 scaleM[72];
uniform mat4 viewProjectionMatrix;
uniform vec4   ambient;

in vec4 vertex;
in vec4 blendIndices;
in vec4 blendWeights;

out vec4 ambientColour;

void main()
{	
	//First phase - applies scaling and shearing:
	int blendIndicesX = int(blendIndices.x) * 3;
	int blendIndicesY = int(blendIndices.y) * 3;
	
	mat3x4 blendS = blendWeights.x*mat3x4(scaleM[blendIndicesX], 
		scaleM[blendIndicesX + 1], scaleM[blendIndicesX + 2]);
	
	blendS += blendWeights.y*mat3x4(scaleM[blendIndicesY],
	    scaleM[blendIndicesY + 1], scaleM[blendIndicesY + 2]);

	mat4x3 blendF = transpose(blendS);

	vec3 pass1_position = blendF * vertex;

	//Second phase
	mat2x4 blendDQ = blendTwoWeightsAntipod(blendWeights, blendIndices, worldDualQuaternion2x4Array);

	blendDQ /= length(blendDQ[0]);

	vec3 blendPosition = calculateBlendPosition(pass1_position, blendDQ);
	
	gl_Position =  viewProjectionMatrix * vec4(blendPosition, 1.0);

	ambientColour = ambient;			
}

