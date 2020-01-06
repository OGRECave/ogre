//---------------------------------------------------------------------------
//These materials/shaders are part of the NEW InstanceManager implementation
//Written by Matias N. Goldberg ("dark_sylinc")
//---------------------------------------------------------------------------
#version 120

//Vertex input
attribute vec4 vertex;
attribute vec3 normal;

#ifdef BONE_TWO_WEIGHTS
	attribute vec4 blendWeights;
#endif

attribute vec4 uv0;
attribute vec4 uv1;
attribute vec4 uv2;
	
#if BONE_MATRIX_LUT
	attribute vec4 uv3;
	attribute vec4 uv4;
	attribute vec4 uv5;
#endif

attribute vec3 tangent;

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
	varying vec2 depth;
#else
	varying vec2 _uv0;
	varying vec3 oNormal;
	varying vec3 oVPos;
	#if DEPTH_SHADOWRECEIVER
		varying vec4 oLightSpacePos;
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
	blendDQ[0] = texture2D( matrixTexture, vec2(uv1.x, 0.0) + uv2.xy );
	blendDQ[1] = texture2D( matrixTexture, vec2(uv1.y, 0.0) + uv2.xy );
#ifdef BONE_TWO_WEIGHTS
	mat2x4 blendDQ2;
	blendDQ2[0] = texture2D( matrixTexture, vec2(uv1.z, 0.0) + uv2.xy );
	blendDQ2[1] = texture2D( matrixTexture, vec2(uv1.w, 0.0) + uv2.xy );

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
	mat3x4 worldMatrix;
	worldMatrix[0] = texture2D( matrixTexture, uv1.xw + uv2.xy );
	worldMatrix[1] = texture2D( matrixTexture, uv1.yw + uv2.xy );
	worldMatrix[2] = texture2D( matrixTexture, uv1.zw + uv2.xy );

	worldPos		= vec4(vertex * worldMatrix, 1);
	worldNorm		= normal * mat3(worldMatrix);
#endif

#if BONE_MATRIX_LUT
	mat4 worldCompMatrix;
	worldCompMatrix[0] = uv3;
	worldCompMatrix[1] = uv4;
	worldCompMatrix[2] = uv5;
	worldCompMatrix[3] = vec4( 0, 0, 0, 1 );
	
	worldPos =  worldPos * worldCompMatrix;
	worldNorm = worldNorm * mat3(worldCompMatrix);
#endif

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
