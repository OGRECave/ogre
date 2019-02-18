uniform mat4 world;
uniform mat4 worldViewProj;
uniform mat4 texViewProj;
uniform vec4 lightPosition; // object space

attribute vec4 vertex;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec4 uv0;

varying vec3 tangentLightDir;
varying	vec4 oUv;
varying	vec2 oUv2;

void main()
{
	vec4 worldPos = world * vertex;

	// Get object space light direction 
    vec3 lightDir = normalize(lightPosition.xyz -  (vertex.xyz * lightPosition.w));

	// calculate shadow map coords
	oUv = texViewProj * worldPos;

	// pass the main uvs straight through unchanged 
	oUv2 = uv0.xy;

	// Calculate the binormal (NB we assume both normal and tangent are 
	// already normalised) 
	vec3 binormal = cross(normal, tangent); 

	// Form a rotation matrix out of the vectors, column major for glsl es
	mat3 rotation = mat3(vec3(tangent[0], binormal[0], normal[0]),
						vec3(tangent[1], binormal[1], normal[1]),
						vec3(tangent[2], binormal[2], normal[2]));
    
	// Transform the light vector according to this matrix 
	tangentLightDir = normalize(rotation * lightDir); 
	
	gl_Position = worldViewProj * vertex;
}

