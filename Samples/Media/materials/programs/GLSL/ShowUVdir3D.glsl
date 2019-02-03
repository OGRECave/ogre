varying vec4 oUv0;

// Basic fragment program to display 3d uv
void main()
{
	vec3 n = normalize(oUv0.xyz);
	gl_FragColor = vec4(n.x, n.y, n.z, 1.0);
}
