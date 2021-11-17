#include <OgreUnifiedShader.h>

OGRE_UNIFORMS(
    uniform vec4 shadowColor;
)

MAIN_PARAMETERS
MAIN_DECLARATION
{
    gl_FragColor = shadowColor;
}
