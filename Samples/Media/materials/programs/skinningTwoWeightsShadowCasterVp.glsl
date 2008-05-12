// Example GLSL program for skinning with two bone weights per vertex

attribute vec4 vertex;
attribute vec4 uv0;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

// 3x4 matrix, passed as vec4's for compatibility with GL 2.0
// GL 2.0 supports 3x4 matrices
// Support 24 bones ie 24*3, but use 72 since our parser can pick that out for sizing
uniform vec4 worldMatrix3x4Array[72];
uniform mat4 viewProjectionMatrix;
uniform vec4 ambient;

void main()
{
	vec3 blendPos = vec3(0,0,0);

	for (int bone = 0; bone < 2; ++bone)
	{
		// perform matrix multiplication manually since no 3x4 matrices
        // ATI GLSL compiler can't handle indexing an array within an array so calculate the inner index first
	    int idx = int(blendIndices[bone]) * 3;
        // ATI GLSL compiler can't handle unrolling the loop so do it manually
        // ATI GLSL has better performance when mat4 is used rather than using individual dot product
        // There is a bug in ATI mat4 constructor (Cat 7.2) when indexed uniform array elements are used as vec4 parameter so manually assign
		mat4 worldMatrix;
		worldMatrix[0] = worldMatrix3x4Array[idx];
		worldMatrix[1] = worldMatrix3x4Array[idx + 1];
		worldMatrix[2] = worldMatrix3x4Array[idx + 2];
		worldMatrix[3] = vec4(0);
		// now weight this into final 
		blendPos += (vertex * worldMatrix).xyz * blendWeights[bone];
	}

	// apply view / projection to position
	gl_Position = viewProjectionMatrix * vec4(blendPos, 1);
	
	gl_FrontSecondaryColor = vec4(0,0,0,0);
	gl_FrontColor = ambient;
	gl_TexCoord[0] = uv0;
}
