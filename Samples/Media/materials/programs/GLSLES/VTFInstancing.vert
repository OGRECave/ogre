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

#ifdef BONE_TWO_WEIGHTS
	in vec4 blendWeights;
#endif

in vec4 uv0;
in vec4 uv1;
in vec4 uv2;
in vec3 tangent;

//Parameters
uniform mat4 viewProjMatrix;
uniform sampler2D matrixTexture;

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

vec3 calculateBlendPosition(vec3 position, mat2x4 blendDQ)
{
	vec3 blendPosition = position + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, position) + blendDQ[0].x*position);
	vec3 trans = 2.0*(blendDQ[0].x*blendDQ[1].yzw - blendDQ[1].x*blendDQ[0].yzw + cross(blendDQ[0].yzw, blendDQ[1].yzw));
	blendPosition += trans;

	return blendPosition;
}

vec3 calculateBlendNormal(vec3 normal, mat2x4 blendDQ)
{
	return normal + 2.0*cross(blendDQ[0].yzw, cross(blendDQ[0].yzw, normal) + blendDQ[0].x*normal);
}

//---------------------------------------------
//Main Vertex Shader
//---------------------------------------------
void main(void)
{
	vec4 worldPos;
	vec3 worldNorm;

#ifdef ST_DUAL_QUATERNION
	mat2x4 blendDQ;	
	blendDQ[0] = texture( matrixTexture, uv1.xy );
	blendDQ[1] = texture( matrixTexture, uv1.zy );
#ifdef BONE_TWO_WEIGHTS
	mat2x4 blendDQ2;
	blendDQ2[0] = texture( matrixTexture, uv2.xy );
	blendDQ2[1] = texture( matrixTexture, uv2.zw );

	//Accurate antipodality handling. For speed increase, remove the following line
	if (dot(blendDQ[0], blendDQ2[0]) < 0.0) blendDQ2 *= -1.0;
	
	//Blend the dual quaternions based on the weights
	blendDQ *= blendWeights.x;
	blendDQ += blendWeights.y*blendDQ2;
	//Normalize the resultant dual quaternion
	blendDQ /= length(blendDQ[0]);
#endif
	worldPos = vec4(calculateBlendPosition(vertex.xyz, blendDQ), 1.0);
	worldNorm = calculateBlendNormal(normal, blendDQ);
#else
	mat4 worldMatrix;
	worldMatrix[0] = texture( matrixTexture, uv1.xy );
	worldMatrix[1] = texture( matrixTexture, uv1.zw );
	worldMatrix[2] = texture( matrixTexture, uv2.xy );
	worldMatrix[3] = vec4( 0, 0, 0, 1 );

	worldPos		= vertex * worldMatrix;
	worldNorm		= normal * mat3(worldMatrix);
#endif

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
