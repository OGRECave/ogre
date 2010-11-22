//-----------------------------------------------------------------------------
// Program Name: SGXLib_IntegratedPSSM
// Program Desc: Texture Atlas functions.
// Program Type: Vertex/Pixel shader
// Language: HLSL
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void SGX_Atlas_Sample(in sampler2D sample, in float2 origTexcoord, in float2 atlasTexcoord, in float4 textureData, out float4 texel)
{
	//texcoord contain:
	// x = texture atlas u
	// y = texture atlas v
	// z = derivative of original u
	// w = derivative of original v
	origTexcoord = textureData.xy + origTexcoord * textureData.zw;
	atlasTexcoord = textureData.xy + atlasTexcoord * textureData.zw;
	texel = tex2D(sample, atlasTexcoord, ddx(origTexcoord), ddy(origTexcoord));
}

//-----------------------------------------------------------------------------
void SGX_Atlas_Wrap(in float inpCoord, out float outCoord)
{
	outCoord = frac(inpCoord);
}

//-----------------------------------------------------------------------------
void SGX_Atlas_Clamp(in float inpCoord, out float outCoord)
{
	outCoord = saturate(inpCoord);
}
//-----------------------------------------------------------------------------
void SGX_Atlas_Mirror(in float inpCoord, out float outCoord)
{
	outCoord = (inpCoord + 1) * 0.5;
	outCoord = abs(frac(outCoord) * 2 - 1);
}

//-----------------------------------------------------------------------------
void SGX_Atlas_Border(in float inpCoord, out float outCoord)
{
	//
	//The shader needs to check whether the original texcoord are beyond the 0,1 range.
	//The shader attempts to do so without using if statments which are complicated for shaders
	//
	
	//if texcoord is in the 0,1 range then check will equal 0, 
	//Otherwise it will equal the number 1 or greater
	float check = step(inpCoord, 0) + step(1, inpCoord);
	
	//using the check value transport the value of the texcoord beyond the 0,1 range so it will
	//recive the border color
	outCoord = abs(inpCoord) + check * 2;
}