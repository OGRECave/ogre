#include <OgreUnifiedShader.h>

OGRE_UNIFORMS(
uniform mat4 worldViewProj;
)

MAIN_PARAMETERS
IN(vec4 vertex, POSITION)
IN(vec2 uv0, TEXCOORD0)
OUT(vec2 oUv0, TEXCOORD0)
MAIN_DECLARATION
{
	// Use standardise transform, so work accord with render system specific (RS depth, requires texture flipping, etc)
    gl_Position = mul(worldViewProj, vertex);
    oUv0 = uv0;
}
