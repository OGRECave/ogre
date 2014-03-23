
#version 330

#define FRAG_COLOR		0

in oUv0;
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;

uniform sampler2D rt;

void main()
{
    outColour = vec4( texture( rt, oUv0 ).x, 1.0 );
}
