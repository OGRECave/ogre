#include <OgreUnifiedShader.h>

OGRE_UNIFORMS(
uniform mat4 worldViewProj;
)

MAIN_PARAMETERS
IN(vec4 vertex, POSITION)
IN(vec3 uv0, TEXCOORD0)
OUT(vec3 oUv, TEXCOORD0)
MAIN_DECLARATION
{
	gl_Position = mul(worldViewProj, vertex);
	oUv = uv0;
}
