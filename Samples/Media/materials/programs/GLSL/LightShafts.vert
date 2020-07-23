// ---------------------------------------------------------------------------------------
// Light shafts shaders with shadows and noise support
// Inpirated on the ATI paper https://developer.amd.com/wordpress/media/2012/10/Mitchell_LightShafts.pdf
// Ogre3D implementation by Xavier Verguín González (xavyiy [at] gmail [dot] com) [Xavyiy]
// ---------------------------------------------------------------------------------------

#include <OgreUnifiedShader.h>

// UNIFORM
uniform mat4 uWorldView;
uniform mat4 uWorldViewProj;
uniform mat4 uTexWorldViewProj;

MAIN_PARAMETERS
IN(vec4 vertex , POSITION)
OUT(vec3 vPosition, TEXCOORD0)
OUT(vec4 oUV      , TEXCOORD1)
MAIN_DECLARATION
{
    gl_Position = mul(uWorldViewProj, vertex);
    vPosition   = mul(uWorldView, vertex).xyz;
    oUV         = mul(uTexWorldViewProj, vertex);
}