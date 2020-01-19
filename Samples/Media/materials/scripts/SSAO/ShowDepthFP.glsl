#version 120
varying vec2 oUv0;

uniform sampler2D mrt1;
	
void main()
{
    float depth = texture2D(mrt1, oUv0).w;
    gl_FragColor = vec4(vec3(pow(depth, 0.3)), 1.0);
}
