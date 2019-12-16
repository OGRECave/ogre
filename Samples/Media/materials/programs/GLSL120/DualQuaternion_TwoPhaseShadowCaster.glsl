#version 120

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, mat4x2 dualQuaternions[24]);
vec3 calculateBlendPosition(vec3 position, mat2x4 blendDQ);

uniform mat4x2 worldDualQuaternion2x4Array[24];
uniform mat4x3 scaleM[24];
uniform mat4 viewProjectionMatrix;
uniform vec4   ambient;

attribute vec4 vertex;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

varying vec4 colour;

//Shadow caster pass
void main()
{	
	//First phase - applies scaling and shearing:
	int blendIndicesX = int(blendIndices.x);
	int blendIndicesY = int(blendIndices.y);
	
    mat4x3 blendF = blendWeights.x*scaleM[blendIndicesX];
	blendF += blendWeights.y*scaleM[blendIndicesY];

	vec3 pass1_position = blendF * vertex;

	//Second phase
	mat2x4 blendDQ = blendTwoWeightsAntipod(blendWeights, blendIndices, worldDualQuaternion2x4Array);

	blendDQ /= length(blendDQ[0]);

	vec3 blendPosition = calculateBlendPosition(pass1_position, blendDQ);
	
	gl_Position =  viewProjectionMatrix * vec4(blendPosition, 1.0);

	gl_FrontColor = ambient;
}

