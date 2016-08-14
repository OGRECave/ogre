//-----------------------------------------------------------------------------
// Program Name: SL_Lighting
// Program Desc: Per pixel lighting functions.
// Program Type: Vertex/Pixel shader
// Language: GLSL
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void SL_TransformNormal(in mat4 m, 
				   in vec3 v, 
				   out vec3 vOut)
{
	vOut = mat3(m) * v;
}

//-----------------------------------------------------------------------------
void SL_TransformPosition(in mat4 mWorldView, 
				   in vec4 vPos, 
				   out vec3 vOut)
{
	vOut = (mWorldView * vPos).xyz;
}

//-----------------------------------------------------------------------------
void SL_Light_Directional_Diffuse(
				   in vec3 vNormal,
				   in vec3 vNegLightDirView,
				   in vec3 vDiffuseColour, 
				   in vec3 vBaseColour, 
				   out vec3 vOut)
{
	vec3 vNormalView = normalize(vNormal);
	float nDotL = dot(vNormalView, vNegLightDirView);
	
	vOut = vBaseColour + vDiffuseColour * min(max(nDotL, 0.0), 1.0);
}

//-----------------------------------------------------------------------------
void SL_Light_Directional_DiffuseSpecular(
					in vec3 vNormal,
					in vec3 vViewPos,					
					in vec3 vNegLightDirView,
					in vec3 vDiffuseColour, 
					in vec3 vSpecularColour, 
					in float fSpecularPower, 
					in vec3 vBaseDiffuseColour,
					in vec3 vBaseSpecularColour,					
					out vec3 vOutDiffuse,
					out vec3 vOutSpecular)
{
	vOutDiffuse  = vBaseDiffuseColour;
	vOutSpecular = vBaseSpecularColour;
	
	vec3 vNormalView = normalize(vNormal);		
	float nDotL		   = dot(vNormalView, vNegLightDirView);			
	vec3 vView       = -normalize(vViewPos);
	vec3 vHalfWay    = normalize(vView + vNegLightDirView);
	float nDotH        = dot(vNormalView, vHalfWay);
	
	nDotL = max(nDotL, 0);
	vOutDiffuse  += vDiffuseColour * nDotL;		
	vOutSpecular += vSpecularColour * pow(clamp(nDotH, 0.0, 1.0), fSpecularPower);						
}

//-----------------------------------------------------------------------------
void SL_Light_Ambient_Diffuse_Inner(
				    in vec3 vNormal,
				    in vec3 vLightView,
				    in float fLightDist,
				    in vec3 vNegLightDirView,
				    in vec3 vSpotParams,
				    in vec3 vDiffuseColour, 
				    inout vec3 vColorOut)
{
	float fLightDistInv = 1.0 / fLightDist;
	float nDotL = dot(vNormal, vLightView) * fLightDistInv;
	
	float fAtten = (1.0 - (fLightDist * vSpotParams.x));
	fAtten = fAtten * fAtten;

	float rho = dot(vNegLightDirView, vLightView) * fLightDistInv;
	float fSpotT = min(max((rho - vSpotParams.y) * vSpotParams.z, 0.0), 1.0);

	nDotL = (nDotL < 0.0 ? 0.0 : 1.0) * (0.7 + (0.3 * nDotL));
	vColorOut += vDiffuseColour * 2.0* nDotL * fAtten * fSpotT;	
}

//-----------------------------------------------------------------------------
void SL_Light_Ambient_Diffuse(
				    in vec3 vNormal,
				    in vec3 vViewPos,
				    in vec3 vLightPosView,
				    in vec3 vNegLightDirView,
				    in vec3 vSpotParams,
				    in vec3 vDiffuseColour, 
				    inout vec3 vColorOut)
{
	vec3 vLightView = vLightPosView - vViewPos;
	float fLightDist = length(vLightView);
	if (fLightDist * vSpotParams.x < 1)
	{
        SL_Light_Ambient_Diffuse_Inner(vNormal, vLightView, fLightDist,
				vNegLightDirView, vSpotParams, vDiffuseColour, vColorOut);
	}
}

//-----------------------------------------------------------------------------
void SL_Light_Segment_Texture_Ambient_Diffuse(
				    in vec3 vNormal,
				    in vec3 vViewPos,
				    in sampler2D dataTexture,
				    in vec2 lightIndexLimit,
					in vec4 lightBounds,
					in float invWidth,
					in float invHeight,
				    inout vec3 vColorOut)
{
	float widthOffset = invWidth * 0.5;		
	float heightOffset = invHeight * 0.5;	
	
	vec2 indexes = (vViewPos.xz - lightBounds.xy) * lightBounds.zw;
	indexes = min(max(indexes, 0.0), 8.0);
	int index = int(indexes.x) + int(indexes.y) * 9;
	widthOffset += invWidth * 3.0 * float(index);
	
	vec4 indexBounds = textureLod(dataTexture, vec2(widthOffset,heightOffset), 0.0);
	int toIndex = int(min(lightIndexLimit.y, indexBounds.x));
	for(int i = int(lightIndexLimit.x); i <= toIndex; ++i)
	{
		float heightCoord = heightOffset + invHeight * float(i);
		vec4 dat1 = textureLod(dataTexture, vec2(widthOffset,heightCoord), 0.0);
		
		vec3 vLightView  = dat1.xyz - vViewPos;
		float fLightDist = length(vLightView);
		if (fLightDist * dat1.w < 1.0)
		{
			vec4 dat2 = textureLod(dataTexture, vec2(widthOffset + invWidth,heightCoord),0.0);
			vec4 dat3 = textureLod(dataTexture, vec2(widthOffset + invWidth * 2.0,heightCoord),0.0);
			SL_Light_Ambient_Diffuse_Inner(vNormal, vLightView, fLightDist, dat2.xyz, vec3(dat1.w, dat2.w, dat3.w), dat3.xyz, vColorOut);
		}
	}
}

//-----------------------------------------------------------------------------
void SL_Light_Segment_Debug(
				    in vec3 vNormal,
				    in vec3 vViewPos,
				    in sampler2D dataTexture,
				    in vec2 lightIndexLimit,
					in vec4 lightBounds,
					in float invWidth,
					in float invHeight,
				    inout vec3 vColorOut)
{
	float widthOffset = invWidth * 0.5;		
	float heightOffset = invHeight * 0.5;	

	vec2 indexes = (vViewPos.xz - lightBounds.xy) * lightBounds.zw;
	indexes = min(max(indexes, 0.0), 8.0);
	int index = int(indexes.x + (indexes.y * 9.0));
	vec4 indexBounds = textureLod(dataTexture, vec2(widthOffset,heightOffset),0.0);

    vec2 debugColors = vColorOut.xy * 0.5 + ((mod(floor(indexes.xy),2.0) == vec2(0)) ? 0.1 : 0.2);
    vColorOut.xy =  debugColors;
    int toIndex = int(min(lightIndexLimit.y, indexBounds.x));
    vColorOut.z = (float(toIndex) - lightIndexLimit.x) / 32.0;
}
