uniform sampler2D RT;
varying vec2 oUv0;

void main()
{
	float nColors = 8.0;
	float gamma = 0.6;

	vec4 texCol = vec4(texture2D(RT, oUv0));
	vec3 tc = texCol.xyz;
	tc = pow(tc, vec3(gamma));
	tc = tc * nColors;
	tc = floor(tc);
	tc = tc / nColors;
	tc = pow(tc, vec3(1.0/gamma));
	gl_FragColor = vec4(tc, texCol.w);
}
