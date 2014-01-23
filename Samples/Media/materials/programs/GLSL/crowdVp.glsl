uniform mat4 viewProjectionMatrix;
uniform float numBones;
uniform vec4 worldMatrix3x4Array[240];
uniform vec4 lightDiffuseColour;
uniform vec4 ambient;
uniform vec4 lightPos;

attribute vec4 blendIndices;
attribute vec4 blendWeights;


void main()
{
	vec3 blendPos = vec3(0,0,0);
	vec3 blendNorm = vec3(0,0,0);
	
	vec3 tmpPos = vec3(0,0,0);
	vec3 tmpNorm = vec3(0,0,0);


	int instanceOffset = int(gl_MultiTexCoord1.x) * 3 * int(numBones);
	for (int bone = 0; bone < 2; ++bone)
	{
		// perform matrix multiplication manually since no 3x4 matrices
		for (int row = 0; row < 3; ++row)
		{
		    int idx = instanceOffset + int(blendIndices[bone]) * 3 + row;
			vec4 blendMatrixRow = worldMatrix3x4Array[idx];
			tmpPos[row] = dot(blendMatrixRow, gl_Vertex);
#if SHADOW_CASTER
#else
			tmpNorm[row] = dot(blendMatrixRow.xyz, gl_Normal);
#endif
			
		}
		// now weight this into final 
		blendPos += tmpPos * blendWeights[bone];
#if SHADOW_CASTER
#else
		blendNorm += tmpNorm * blendWeights[bone];
#endif
	}

	// apply view / projection to position
	gl_Position = viewProjectionMatrix * vec4(blendPos, 1);

	
#if SHADOW_CASTER
	gl_FrontColor = ambient;
#else
	// simple lighting model
	vec3 lightDir = normalize(
		lightPos.xyz -  (blendPos.xyz * lightPos.w));
	gl_FrontColor = ambient 
		+ clamp(dot(lightDir, blendNorm), 0.0, 1.0) * lightDiffuseColour;
#endif
    gl_FrontSecondaryColor = vec4(0);
	gl_TexCoord[0] = gl_MultiTexCoord0;

	
}

