#version 120

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, mat4x2 dualQuaternions[24]);
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

uniform mat4x2 worldDualQuaternion2x4Array[24];
uniform mat4x3 scaleM[24];
uniform mat4 viewProjectionMatrix;
uniform vec4   lightPos[2];
uniform vec4   lightDiffuseColour[2];
uniform vec4   ambient;

attribute vec4 vertex;
attribute vec3 normal;
attribute vec4 blendIndices;
attribute vec4 blendWeights;
attribute vec4 uv0;

void main()
{	
	//First phase - applies scaling and shearing:
	int blendIndicesX = int(blendIndices.x);
	int blendIndicesY = int(blendIndices.y);
	
	mat4x3 blendF = blendWeights.x*scaleM[blendIndicesX];
	blendF += blendWeights.y*scaleM[blendIndicesY];

	vec3 pass1_position = blendF * vertex;

	mat3x3 blendSrotAT = adjointTransposeMatrix(mat3x3(blendF));
	vec3 pass1_normal = normalize(blendSrotAT * normal);

	//Second phase
	mat2x4 blendDQ = blendTwoWeightsAntipod(blendWeights, blendIndices, worldDualQuaternion2x4Array);

	blendDQ /= length(blendDQ[0]);

	vec3 blendPosition = calculateBlendPosition(pass1_position, blendDQ);

	//No need to normalize, the magnitude of the normal is preserved because only rotation is performed
	vec3 blendNormal = calculateBlendNormal(pass1_normal, blendDQ);
	
	gl_Position =  viewProjectionMatrix * vec4(blendPosition, 1.0);

	// Lighting - support point and directional
	vec3 lightDir0 = normalize(lightPos[0].xyz - (blendPosition * lightPos[0].w));
	vec3 lightDir1 = normalize(lightPos[1].xyz - (blendPosition * lightPos[1].w));

	gl_TexCoord[0] = uv0;

	gl_FrontColor = gl_FrontMaterial.diffuse * (ambient + (clamp(dot(lightDir0, blendNormal), 0.0, 1.0) * lightDiffuseColour[0]) + 
		(clamp(dot(lightDir1, blendNormal), 0.0, 1.0) * lightDiffuseColour[1]));			
}

