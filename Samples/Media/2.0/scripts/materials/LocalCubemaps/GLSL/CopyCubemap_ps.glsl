#version 330

#define FACE_POS_X 0
#define FACE_NEG_X 1
#define FACE_POS_Y 2
#define FACE_NEG_Y 3
#define FACE_POS_Z 4
#define FACE_NEG_Z 5

out vec4 fragColour;

in block
{
	vec2 uv0;
} inPs;

uniform samplerCube cubeTexture;

uniform float lodLevel;

// Get direction from cube texel for a given face. x and y are in the [-1, 1] range.
vec3 getCubeDir( vec2 uv )
{
	float x = uv.x;
	float y = uv.y;

	// Set direction according to face.
	// Note : No need to normalize as we sample in a cubemap
#if FACEIDX == FACE_POS_X
	return vec3( 1.0, -y, -x );
#elif FACEIDX == FACE_NEG_X
	return vec3( -1.0, -y, x );
#elif FACEIDX == FACE_POS_Y
	return vec3( x, 1.0, y );
#elif FACEIDX == FACE_NEG_Y
	return vec3( x, -1.0, -y );
#elif FACEIDX == FACE_POS_Z
	return vec3( x, -y, 1.0 );
#elif FACEIDX == FACE_NEG_Z
	return vec3( -x, -y, -1.0 );
#endif
}

void main()
{
	vec3 vDir = getCubeDir( inPs.uv0.xy * 2.0 - 1.0 );
	fragColour = textureLod( cubeTexture, vDir, lodLevel.x ).xyzw;
}
