#version 150

in vec2 depth;
//out vec4 fragColour;

void main()
{
#if LINEAR_RANGE
	float finalDepth = depth.x;
#else
	float finalDepth = depth.x;// / depth.y;
#endif
	// just smear across all components 
	// therefore this one needs high individual channel precision
//	fragColour = vec4(finalDepth, finalDepth, finalDepth, 1);
	gl_FragDepth = finalDepth;
}

