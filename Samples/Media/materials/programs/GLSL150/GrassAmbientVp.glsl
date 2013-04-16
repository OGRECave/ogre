#version 150


uniform mat4 worldViewProj;
uniform vec4 ambient;
uniform vec4 offset;

in vec4 position;
in vec4 normal;
in vec4 uv0;

out vec4 oUv0;
out vec4 oColour;

/// grass_vp ambient
void main()
{	
     // Position
	vec4 mypos = position;
	vec4 factor = vec4(1.0, 1.0, 1.0, 1.0) - uv0.yyyy;
	mypos = mypos + offset * factor;
  	gl_Position = worldViewProj * mypos;
    // Texture Coord
	oUv0.xy = uv0.xy;      
    /*
    // Normal
    // Make vec from vertex to camera
    vec4 EyeVec = camObjPos - mypos;
    // Dot the v to eye and the normal to see if they point
    //  in the same direction or opposite
    float aligned = dot(normal, EyeVec); // -1..1
    // If aligned is negative, we need to flip the normal
    if (aligned < 0)  
        normal = -normal;  
    //oNormal = normal; 
    */
    // Color    
	oColour = ambient; 
}
