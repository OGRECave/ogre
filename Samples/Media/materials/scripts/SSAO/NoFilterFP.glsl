#version 120
varying vec2 uv;

uniform sampler2D sOcclusion;

void main ()
{
    gl_FragColor = vec4(texture2D(sOcclusion, uv).xyz, 1);
} 