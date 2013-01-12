

//-----------------------------------------------------------------------------
// Program Name: SL_Lighting
// Program Desc: Per pixel lighting functions.
// Program Type: Vertex/Pixel shader
// Language: HLSL
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void SL_TransformNormal(in float4x4 m, 
				   in float3 v, 
				   out float3 vOut)
{
	vOut = mul((float3x3)m, v);
}

//-----------------------------------------------------------------------------
void SL_TransformPosition(in float4x4 mWorldView, 
				   in float4 vPos, 
				   out float3 vOut)
{
	vOut = mul(mWorldView, vPos).xyz;
}

//-----------------------------------------------------------------------------
void SL_Light_Directional_Diffuse(
				   in float3 vNormal,
				   in float3 vNegLightDirView,
				   in float3 vDiffuseColour, 
				   in float3 vBaseColour, 
				   out float3 vOut)
{
	float3 vNormalView = normalize(vNormal);
	float nDotL = dot(vNormalView, vNegLightDirView);
	
	vOut = vBaseColour + vDiffuseColour * saturate(nDotL);
}

//-----------------------------------------------------------------------------
void SL_Light_Directional_DiffuseSpecular(
					in float3 vNormal,
					in float3 vViewPos,					
					in float3 vNegLightDirView,
					in float3 vDiffuseColour, 
					in float3 vSpecularColour, 
					in float fSpecularPower, 
					in float3 vBaseDiffuseColour,
					in float3 vBaseSpecularColour,					
					out float3 vOutDiffuse,
					out float3 vOutSpecular)
{
	vOutDiffuse  = vBaseDiffuseColour;
	vOutSpecular = vBaseSpecularColour;
	
	float3 vNormalView = normalize(vNormal);		
	float nDotL		   = dot(vNormalView, vNegLightDirView);			
	float3 vView       = -normalize(vViewPos);
	float3 vHalfWay    = normalize(vView + vNegLightDirView);
	float nDotH        = dot(vNormalView, vHalfWay);
	
	nDotL = max(nDotL, 0);
	vOutDiffuse  += vDiffuseColour * nDotL;		
	vOutSpecular += vSpecularColour * pow(saturate(nDotH), fSpecularPower);						
}


//the amount of light taken for ambient light (does not realy on direction)
//const float spotAmbientPart = 1;


//-----------------------------------------------------------------------------
void SL_Light_Ambient_Diffuse_Inner(
				    in float3 vNormal,
				    in float3 vLightView,
				    in float fLightDist,
				    in float3 vNegLightDirView,
				    in float3 vSpotParams,
				    in float3 vDiffuseColour, 
				    inout float3 vColorOut)
{
	float fLightDistInv = 1 / fLightDist;
	float nDotL = dot(vNormal, vLightView) * fLightDistInv;
	
	float fAtten = (1 - (fLightDist * vSpotParams.x));
	fAtten = fAtten * fAtten;

	float rho = dot(vNegLightDirView, vLightView) * fLightDistInv;
	float fSpotT = saturate((rho - vSpotParams.y) * vSpotParams.z);

	nDotL = step(0,nDotL) * (0.7 + (0.3 * nDotL));
	vColorOut += vDiffuseColour * 2* nDotL * fAtten * fSpotT;
	
	//if ((fLightDist < 150) && (rho < 0.85))
	//	vColorOut.x = 0.5;
}


//-----------------------------------------------------------------------------
void SL_Light_Ambient_Diffuse(
				    in float3 vNormal,
				    in float3 vViewPos,
				    in float3 vLightPosView,
				    in float3 vNegLightDirView,
				    in float3 vSpotParams,
				    in float3 vDiffuseColour, 
				    inout float3 vColorOut)
{
	float3 vLightView = vLightPosView - vViewPos;
	float fLightDist = length(vLightView);
	if (fLightDist * vSpotParams.x < 1)
	{
		SL_Light_Ambient_Diffuse_Inner(vNormal, vLightView, fLightDist,
				vNegLightDirView, vSpotParams, vDiffuseColour, vColorOut);
	}
}

//-----------------------------------------------------------------------------

void SL_Light_Segment_Texture_Ambient_Diffuse(
				    in float3 vNormal,
				    in float3 vViewPos,
				    in sampler2D dataTexture,
				    in float2 lightIndexLimit,
					in float4 lightBounds,
					in float invWidth,
					in float invHeight,
				    inout float3 vColorOut)
{
	float widthOffset = invWidth * 0.5;		
	float heightOffset = invHeight * 0.5;	
	
	float2 indexes = (vViewPos.xz - lightBounds.xy) * lightBounds.zw;
	indexes = clamp(indexes,0,8);
	int index = (int)indexes.x + (int)(indexes.y) * 9;
	widthOffset += invWidth * 3 * index;
	
	float4 indexBounds = tex2Dlod(dataTexture, float4(widthOffset,heightOffset,0,0));
	int toIndex = min(lightIndexLimit.y, indexBounds.x);
	for(int i = lightIndexLimit.x; i <= toIndex; ++i)
	{
		float heightCoord = heightOffset + invHeight * i;
		float4 dat1 = tex2Dlod(dataTexture, float4(widthOffset,heightCoord,0,0));
		
		float3 vLightView  = dat1.xyz - vViewPos;
		float fLightDist = length(vLightView);
		if (fLightDist * dat1.w < 1)
		{
			float4 dat2 = tex2Dlod(dataTexture, float4(widthOffset + invWidth,heightCoord,0,0));
			float4 dat3 = tex2Dlod(dataTexture, float4(widthOffset + invWidth * 2,heightCoord,0,0));
			SL_Light_Ambient_Diffuse_Inner(vNormal, vLightView, fLightDist, dat2.xyz, float3(dat1.w, dat2.w, dat3.w), dat3.xyz, vColorOut);
		}
	}
}


void SL_Light_Segment_Debug(
				    in float3 vNormal,
				    in float3 vViewPos,
				    in sampler2D dataTexture,
				    in float2 lightIndexLimit,
					in float4 lightBounds,
					in float invWidth,
					in float invHeight,
				    inout float3 vColorOut)
{
	float widthOffset = invWidth * 0.5;		
	float heightOffset = invHeight * 0.5;		
	
	float2 indexes = (vViewPos.xz - lightBounds.xy) * lightBounds.zw;
	indexes = clamp(indexes,0,8);
	int index = (int)indexes.x + (int)(indexes.y) * 9;
	float4 indexBounds = tex2Dlod(dataTexture, float4(widthOffset,heightOffset,0,0));
	
	float2 debugColors = vColorOut.xy * 0.5 + ((fmod(floor(indexes.xy),2) == 0) ? 0.1 : 0.2);
	vColorOut.xy =  debugColors;
	int toIndex = min(lightIndexLimit.y, indexBounds.x);
	vColorOut.z = (toIndex - lightIndexLimit.x) / 32;	
}
