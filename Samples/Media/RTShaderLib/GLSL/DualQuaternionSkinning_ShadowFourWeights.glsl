#version 120

mat2x4 blendFourWeightsAntipod(vec4 blendWgt, vec4 blendIdx, vec4 dualQuaternions[48]);
vec3 calculateBlendPosition(vec3 position, mat2x4 blendDQ);

uniform vec4 worldDualQuaternion2x4Array[48];
uniform mat4 viewProjectionMatrix;
uniform vec4   ambient;

attribute vec4 vertex;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

varying vec4 colour;

//Shadow caster pass
void main()
{
	mat2x4 blendDQ = blendFourWeightsAntipod(blendWeights, blendIndices, worldDualQuaternion2x4Array);

	float len = length(blendDQ[0]);
	blendDQ /= len;

	vec3 blendPosition = calculateBlendPosition(vertex.xyz, blendDQ);

	// view / projection
	gl_Position = viewProjectionMatrix * vec4(blendPosition, 1.0);
	
	colour = ambient;
}

