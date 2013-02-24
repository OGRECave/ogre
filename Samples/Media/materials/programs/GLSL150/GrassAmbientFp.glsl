#version 150

uniform sampler2D diffuseMap;

in vec4 oUv0;
in vec4 oColour;

out vec4 fragColour;

/// grass_vp ambient
void main()
{	
    vec4 texColor = texture(diffuseMap, oUv0.xy);
    fragColour = vec4(oColour.rgb, texColor.a);
}
