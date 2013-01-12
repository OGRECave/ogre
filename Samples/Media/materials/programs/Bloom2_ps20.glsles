#version 100
precision mediump int;
precision mediump float;

//-------------------------------
// Bloom_ps20.glsles
// Blends using weights the blurred image with the sharp one
// Params:
//   OriginalImageWeight
//   BlurWeight
//-------------------------------

uniform sampler2D RT;
uniform sampler2D Blur1;

uniform float OriginalImageWeight;
uniform float BlurWeight;
varying vec2 texCoord;

void main()
{
    vec4 sharp;
    vec4 blur;

    sharp = texture2D( RT, texCoord);
    blur = texture2D( Blur1, texCoord);
    
    gl_FragColor = ( (blur * BlurWeight) + (sharp * OriginalImageWeight) );
    //gl_FragColor = vec4(0);
}
