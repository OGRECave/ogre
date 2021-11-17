#include <OgreUnifiedShader.h>

OGRE_UNIFORMS(
    uniform mat4 worldViewProj;
)

MAIN_PARAMETERS
IN(vec4 vertex, POSITION)
MAIN_DECLARATION
{
    gl_Position = mul(worldViewProj, vertex);
}
