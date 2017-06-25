#version 120
// Example GLSL program for skinning with two bone weights per vertex

attribute vec4 vertex;
attribute vec4 uv0;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

uniform mat4x3 worldMatrix3x4Array[24];
uniform mat4 viewProjectionMatrix;
uniform vec4 ambient;

void main()
{
	vec3 blendPos = vec3(0,0,0);

	for (int bone = 0; bone < 2; ++bone)
	{
        // ATI GLSL compiler can't handle indexing an array within an array so calculate the inner index first
		int idx = int(blendIndices[bone]);

		// now weight this into final
		float weight = blendWeights[bone];
		blendPos += worldMatrix3x4Array[idx]* vertex * weight;
	}

	// apply view / projection to position
	gl_Position = viewProjectionMatrix * vec4(blendPos, 1);
	
	gl_FrontSecondaryColor = vec4(0,0,0,0);
	gl_FrontColor = ambient;
	gl_TexCoord[0] = uv0;
}
