#include <OgreUnifiedShader.h>

uniform vec4 shadowColor;

ATTRIBUTES_BEGIN
MAIN_DECLARATION
{
    gl_FragColor = shadowColor;
}
