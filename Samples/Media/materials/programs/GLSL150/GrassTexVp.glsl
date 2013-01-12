#version 150

out vec4 oUv0;

in vec4 position;
in vec4 uv0;

uniform mat4 worldViewProj;
uniform vec4 ambient;
uniform vec4 objSpaceLight;
uniform vec4 lightColour;
uniform vec4 offset;

void main()
{	    
     // Position
	vec4 mypos = position;
	vec4 factor = vec4(1.0, 1.0, 1.0, 1.0) - uv0.yyyy;
	mypos = mypos + offset * factor;
	gl_Position = worldViewProj * mypos;
    // Texture Coord
	oUv0.xy = uv0.xy;
}
