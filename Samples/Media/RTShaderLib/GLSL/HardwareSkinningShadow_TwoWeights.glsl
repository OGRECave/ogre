#version 120

attribute vec4 vertex;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

uniform vec4   ambient;
uniform mat4x4 viewProjectionMatrix;
uniform mat4x3  worldMatrix3x4Array[80];

varying vec4 colour;

void main()
{
	// output position.
	gl_Position = vertex.xyz * worldMatrix3x4Array[int(blendIndices.x)] * blendWeights.x + vertex.xyz * worldMatrix3x4Array[int(blendIndices.y)] * blendWeights.y;
	gl_Position = viewProjectionMatrix * gl_Position;

	colour = ambient;
}
