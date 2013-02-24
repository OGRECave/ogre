#version 150

in vec2 oDepth;
out vec4 fragColour;
uniform vec4 pssmSplitPoints;

void main()
{
	float finalDepth = oDepth.x / oDepth.y;
	fragColour = vec4(finalDepth, finalDepth, finalDepth, 1.0);
}
