//-----------------------------------------------------------------------------
// Program Name: SGXLib_IntegratedPSSM
// Program Desc: Integrated PSSM functions.
// Program Type: Vertex/Pixel shader
// Language: GLSL
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void SGX_CopyDepth(in vec4 clipSpacePos,
			 out float oDepth)
{
	oDepth = clipSpacePos.z;
}

//-----------------------------------------------------------------------------
void SGX_ModulateScalar(in float vIn0, in vec4 vIn1, out vec4 vOut)
{
	vOut = vIn0 * vIn1;
}
	
//-----------------------------------------------------------------------------
void SGX_ApplyShadowFactor_Diffuse(in vec4 ambient, 
					  in vec4 lightSum, 
					  in float fShadowFactor, 
					  out vec4 oLight)
{
	oLight.rgb = ambient.rgb + (lightSum.rgb - ambient.rgb) * fShadowFactor;
	oLight.a   = lightSum.a;
}
	
//-----------------------------------------------------------------------------
float _SGX_ShadowPCF4(sampler2D shadowMap, vec4 shadowMapPos, vec2 offset)
{
	shadowMapPos = shadowMapPos / shadowMapPos.w;
	vec2 uv = shadowMapPos.xy;
	vec3 o = vec3(offset, -offset.x) * 0.3;

	// Note: We using 2x2 PCF. Good enough and is alot faster.
	float c =	(shadowMapPos.z <= texture2D(shadowMap, uv.xy - o.xy).r) ? 1.0 : 0.0; // top left
	c +=		(shadowMapPos.z <= texture2D(shadowMap, uv.xy + o.xy).r) ? 1.0 : 0.0; // bottom right
	c +=		(shadowMapPos.z <= texture2D(shadowMap, uv.xy + o.zy).r) ? 1.0 : 0.0; // bottom left
	c +=		(shadowMapPos.z <= texture2D(shadowMap, uv.xy - o.zy).r) ? 1.0 : 0.0; // top right
		
	return c / 4.0;
}

//-----------------------------------------------------------------------------
void SGX_ComputeShadowFactor_PSSM3(in float fDepth,
							in vec4 vSplitPoints,	
							in vec4 lightPosition0,
							in vec4 lightPosition1,
							in vec4 lightPosition2,
							in sampler2D shadowMap0,
							in sampler2D shadowMap1,
							in sampler2D shadowMap2,
							in vec4 invShadowMapSize0,
							in vec4 invShadowMapSize1,
							in vec4 invShadowMapSize2,																			
							out float oShadowFactor)
{
	float shadowFactor0;
	float shadowFactor1;
	float shadowFactor2;
														
	shadowFactor0 = _SGX_ShadowPCF4(shadowMap0, lightPosition0, invShadowMapSize0.xy);																								
	shadowFactor1 = _SGX_ShadowPCF4(shadowMap1, lightPosition1, invShadowMapSize1.xy);												
	shadowFactor2 = _SGX_ShadowPCF4(shadowMap2, lightPosition2, invShadowMapSize2.xy);							
		
	if (fDepth  <= vSplitPoints.x)
	{									
		oShadowFactor = shadowFactor0;				
	}
	else if (fDepth <= vSplitPoints.y)
	{									
		oShadowFactor = shadowFactor1;		
	}
	else
	{										
		oShadowFactor = shadowFactor2;				
	}
}

