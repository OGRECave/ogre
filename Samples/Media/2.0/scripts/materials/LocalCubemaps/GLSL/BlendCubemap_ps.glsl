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

uniform samplerCube cubeTexture0;
uniform samplerCube cubeTexture1;
uniform samplerCube cubeTexture2;
uniform samplerCube cubeTexture3;

uniform vec4 packed3x3Mat[7];
uniform vec4 weights;
uniform vec4 lodLevel;

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
	vec3 vOrigDir = getCubeDir( inPs.uv0.xy * 2.0 - 1.0 );
	vec3 vDir;

	vec4 fAvg;
	mat3 rotMat;

	vDir = vOrigDir;
	fAvg  = textureLod( cubeTexture0, vDir, lodLevel.x ).xyzw * weights.x;

	rotMat = mat3( packed3x3Mat[0].x, packed3x3Mat[0].y, packed3x3Mat[0].z,
				   packed3x3Mat[0].w, packed3x3Mat[1].x, packed3x3Mat[1].y,
				   packed3x3Mat[1].z, packed3x3Mat[1].w, packed3x3Mat[2].x );
	vDir = rotMat * vOrigDir;
	fAvg += textureLod( cubeTexture1, vDir, lodLevel.y ).xyzw * weights.y;

	rotMat = mat3( packed3x3Mat[2].y, packed3x3Mat[2].z, packed3x3Mat[2].w,
				   packed3x3Mat[3].x, packed3x3Mat[3].y, packed3x3Mat[3].z,
				   packed3x3Mat[3].w, packed3x3Mat[4].x, packed3x3Mat[4].y );
	vDir = rotMat * vOrigDir;
	fAvg += textureLod( cubeTexture2, vDir, lodLevel.z ).xyzw * weights.z;

	rotMat = mat3( packed3x3Mat[4].z, packed3x3Mat[4].w, packed3x3Mat[5].x,
				   packed3x3Mat[5].y, packed3x3Mat[5].z, packed3x3Mat[5].w,
				   packed3x3Mat[6].x, packed3x3Mat[6].y, packed3x3Mat[6].z );
	vDir = rotMat * vOrigDir;
	fAvg += textureLod( cubeTexture3, vDir, lodLevel.w ).xyzw * weights.w;

	fragColour = fAvg;
}
