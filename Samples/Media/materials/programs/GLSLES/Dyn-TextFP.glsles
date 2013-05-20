#version 100

precision mediump int;
precision mediump float;

uniform sampler2D Image;
uniform sampler2D Thaw;
varying vec4 oUv;

void main(void)
{
    vec4 c0 = texture2D(Image, oUv.xy);
    vec4 c1 = texture2D(Thaw, oUv.xy);

	gl_FragColor = c0 * c1;
}
