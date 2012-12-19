//---------------------------------------------------------------------------
//These materials/shaders are part of the NEW InstanceManager implementation
//Written by Matias N. Goldberg ("dark_sylinc")
//---------------------------------------------------------------------------
#version 150

//Vertex input
in vec4 vertex;
in vec3 normal;
in vec4 uv0;
in vec4 uv1;
in vec4 uv2;
in vec4 uv3;
in vec3 tangent;

//Parameters
uniform mat4 viewProjMatrix;

#if (DEPTH_SHADOWCASTER || DEPTH_SHADOWRECEIVER)
uniform vec4 depthRange;
#endif

#if DEPTH_SHADOWRECEIVER
uniform mat4 texViewProjMatrix;
#endif

//Output
#if DEPTH_SHADOWCASTER
	out vec2 depth;
#else
	out vec2 _uv0;
	out vec3 oNormal;
	out vec3 oVPos;
	#if DEPTH_SHADOWRECEIVER
		out vec4 oLightSpacePos;
	#endif
#endif

//---------------------------------------------
//Main Vertex Shader
//---------------------------------------------
void main(void)
{
	mat4 worldMatrix;
	worldMatrix[0] = uv1;
	worldMatrix[1] = uv2;
	worldMatrix[2] = uv3;
	worldMatrix[3] = vec4( 0, 0, 0, 1 );

	vec4 worldPos		= vertex * worldMatrix;
	vec3 worldNorm		= normal * mat3(worldMatrix);

	//Transform the position
	gl_Position			= viewProjMatrix * worldPos;
	
#if DEPTH_SHADOWCASTER
	depth.x				= (gl_Position.z - depthRange.x) * depthRange.w;
	depth.y				= depthRange.w;
#else
	_uv0		= uv0.xy;
	oNormal		= worldNorm;
	oVPos		= worldPos.xyz;

	#if DEPTH_SHADOWRECEIVER
		oLightSpacePos		= texViewProjMatrix * worldPos;
		oLightSpacePos.z	= (oLightSpacePos.z - depthRange.x) * depthRange.w;
	#endif
#endif
}
