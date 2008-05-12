uniform vec4 worldMatrix3x4Array[240];
uniform mat4 viewProjectionMatrix;
uniform vec4 lightPos;
uniform vec4 ambient;
uniform vec4 lightDiffuseColour;

void main()
{

	// transform by indexed matrix
	// perform matrix multiplication manually since no 3x4 matrices
	vec3 transformedPos;
	vec3 transformedNorm;
	int instanceOffset = int(gl_MultiTexCoord1.x) * 3;
	for (int row = 0; row < 3; ++row)
	{
		vec4 matrixRow = worldMatrix3x4Array[instanceOffset + row];
		transformedPos[row] = dot(matrixRow, gl_Vertex);
#if SHADOW_CASTER
#else
		transformedNorm[row] = dot(matrixRow.xyz, gl_Normal);
#endif
		
	}
	
	// view / projection
	gl_Position = viewProjectionMatrix * vec4(transformedPos,1);
	gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_FrontSecondaryColor = vec4(0);
	
#if SHADOW_CASTER
	gl_FrontColor = ambient;
#else
	vec3 lightDir = normalize(
		lightPos.xyz -  (transformedPos.xyz * lightPos.w));
	gl_FrontColor = ambient + clamp(dot(lightDir, transformedNorm),0.0,1.0) * lightDiffuseColour;
#endif
	
}
