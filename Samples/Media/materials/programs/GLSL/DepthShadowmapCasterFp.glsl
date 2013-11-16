#version 120

varying vec2 depth;

void main()
{
	float finalDepth = depth.x / depth.y;

	// just smear across all components 
	// therefore this one needs high individual channel precision
	gl_FragColor = vec4(finalDepth, finalDepth, finalDepth, 1.0);
}

