#version 150

in vec4 position;
in vec3 normal;

uniform vec3 eyePosition;
uniform mat4 worldviewproj;

#if FOGLINEAR || FOGEXPONENTIAL || FOGEXPONENTIAL2
uniform vec4 fogParams;
#endif

out vec3 oPos;
out vec4 oNormAndFogVal;
out vec3 oEyePos;

void main()
{
	gl_Position = worldviewproj * position;
	
	oPos = position.xyz;
	oNormAndFogVal.xyz = normal;
	oEyePos = eyePosition;
		
	// Fog like in the terrain component, but exp2 added
	#if FOGLINEAR
	oNormAndFogVal.w = clamp((gl_Position.z - fogParams.y) * fogParams.w, 0.0, 1.0);
	#endif
	#if FOGEXPONENTIAL
    // Fog density increases  exponentially from the camera (fog = 1/e^(distance * density))
	oNormAndFogVal.w = 1 - clamp(1 / (exp(gl_Position.z * fogParams.x)), 0.0, 1.0);
	#endif
	#if FOGEXPONENTIAL2
    // Fog density increases at the square of FOG_EXP, i.e. even quicker (fog = 1/e^(distance * density)^2)
	float distanceTimesDensity = exp(gl_Position.z * fogParams.x);
	oNormAndFogVal.w = 1 - clamp(1 / (distanceTimesDensity * distanceTimesDensity), 0.0, 1.0);
	#endif
}
