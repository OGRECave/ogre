#version 330

uniform samplerCube depthTexture;

in block
{
	vec2 uv0;
} inPs;

in vec4 gl_FragCoord;
//out float gl_FragDepth;

#if OUTPUT_TO_COLOUR
	out float fragColour;
#endif

void main()
{
	vec3 cubeDir;

	cubeDir.x = mod( inPs.uv0.x, 0.5 ) * 4.0 - 1.0;
	cubeDir.y = inPs.uv0.y * 2.0 - 1.0;
	cubeDir.z = 0.5 - 0.5 * (cubeDir.x * cubeDir.x + cubeDir.y * cubeDir.y);

	cubeDir.z = inPs.uv0.x < 0.5 ? cubeDir.z : -cubeDir.z;

	float depthValue = textureLod( depthTexture, cubeDir.xyz, 0 ).x;

#if OUTPUT_TO_COLOUR
	fragColour = depthValue;
#else
	gl_FragDepth = depthValue;
#endif
}
