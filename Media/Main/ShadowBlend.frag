#include <OgreUnifiedShader.h>

OGRE_UNIFORMS(
    uniform vec4 shadowColor;
)

MAIN(VOID)
{
    gl_FragColor = shadowColor;
}
