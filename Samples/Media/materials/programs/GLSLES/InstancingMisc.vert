//---------------------------------------------------------------------------
//These materials/shaders are part of the NEW InstanceManager implementation
//Written by Matias N. Goldberg ("dark_sylinc")
//---------------------------------------------------------------------------
#version 300 es
precision mediump int;
precision mediump float;

//Vertex input
in vec4 vertex;
in vec3 normal;
in vec3 tangent;
in vec4 uv0;
in vec4 blendIndices;
in vec4 blendWeights;

//Parameters
uniform mat4 viewProjMatrix;
uniform mat4 worldMatrix;

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
