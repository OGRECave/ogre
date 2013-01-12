#version 100

precision highp int;
precision highp float;

/*
  Two-weight-per-vertex hardware skinning, shadow caster pass
*/

attribute vec4 vertex;
attribute vec4 uv0;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

// 3x4 matrix, passed as vec4's for compatibility with GL ES 2.0
// Support 24 bones ie 24*3, but use 72 since our parser can pick that out for sizing
uniform vec4 worldMatrix3x4Array[72];
uniform mat4 viewProjectionMatrix;
uniform vec4 ambient;

varying vec4 colour;

void main()
{
	vec3 blendPos = vec3(0.0);

	for (int bone = 0; bone < 2; ++bone)
	{
		// Perform matrix multiplication manually since no 3x4 matrices
	    int idx = int(blendIndices[bone]) * 3;

        // Unroll the loop manually
		mat4 worldMatrix;
		worldMatrix[0] = worldMatrix3x4Array[idx];
		worldMatrix[1] = worldMatrix3x4Array[idx + 1];
		worldMatrix[2] = worldMatrix3x4Array[idx + 2];
		worldMatrix[3] = vec4(0.0);
		// Now weight this into final 
		blendPos += (vertex * worldMatrix).xyz * blendWeights[bone];
	}

	// Apply view / projection to position
	gl_Position = viewProjectionMatrix * vec4(blendPos, 1.0);

	colour = ambient;
}
