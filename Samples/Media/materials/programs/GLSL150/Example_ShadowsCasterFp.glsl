
#version 330

#define FRAG_COLOR		0

in float psDepth;
layout(location = FRAG_COLOR, index = 0) out float outColour;

void main()
{
	outColour = psDepth;
}
