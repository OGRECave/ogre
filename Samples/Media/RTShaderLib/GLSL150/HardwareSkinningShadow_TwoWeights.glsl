#version 150

in vec4 vertex;
in vec4 blendIndices;
in vec4 blendWeights;
out vec4 oColour;

uniform vec4   ambient;
uniform mat4x4 viewProjectionMatrix;
uniform mat4x3  worldMatrix3x4Array[80];

void main()
{
	// output position.
	gl_Position = vertex.xyz * worldMatrix3x4Array[int(blendIndices.x)] * blendWeights.x + vertex.xyz * worldMatrix3x4Array[int(blendIndices.y)] * blendWeights.y;
	gl_Position = viewProjectionMatrix * gl_Position;

	oColour = ambient;
}
