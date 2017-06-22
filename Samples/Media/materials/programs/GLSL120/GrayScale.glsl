uniform sampler2D RT;
varying vec2 oUv0;

void main()
{
    vec3 greyscale = vec3(dot(texture2D(RT, oUv0).rgb, vec3(0.3, 0.59, 0.11)));
	gl_FragColor = vec4(greyscale, 1.0);
}
