#version 100

precision highp int;
precision highp float;

varying vec2 depth;

void main()
{
#if LINEAR_RANGE
	float finalDepth = depth.x;
#else
	float finalDepth = depth.x / depth.y;
#endif
	// just smear across all components 
	// therefore this one needs high individual channel precision
    gl_FragColor.r = finalDepth;// = vec4(finalDepth, finalDepth, finalDepth, 1.0);
}
