
#version 330

#define FRAG_COLOR		0

in vec2 oUv0;
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;

uniform sampler2D rt;

void main()
{
    outColour = vec4( texture( rt, oUv0 ).xxx, 1.0 );
}
