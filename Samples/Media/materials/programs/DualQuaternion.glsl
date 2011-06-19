#version 140

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, mat4x2 dualQuaternions[24]);
vec3 calculateBlendPosition(vec4 position, mat2x4 blendDQ);
vec3 calculateBlendNormal(vec3 normal, mat2x4 blendDQ);

uniform mat4x2 worldDualQuaternion2x4Array[24];
uniform mat4x4 viewProjectionMatrix;
uniform vec4   lightPos[2];
uniform vec4   lightDiffuseColour[2];
uniform vec4   ambient;

in vec4 vertex;
in vec3 normal;
in vec4 blendIndices;
in vec4 blendWeights;

void main()
{	
	mat2x4 blendDQ = blendTwoWeightsAntipod(blendWeights, blendIndices, worldDualQuaternion2x4Array);

	float len = length(blendDQ[0]);
	blendDQ /= len;

	vec3 blendPosition = calculateBlendPosition(vertex, blendDQ);
		
	//No need to normalize, the magnitude of the normal is preserved because only rotation is performed
	vec3 blendNormal = calculateBlendNormal(normal, blendDQ);
	
	gl_Position =  viewProjectionMatrix * vec4(blendPosition, 1.0);
	
	// Lighting - support point and directional
	vec3 lightDir0 = normalize(lightPos[0].xyz - (blendPosition.xyz * lightPos[0].w));
	vec3 lightDir1 = normalize(lightPos[1].xyz - (blendPosition.xyz * lightPos[1].w));

	gl_FrontColor = ambient + (clamp(dot(lightDir0, blendNormal), 0.0, 1.0) * lightDiffuseColour[0]) + 
		(clamp(dot(lightDir1, blendNormal), 0.0, 1.0) * lightDiffuseColour[1]);			
}

