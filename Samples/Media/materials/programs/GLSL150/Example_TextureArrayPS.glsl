#version 150

uniform sampler2DArray TextureArrayTex;
in vec4 oUv;
out vec4 fragColour;

void main(void)
{
	vec4 texcoord;
	texcoord = oUv;
	texcoord.z = floor(texcoord.z);
    vec4 c0 = texture(TextureArrayTex, texcoord.xyz);
	texcoord.z += 1.0;
    vec4 c1 = texture(TextureArrayTex, texcoord.xyz);

	fragColour = mix(c0, c1, fract(oUv.z));
}
