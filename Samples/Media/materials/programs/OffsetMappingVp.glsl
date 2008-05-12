attribute vec3 tangent;

uniform vec4 lightPosition; // object space 
uniform vec4 lightPosition1; // object space 
uniform vec4 eyePosition;   // object space 
uniform vec4 spotDirection; // object space
uniform vec4 spotDirection1; // object space
uniform mat4 worldViewProj; // not actually used but here for compat with HLSL
uniform mat4 worldMatrix;
uniform mat4 texViewProj1;
uniform mat4 texViewProj2;


varying vec3 tangentEyeDir;
varying vec3 tangentLightDir[2];
varying vec3 tangentSpotDir[2];
varying vec4 shadowUV[2];

void main()
{
	gl_Position = ftransform();

	vec4 worldPos = worldMatrix * gl_Vertex;

	shadowUV[0] = texViewProj1 * worldPos;
	shadowUV[1] = texViewProj2 * worldPos;

	// pass the main uvs straight through unchanged 
	gl_TexCoord[0] = gl_MultiTexCoord0;

	// calculate tangent space light vector 
	// Get object space light direction 
    vec3 lightDir = normalize(lightPosition.xyz -  (gl_Vertex.xyz * lightPosition.w));
	vec3 lightDir1 = normalize(lightPosition1.xyz -  (gl_Vertex.xyz * lightPosition1.w));
	
	vec3 eyeDir = (eyePosition - gl_Vertex).xyz; 

	// Calculate the binormal (NB we assume both normal and tangent are 
	// already normalised) 
	vec3 binormal = cross(gl_Normal, tangent); 

	// Form a rotation matrix out of the vectors 
	mat3 rotation = mat3(tangent, binormal, gl_Normal); 
    
	// Transform the light vector according to this matrix 
	tangentLightDir[0] = normalize(rotation * lightDir); 
	tangentLightDir[1] = normalize(rotation * lightDir1); 
	// Invert the Y on the eye dir since we'll be using this to alter UVs and
	// GL has Y inverted
	tangentEyeDir = normalize(rotation * eyeDir) * vec3(1, -1, 1); 

	tangentSpotDir[0] = normalize(rotation * -spotDirection.xyz);
	tangentSpotDir[1] = normalize(rotation * -spotDirection1.xyz);	
}
