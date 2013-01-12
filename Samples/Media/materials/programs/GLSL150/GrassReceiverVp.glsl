#version 150

uniform mat4 world;
uniform mat4 worldViewProj;
uniform mat4 texViewProj;
uniform vec4 camObjPos;
uniform vec4 objSpaceLight;
uniform vec4 lightColour;
uniform vec4 offset;

in vec4 position;
in vec4 normal;
in vec4 uv0;

out vec4 oShadowUV;
out vec3 oUv0;
out vec4 oColour;

//////////////////////// GRASS SHADOW RECEIVER
void main()
{	    
	vec4 mypos = position;
	vec4 factor = vec4(1.0, 1.0, 1.0, 1.0) - uv0.yyyy;
	mypos = mypos + offset * factor;
	gl_Position = worldViewProj * mypos;
	oUv0.xy = uv0.xy;    
    // Transform position to world space
	vec4 worldPos = world * mypos;
	// calculate shadow map coords
	oShadowUV = texViewProj * worldPos;
       
    // Make vec from vertex to camera
    vec4 EyeVec = camObjPos - mypos;
    // Dot the v to eye and the normal to see if they point
    // in the same direction or opposite
    float alignedEye = dot(normal, EyeVec); // -1..1
    // If aligned is negative, we need to flip the normal
	vec4 myNormal = normal;
    if (alignedEye < 0.0)
        myNormal = -normal;
    //oNormal = normal;
    
  	// get vertex light direction (support directional and point)
	vec3 lightVec = normalize(objSpaceLight.xyz - (mypos.xyz * objSpaceLight.w).xyz);
    // Dot the v to light and the normal to see if they point
    // in the same direction or opposite
    float alignedLight = dot(myNormal.xyz, lightVec); // -1..1
    // If aligned is negative, shadowing/lighting is not possible.
    oUv0.z = (alignedLight < 0.0) ? 0.0 : 1.0 ;
         
    float diffuseFactor = max(alignedLight, 0.0);
	//oColour = diffuseFactor * lightColour;    
	oColour = lightColour;    
}
