attribute vec3 tangent;

uniform mat4 world;
uniform mat4 worldViewProj;
uniform mat4 texViewProj;
uniform vec4 lightPosition; // object space
uniform vec4 shadowDepthRange;

varying vec3 tangentLightDir;


void main()
{
	gl_Position = ftransform();
	
	vec4 worldPos = world * gl_Vertex;

	// Get object space light direction 
    vec3 lightDir = normalize(lightPosition.xyz -  (gl_Vertex.xyz * lightPosition.w));

	// calculate shadow map coords
	gl_TexCoord[0] = texViewProj * worldPos;
#if LINEAR_RANGE
	// adjust by fixed depth bias, rescale into range
	gl_TexCoord[0].z = (gl_TexCoord[0].z - shadowDepthRange.x) * shadowDepthRange.w;
#endif

	// pass the main uvs straight through unchanged 
	gl_TexCoord[1] = gl_MultiTexCoord0;

	// Calculate the binormal (NB we assume both normal and tangent are 
	// already normalised) 
	vec3 binormal = cross(gl_Normal, tangent); 

	// Form a rotation matrix out of the vectors 
	mat3 rotation = mat3(tangent, binormal, gl_Normal); 
    
	// Transform the light vector according to this matrix 
	tangentLightDir = normalize(rotation * lightDir); 

}

