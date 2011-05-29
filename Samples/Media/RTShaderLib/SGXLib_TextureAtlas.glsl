//-----------------------------------------------------------------------------
// Program Name: SGXLib_IntegratedPSSM
// Program Desc: Texture Atlas functions.
// Program Type: Vertex/Pixel shader
// Language: GLSL
//-----------------------------------------------------------------------------

float mipmapLevel(vec2 coords, vec2 texSize)
{
	float2 coordInPix = coords * texSize;
	float2 dx = ddx(coordInPix);
	float2 dy = ddy(coordInPix);
	float d = max(dot(dx, dx), dot(dy, dy));
	return 0.4 * log2(d);
}
	

//-----------------------------------------------------------------------------
void SGX_Atlas_Sample(in sampler2D sample, 
		in vec2 origTexcoord, 
		in vec2 atlasTexcoord, 
		in vec4 textureData, 
		in vec2 imageSize,
		out vec4 texel)
{
	//
	// Most of the idea for this function has been taken from the code under the webpage:
	// http://www.gamedev.net/topic/534149-solved-texture-seams-using-atlas-with-screenshots/ 


	// origTexcoord - original texture coordintates as recived from the vertex buffer.
	// atlasTexcoord - texture coordinates that have gone through on which diffrent mathematical 
	//			functions to simulate diffrent texture addressing modes (wrap, mirror, clamp)
	// textureData.xy - top left corner (in 0-1 units) where the needed texture in the texture atlas begins
	// textureData.zw - width and height in power of 2 for the needed texture in the texture atlas
	// imageSize - size of the image in pixels
	// texel - [Output] The color of the pixel at the requested position
	
	float2 startPos = textureData.xy;
	
	
	// calculate the tileSize by using the power of 2 value
	//float2 pwrs = float2(6,6);
	float2 pwrs = textureData.zw;

	float pwr = min(pwrs.x,pwrs.y) - 2;	
	float2 tileSize = pow(float2(2.0,2.0),pwrs);
	
	// retrieve the mipmap level for this pixel clamped by the power of 2 value
	float lod = clamp(mipmapLevel(origTexcoord, tileSize), 0, pwr);
	
	// get the width/height of the mip surface we've decided on
	float mipSize = pow(2.0, pwr - lod);
	
	// compute the inverse fraction size for the tile 
	//float2 lodSize = mipSize * imageSize / tileSize;
	float2 lodSizeInv = tileSize / (mipSize * imageSize);
	//compute the new coordinates
	//atlasTexcoord = atlasTexcoord * ((lodSize * (tileSize / imageSize) - 1.0) / lodSize) + (0.5 / lodSize) + startPos;
	atlasTexcoord = atlasTexcoord * ((tileSize / imageSize) - lodSizeInv) + (0.5 * lodSizeInv) + startPos;
	
	//return the pixel from the correct mip surface of the atlas
	texel = texture2DLod(sample, float4(atlasTexcoord, 0, lod));
}

//-----------------------------------------------------------------------------
void SGX_Atlas_Wrap(in float inpCoord, out float outCoord)
{
	outCoord = frac(inpCoord);
}

//-----------------------------------------------------------------------------
void SGX_Atlas_Clamp(in float inpCoord, out float outCoord)
{
	outCoord = clamp(inpCoord, 0.0, 1.0);
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