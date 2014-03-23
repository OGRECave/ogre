
#version 330

in vec4 vertex;
in vec3 normal;
in vec2 uv0;

#ifdef DEPTH_SHADOWCASTER
	out float psDepth;
    uniform float shadowConstantBias;
#else
	out vec2 psUv;
	out vec3 psWorldPos;
	out vec3 psOutNorm;

	out vec4 psLightSpacePos0;

	#ifdef PSSM
		out vec4 psLightSpacePos1;
		out vec4 psLightSpacePos2;
		out float psDepth;

		uniform mat4x4 texViewProjMatrix1;
		uniform mat4x4 texViewProjMatrix2;
		uniform vec4 depthRange1;
		uniform vec4 depthRange2;
	#endif

    uniform mat4x4 texViewProjMatrix0;
    uniform mat4x4 world;
#endif

uniform mat4x4 worldViewProj;
uniform vec4 depthRange0;


void main()
{
	gl_Position	= worldViewProj * vertex;

#ifdef DEPTH_SHADOWCASTER
	//Linear depth
    psDepth	= (gl_Position.z - depthRange0.x + shadowConstantBias) * depthRange0.w;

	//We can't make the depth buffer linear without Z out in the fragment shader;
	//however we can use a cheap approximation ("pseudo linear depth")
	//see http://yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
    gl_Position.z = gl_Position.z * (gl_Position.w * depthRange0.w);
#else
	psWorldPos	= (world * vertex).xyz;
	psOutNorm	= mat3x3(world) * normal;
	psUv		= uv0;

	// Calculate the position of vertex in light space to do shadows
	vec4 shadowWorldPos = vec4( psWorldPos, 1.0f );
	psLightSpacePos0 = texViewProjMatrix0 * shadowWorldPos;
	// Linear depth
    psLightSpacePos0.z = (psLightSpacePos0.z - depthRange0.x) * depthRange0.w;

	#ifdef PSSM
		psLightSpacePos1	= texViewProjMatrix1 * shadowWorldPos;
		psLightSpacePos1.z	= (psLightSpacePos1.z - depthRange1.x) * depthRange1.w;
		psLightSpacePos2	= texViewProjMatrix2 * shadowWorldPos;
		psLightSpacePos2.z	= (psLightSpacePos2.z - depthRange2.x) * depthRange2.w;

		psDepth = gl_Position.z;
	#endif
#endif
}
