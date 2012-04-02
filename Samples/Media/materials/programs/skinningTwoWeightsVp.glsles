#version 100

precision highp int;
precision highp float;

// Example GLSL ES program for skinning with two bone weights per vertex

attribute vec4 vertex;
attribute vec4 uv0;
attribute vec3 normal;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

varying vec4 colour;
varying vec4 uv;

// 3x4 matrix, passed as vec4's for compatibility with GL ES 2.0
// Support 24 bones ie 24*3, but use 72 since our parser can pick that out for sizing
uniform vec4 worldMatrix3x4Array[72];
uniform mat4 viewProjectionMatrix;
uniform vec4 lightPos[2];
uniform vec4 lightDiffuseColour[2];
uniform vec4 ambient;
uniform vec4 diffuse;

void main()
{
	vec3 blendPos = vec3(0.0);
	vec3 blendNorm = vec3(0.0);
	
	for (int bone = 0; bone < 2; ++bone)
	{
		// perform matrix multiplication manually since no 3x4 matrices
	    int idx = int(blendIndices[bone]) * 3;

		mat4 worldMatrix;
		worldMatrix[0] = worldMatrix3x4Array[idx];
		worldMatrix[1] = worldMatrix3x4Array[idx + 1];
		worldMatrix[2] = worldMatrix3x4Array[idx + 2];
		worldMatrix[3] = vec4(0.0);
		// now weight this into final 
	    float weight = blendWeights[bone];
		blendPos += (vertex * worldMatrix).xyz * weight;

		mat3 worldRotMatrix = mat3(worldMatrix[0].xyz, worldMatrix[1].xyz, worldMatrix[2].xyz);
		blendNorm += (normal * worldRotMatrix) * weight;
	}

	// apply view / projection to position
	gl_Position = viewProjectionMatrix * vec4(blendPos, 1.0);

	// simple vertex lighting model
	vec3 lightDir0 = normalize(
		lightPos[0].xyz -  (blendPos.xyz * lightPos[0].w));
	vec3 lightDir1 = normalize(
		lightPos[1].xyz -  (blendPos.xyz * lightPos[1].w));

  
	colour = diffuse * (ambient
		+ clamp(dot(lightDir0, blendNorm), 0.0, 1.0) * lightDiffuseColour[0]
		+ clamp(dot(lightDir1, blendNorm), 0.0, 1.0) * lightDiffuseColour[1]);
    uv = uv0;
}
