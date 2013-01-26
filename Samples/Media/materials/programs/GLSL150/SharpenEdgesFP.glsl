#version 150

uniform sampler2D RT;
uniform vec2 vTexelSize;

in vec2 oUv0;
in vec2 oUv1;
out vec4 fragColour;

void main()
{
	vec2 usedTexelED[8];
	usedTexelED[0] = vec2(-1, -1);
	usedTexelED[1] = vec2(0, -1);
	usedTexelED[2] = vec2(1, -1);
	usedTexelED[3] = vec2(-1, 0);
	usedTexelED[4] = vec2(1, 0);
	usedTexelED[5] = vec2(-1, 1);
	usedTexelED[6] = vec2(0, 1);
	usedTexelED[7] = vec2(1, 1);

    vec4 tc = texture(RT, oUv0);
	vec4 cAvgColor = vec4(9.0 * tc);

	for(int t=0; t<8; t++)
		cAvgColor -= texture(RT, oUv0 + vTexelSize * usedTexelED[t]);

	fragColour = cAvgColor;
}
