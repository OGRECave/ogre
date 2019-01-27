#version 120
varying vec2 oUv0;

uniform sampler2D mrt1;
uniform sampler2D tex;
	
void main()
{
    float depth = texture2D(mrt1, oUv0).w;
    gl_FragColor = vec4(texture2D(tex, vec2(depth*20.0, 0)).rgb, 1.0);
}
