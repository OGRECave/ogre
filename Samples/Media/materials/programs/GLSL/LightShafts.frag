// ---------------------------------------------------------------------------------------
// Light shafts shaders with shadows and noise support
// Inpirated on the ATI paper https://developer.amd.com/wordpress/media/2012/10/Mitchell_LightShafts.pdf
// Ogre3D implementation by Xavier Verguín González (xavyiy [at] gmail [dot] com) [Xavyiy]
// ---------------------------------------------------------------------------------------

#include <OgreUnifiedShader.h>

// UNIFORM
uniform vec4    uAttenuation;
uniform vec3    uLightPosition;
uniform SAMPLER2D(uDepthMap,  0);
uniform SAMPLER2D(uCookieMap, 1);
uniform SAMPLER2D(uNoiseMap,  2);
uniform float Time;

MAIN_PARAMETERS
IN(vec3 vPosition, TEXCOORD0)
IN(vec4 oUV      , TEXCOORD1)
MAIN_DECLARATION
{
    vec4 iUV = oUV / oUV.w;

    float Depth  = texture2D(uDepthMap,  iUV.xy).r;

#if !defined(OGRE_HLSL) && !defined(OGRE_REVERSED_Z)
    iUV.z = iUV.z * 0.5 + 0.5;
#endif

    if (Depth < iUV.z)
    {
        gl_FragColor = vec4(0,0,0,1);
    }
    else
    {
        vec4 Cookie = texture2D(uCookieMap, iUV.xy);
        vec2 Noise  = vec2(texture2D(uNoiseMap,  iUV.xy - Time).r,
                           texture2D(uNoiseMap,  iUV.xy + Time).g);

        float noise  = Noise.x * Noise.y;
        float length_ = length(uLightPosition - vPosition);
        float atten  = 1.0 / (uAttenuation.y + uAttenuation.z*length_ + 20.0*uAttenuation.w*length_*length_);

        gl_FragColor = vec4(Cookie.rgb * atten * noise , 1);
    }
}