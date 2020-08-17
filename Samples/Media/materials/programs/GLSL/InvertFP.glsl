uniform sampler2D RT;
varying vec2 oUv0;

void main()
{
	gl_FragColor = 1.0 - texture2D(RT, oUv0);
}
