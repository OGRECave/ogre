#version 150

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, mat2x4 dualQuaternions[24]);
vec3 calculateBlendPosition(vec3 position, mat2x4 blendDQ);

in vec4 vertex;
in vec4 blendIndices;
in vec4 blendWeights;

out vec4 oColour;
// Support up to 24 bones of float3x4
// vs_1_1 only supports 96 params so more than this is not feasible
uniform mat2x4 worldDualQuaternion2x4Array[24];
uniform mat3x4 scaleM[24];
uniform mat4 viewProjectionMatrix;
uniform vec4 ambient;

//Two-phase skinning shadow caster pass
void main()
{
	//First phase - applies scaling and shearing:
	mat3x4 blendS = blendWeights.x*scaleM[int(blendIndices.x)];
	blendS += blendWeights.y*scaleM[int(blendIndices.y)];
		
	vec3 pass1_position = vertex * blendS;

	//Second phase
	mat2x4 blendDQ = blendTwoWeightsAntipod(blendWeights, blendIndices, worldDualQuaternion2x4Array);
	
	float len = length(blendDQ[0]);
	blendDQ /= len;

	vec3 blendPosition = calculateBlendPosition(pass1_position, blendDQ);

	// view / projection
	gl_Position = viewProjectionMatrix * vec4(blendPosition, 1.0);
	
	oColour = ambient;
}
