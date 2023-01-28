#include <OgreUnifiedShader.h>

SAMPLER2D(RT, 0);

MAIN_PARAMETERS
IN(vec2 oUv0, TEXCOORD0)
MAIN_DECLARATION
{
    vec3 greyscale = vec3_splat(dot(texture2D(RT, oUv0).rgb, vec3(0.3, 0.59, 0.11)));
	gl_FragColor = vec4(greyscale, 1.0);
}
