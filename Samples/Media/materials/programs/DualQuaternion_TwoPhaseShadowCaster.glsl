#version 140

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, mat4x2 dualQuaternions[24]);
vec3 calculateBlendPosition(vec4 position, mat2x4 blendDQ);

uniform mat4x2 worldDualQuaternion2x4Array[24];
uniform mat4x3 scaleM[24];
uniform mat4x4 viewProjectionMatrix;
uniform vec4   ambient;

in vec4 vertex;
in vec4 blendIndices;
in vec4 blendWeights;

void main()
{	
	//First phase - applies scaling and shearing:
	mat4x3 blendS = blendWeights.x*scaleM[int(blendIndices.x)];
	blendS += blendWeights.y*scaleM[int(blendIndices.y)];	
	mat3x4 blendF = transpose(blendS);

	vec4 pass1_position = vec4(vertex * blendF, 1.0);

	//Second phase
	mat2x4 blendDQ = blendTwoWeightsAntipod(blendWeights, blendIndices, worldDualQuaternion2x4Array);

	float len = length(blendDQ[0]);
	blendDQ /= len;

	vec3 blendPosition = calculateBlendPosition(pass1_position, blendDQ);
	
	gl_Position =  viewProjectionMatrix * vec4(blendPosition, 1.0);

	gl_FrontColor = ambient;			
}

