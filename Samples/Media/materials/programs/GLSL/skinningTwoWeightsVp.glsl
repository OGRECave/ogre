#version 120
// Example GLSL program for skinning with two bone weights per vertex

attribute vec4 vertex;
attribute vec3 normal;
attribute vec4 uv0;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

// ogre <> glsl notation is transposed
uniform mat4x3 worldMatrix3x4Array[24];
uniform mat4 viewProjectionMatrix;
uniform vec4 lightPos[2];
uniform vec4 lightDiffuseColour[2];
uniform vec4 ambient;

void main()
{
	vec3 blendPos = vec3(0.0, 0.0, 0.0);
	vec3 blendNorm = vec3(0.0, 0.0, 0.0);

	for (int bone = 0; bone < 2; ++bone)
	{
        // ATI GLSL compiler can't handle indexing an array within an array so calculate the inner index first
		int idx = int(blendIndices[bone]);

		// now weight this into final
		float weight = blendWeights[bone];
		blendPos += worldMatrix3x4Array[idx]* vertex * weight;

		mat3 worldRotMatrix = mat3(worldMatrix3x4Array[idx]);
		blendNorm += (worldRotMatrix * normal) * weight;
	}

	blendNorm = normalize(blendNorm);

	// apply view / projection to position
	gl_Position = viewProjectionMatrix * vec4(blendPos, 1.0);

	// simple vertex lighting model
	vec3 lightDir0 = normalize(
		lightPos[0].xyz -  (blendPos * lightPos[0].w));
	vec3 lightDir1 = normalize(
		lightPos[1].xyz -  (blendPos * lightPos[1].w));

	gl_FrontColor = gl_FrontMaterial.diffuse * (ambient + (clamp(dot(lightDir0, blendNorm), 0.0, 1.0) * lightDiffuseColour[0]) +
		(clamp(dot(lightDir1, blendNorm), 0.0, 1.0) * lightDiffuseColour[1]));

	gl_TexCoord[0] = uv0;
}
