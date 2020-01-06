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
#ifdef ST_DUAL_QUATERNION
uniform mat2x4 worldDualQuaternion2x4Array[80];
#else
uniform mat3x4 worldMatrix3x4Array[80];
#endif


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
	int idx = int(blendIndices[0]);
	mat2x4 blendDQ = worldDualQuaternion2x4Array[int(blendIndices[0])];
#ifdef BONE_TWO_WEIGHTS
	int idx2 = int(blendIndices[1]) * 2;
	mat2x4 blendDQ2 = worldDualQuaternion2x4Array[int(blendIndices[1])];

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
	mat3x4 worldMatrix = worldMatrix3x4Array[int(blendIndices[0])];

	worldPos		= vec4(vertex * worldMatrix, 1);
	worldNorm		= normal * mat3(worldMatrix);
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
