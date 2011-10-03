#version 120

mat2x4 blendTwoWeightsAntipod(vec4 blendWgt, vec4 blendIdx, vec4 dualQuaternions[48]);
vec3 calculateBlendPosition(vec3 position, mat2x4 blendDQ);
vec3 calculateBlendNormal(vec3 normal, mat2x4 blendDQ);

mat3x3 adjointTransposeMatrix(mat3x3 M)
{
	mat3x3 atM;
	atM._m00 = M._m22 * M._m11 - M._m12 * M._m21;
	atM._m01 = M._m12 * M._m20 - M._m10 * M._m22;
	atM._m02 = M._m10 * M._m21 - M._m20 * M._m11;

	atM._m10 = M._m02 * M._m21 - M._m22 * M._m01;
	atM._m11 = M._m22 * M._m00 - M._m02 * M._m20;
	atM._m12 = M._m20 * M._m01 - M._m00 * M._m21;

	atM._m20 = M._m12 * M._m01 - M._m02 * M._m11;
	atM._m21 = M._m10 * M._m02 - M._m12 * M._m00;
	atM._m22 = M._m00 * M._m11 - M._m10 * M._m01;

	return atM;
}

uniform vec4 worldDualQuaternion2x4Array[48];
uniform mat4x3 scaleM[24];
uniform mat4x4 viewProjectionMatrix;
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
	mat4x3 blendS = blendWeights.x*scaleM[int(blendIndices.x)];
	blendS += blendWeights.y*scaleM[int(blendIndices.y)];	
	mat3x4 blendF = transpose(blendS);

	vec3 pass1_position = vertex * blendF;

	mat3x3 blendSrotAT = adjointTransposeMatrix(mat3x3(blendF));
	vec3 pass1_normal = normalize(blendSrotAT * normal);

	//Second phase
	mat2x4 blendDQ = blendTwoWeightsAntipod(blendWeights, blendIndices, worldDualQuaternion2x4Array);

	float len = length(blendDQ[0]);
	blendDQ /= len;

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

