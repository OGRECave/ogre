#include "OgreUnifiedShader.h"

SAMPLER2D(inRTT, 0);

uniform vec4 sampleOffsets[15];
uniform vec4 sampleWeights[15];

MAIN_PARAMETERS
IN(vec2 oUv0, TEXCOORD0)
MAIN_DECLARATION
{
    vec4 accum = vec4(0.0, 0.0, 0.0, 1.0);
	vec2 sampleUV;
    
    for( int i = 0; i < 15; i++ )
    {
        // Sample from adjacent points, 7 each side and central
        sampleUV = oUv0 + sampleOffsets[i].xy;
        accum += sampleWeights[i] * texture2D(inRTT, sampleUV);
    }
    
    gl_FragColor = accum;
	
}
