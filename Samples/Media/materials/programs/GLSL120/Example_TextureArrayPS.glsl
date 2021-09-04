#ifdef OGRE_GLSL
#version 120
#extension GL_EXT_texture_array : enable
#endif

#include <OgreUnifiedShader.h>

uniform SAMPLER2DARRAY(TextureArrayTex, 0);

MAIN_PARAMETERS
IN(vec3 oUv, TEXCOORD0)
MAIN_DECLARATION
{
	vec3 texcoord;
	texcoord = oUv;
	texcoord.z = floor(texcoord.z);
    vec4 c0 = texture2DArray(TextureArrayTex, texcoord);
	texcoord.z += 1.0;
    vec4 c1 = texture2DArray(TextureArrayTex, texcoord);

	gl_FragColor = mix(c0, c1, fract(oUv.z));
}
