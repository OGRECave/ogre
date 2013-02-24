#version 150

uniform sampler2D diffuseMap;

in vec4 oUv0;
in vec2 oDepth;

out vec4 fragColour;

//////////////////////// GRASS SHADOW CASTER
void main()
{	
	float alpha = texture(diffuseMap, oUv0.xy).a;
	if (alpha > 0.001)
    {
       fragColour = vec4(1.0, 1.0, 1.0, 0.0);
    }
    else
    {
        float finalDepth = oDepth.x / oDepth.y;
        // just smear across all components 
        // therefore this one needs high individual channel precision
        fragColour = vec4(vec3(finalDepth), 1.0);
    }
}
