#version 150

uniform sampler2D Image;
uniform sampler2D Thaw;
in vec4 oUv;
out vec4 fragColour;

void main(void)
{
    vec4 c0 = texture(Image, oUv.xy);
    vec4 c1 = texture(Thaw, oUv.xy);

	fragColour = c0 * c1;
}
