#include <OgreUnifiedShader.h>

SAMPLER2D(accumTexture, 0);
SAMPLER2D(revealageTexture, 1);

#ifdef OGRE_HLSL
void main(vec4 pos : POSITION, vec2 oUv0 : TEXCOORD0, out vec4 gl_FragColor : COLOR)
#else
varying vec2 oUv0;
void main()
#endif
{
    // Weighted Blended Order-Independent Transparency, Listing 4
    vec4 accum = texture2D(accumTexture, oUv0);
    float r = accum.a;
    accum.a = texture2D(revealageTexture, oUv0).r;
    gl_FragColor = vec4(accum.rgb / clamp(accum.a, 1e-4, 5e4), r);
}