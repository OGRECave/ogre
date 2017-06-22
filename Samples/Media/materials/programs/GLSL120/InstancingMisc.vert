//---------------------------------------------------------------------------
//These materials/shaders are part of the NEW InstanceManager implementation
//Written by Matias N. Goldberg ("dark_sylinc")
//---------------------------------------------------------------------------
#version 120

//Vertex input
attribute vec4 vertex;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec4 uv0;
attribute vec4 blendIndices;
attribute vec4 blendWeights;

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
	varying vec2 depth;
#else
	varying vec2 _uv0;
	varying vec3 oNormal;
	varying vec3 oVPos;
	#if DEPTH_SHADOWRECEIVER
		varying vec4 oLightSpacePos;
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
