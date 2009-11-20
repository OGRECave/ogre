//-----------------------------------------------------------------------------
// Program Name: SGXLib_IntegratedPSSM
// Program Desc: Integrated PSSM functions.
// Program Type: Vertex/Pixel shader
// Language: HLSL
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void SGX_CopyDepth(in float4 clipSpacePos,
				 out float oDepth)
{
	oDepth = clipSpacePos.z;
}

//-----------------------------------------------------------------------------
void SGX_ModulateScalar(in float vIn0, in float4 vIn1, out float4 vOut)
{
	vOut = vIn0 * vIn1;
}
	
//-----------------------------------------------------------------------------
void SGX_ApplyShadowFactor_Diffuse(in float4 ambient, 
					  in float4 lightSum, 
					  in float fShadowFactor, 
					  out float4 oLight)
{
	oLight.rgb = ambient.rgb + (lightSum.rgb - ambient.rgb) * fShadowFactor;
	oLight.a   = lightSum.a;
}
	
//-----------------------------------------------------------------------------
float _SGX_ShadowPCF4(sampler2D shadowMap, float4 shadowMapPos, float2 offset)
{
	shadowMapPos = shadowMapPos / shadowMapPos.w;
	float2 uv = shadowMapPos.xy;
	float3 o = float3(offset, -offset.x) * 0.3f;

	// Note: We using 2x2 PCF. Good enough and is alot faster.
	float c =	(shadowMapPos.z <= tex2D(shadowMap, uv.xy - o.xy).r) ? 1 : 0; // top left
	c +=		(shadowMapPos.z <= tex2D(shadowMap, uv.xy + o.xy).r) ? 1 : 0; // bottom right
	c +=		(shadowMapPos.z <= tex2D(shadowMap, uv.xy + o.zy).r) ? 1 : 0; // bottom left
	c +=		(shadowMapPos.z <= tex2D(shadowMap, uv.xy - o.zy).r) ? 1 : 0; // top right
		
	return c / 4;
}

//-----------------------------------------------------------------------------
void SGX_ComputeShadowFactor_PSSM3(in float fDepth,
							in float4 vSplitPoints,							
							in float4 lightPosition0,
							in float4 lightPosition1,
							in float4 lightPosition2,
							in sampler2D shadowMap0,
							in sampler2D shadowMap1,
							in sampler2D shadowMap2,
							in float4 invShadowMapSize0,
							in float4 invShadowMapSize1,
							in float4 invShadowMapSize2,																			
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
