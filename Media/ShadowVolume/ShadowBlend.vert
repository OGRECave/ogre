#include <OgreUnifiedShader.h>

uniform mat4 worldViewProj;

ATTRIBUTES_BEGIN
ATTRIBUTE(vec4 vertex, POSITION)
MAIN_DECLARATION
{
    gl_Position = mul(worldViewProj, vertex);
}
