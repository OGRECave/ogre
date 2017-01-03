//-----------------------------------------------------------------------------
// Program Name: SGXLib_TextureAtlas
// Program Desc: Texture Atlas functions.
// Program Type: Vertex/Pixel shader
// Language: GLSL
//-----------------------------------------------------------------------------
#extension GL_ARB_shader_texture_lod : require

float mipmapLevel(in vec2 coords, in vec2 texSize)
{
    coords = coords.xy * texSize;
    vec2 dx = dFdx(coords.xy);
    vec2 dy = dFdy(coords.xy);
    float Px = length(dx);
    float Py = length(dy);
    float Pmax = max(Px, Py);
    return log2(max(Pmax,1.0));
}


void SGX_Atlas_Sample_Auto_Adjust(in sampler2D textureSampler,
                                  in vec2 origTexCoord,
                                  in vec2 atlasTexCoord,
                                  in vec4 textureData,
                                  in vec2 imageSize,
                                  out vec4 texel)
{
    // Most of the idea for this function has been taken from the code under the webpage:
    // http://www.gamedev.net/topic/534149-solved-texture-seams-using-atlas-with-screenshots/


    // origTexCoord - original texture coordintates as received from the vertex buffer.
    // atlasTexCoord - texture coordinates that have gone through on which different mathematical
    //                  functions to simulate different texture addressing modes (wrap, mirror, clamp).
    // textureData.xy - top left corner (in 0-1 units) where the needed texture in the texture atlas begins.
    // textureData.zw - width and height in power of 2 for the needed texture in the texture atlas.
    // imageSize - size of the image in pixels.
    // texel - [Output] The color of the pixel at the requested position.

    vec2 startPos = textureData.xy;


    // Calculate the tileSize by using the power of 2 value.
    vec2 pwrs = textureData.zw;

    // Calculate the power of 2 size of the maximum avialable Mipmap
    // Note: We limit the amount of available LODs to [actual number]
    // - 2 as some atlas packaging tools do not include the last 2
    // LODs when packeging DXT format atlas textures.
    float availableLODCount = min(pwrs.x,pwrs.y) - 2.0;
    vec2 tileSize = pow(vec2(2.0, 2.0),pwrs);
    vec2 relativeTileSize = tileSize / imageSize;

    // Retrieve the mipmap level for this pixel clamped by the power of 2 value.
    float lod = clamp(mipmapLevel(origTexCoord, tileSize), 0.0, availableLODCount);

    // Get the width/height of the mip surface we've decided on.
    vec2 mipSize = pow(vec2(2.0, 2.0), pwrs.xy - ceil(lod));

    // Compute the inverse fraction size for the tile.
    //vec2 lodSize = mipSize * imageSize / tileSize;
    vec2 lodSizeInv = (relativeTileSize / mipSize);

    // Compute the new coordinates.
    //atlasTexCoord = atlasTexCoord * ((lodSize * (tileSize / imageSize) - 1.0) / lodSize) + (0.5 / lodSize) + startPos;
    atlasTexCoord = atlasTexCoord * (relativeTileSize - lodSizeInv) + (0.5 * lodSizeInv) + startPos;

    // Return the pixel from the correct mip surface of the atlas.
    texel = texture2DLod(textureSampler, atlasTexCoord, lod);
}


void SGX_Atlas_Sample_Normal(in sampler2D textureSampler,
                             in vec2 origTexCoord,
                             in vec2 atlasTexCoord,
                             in vec4 textureData,
                             in vec2 imageSize,
                             out vec4 texel)
{
    // texcoord contains:
    // x = texture atlas u
    // y = texture atlas v
    // z = derivative of original u
    // w = derivative of original v
    atlasTexCoord = textureData.xy + (atlasTexCoord * pow(vec2(2.0,2.0),textureData.zw) / imageSize);
    //texel = texture2DLod(textureSampler, vec4(atlasTexCoord, 0,0));
    texel = texture2D(textureSampler, atlasTexCoord);
}


void SGX_Atlas_Wrap(in float inpCoord, out float outCoord)
{
    outCoord = fract(inpCoord);
}


void SGX_Atlas_Clamp(in float inpCoord, out float outCoord)
{
    outCoord = clamp(inpCoord, 0.0, 1.0);
}


void SGX_Atlas_Mirror(in float inpCoord, out float outCoord)
{
    outCoord = (inpCoord + 1) * 0.5;
    outCoord = abs(fract(outCoord) * 2 - 1);
}


void SGX_Atlas_Border(in float inpCoord, out float outCoord)
{
    // The shader needs to check whether the original texcoord are
    // beyond the [0, 1] range. The shader attempts to do so without
    // using if statments which are complicated for shaders.

    // If texcoord is in the [0, 1] range then check will equal 0,
    // otherwise it will equal the number 1 or greater.
    float check = step(inpCoord, 0) + step(1, inpCoord);

    // Using the check value transport the value of the texcoord
    // beyond the [0, 1] range so it will receive the border color.
    outCoord = abs(inpCoord) + check * 2;
}
