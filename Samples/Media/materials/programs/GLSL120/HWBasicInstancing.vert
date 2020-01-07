//---------------------------------------------------------------------------
//These materials/shaders are part of the NEW InstanceManager implementation
//Written by Matias N. Goldberg ("dark_sylinc")
//---------------------------------------------------------------------------
#version 120

//Vertex input
attribute vec4 vertex;
attribute vec3 normal;
attribute vec4 uv0;
attribute vec4 uv1;
attribute vec4 uv2;
attribute vec4 uv3;
attribute vec3 tangent;

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
	mat3x4 worldMatrix = mat3x4(uv1, uv2, uv3);
	vec4 worldPos		= vec4(vertex * worldMatrix, 1);
	vec3 worldNorm		= normal * mat3(worldMatrix);

	//Transform the position
	gl_Position			= viewProjMatrix * worldPos;
	
#if DEPTH_SHADOWCASTER
	depth				= gl_Position.zw;
#else
	_uv0		= uv0.xy;
	oNormal		= worldNorm;
	oVPos		= worldPos.xyz;

	#if DEPTH_SHADOWRECEIVER
		oLightSpacePos		= texViewProjMatrix * worldPos;
	#endif
#endif
}
