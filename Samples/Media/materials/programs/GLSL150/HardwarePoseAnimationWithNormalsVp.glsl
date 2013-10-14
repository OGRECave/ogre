#version 150

in vec4 vertex;
in vec4 normal;
in vec4 uv0;
in vec4 uv1; // pose1pos
in vec4 uv2; // pose1norm
in vec4 uv3; // pose2pos
in vec4 uv4; // pose2norm

out vec2 oUv;
out vec4 colour;

uniform mat4 worldViewProj;
uniform vec4 anim_t;
uniform vec4 objSpaceLightPos;
uniform vec4 ambient;

// hardware pose animation (with normals)
void main()
{
	// interpolate position
	vec4 posinterp = vec4(pos + anim_t.x*uv1.xyz + anim_t.y*uv3.xyz, 1.0);

    // nlerp normal
	// nlerp normal
	// First apply the pose normals (these are actual normals, not offsets)
	vec3 ninterp = anim_t.x*uv2.xyz + anim_t.y*uv4.xyz;

	// Now add back any influence of the original normal
	// This depends on what the cumulative weighting left the normal at, if it's lacking or cancelled out
	//float remainder = 1.0 - min(anim_t.x + anim_t.y, 1.0);
	float remainder = 1.0 - min(length(ninterp), 1.0);
	ninterp = ninterp + (normal * remainder);
	ninterp = normalize(ninterp);

	gl_Position = worldViewProj * posinterp;
	oUv = uv0.xy;
	
	vec3 lightDir = normalize(
		objSpaceLightPos.xyz - (posinterp.xyz * objSpaceLightPos.w));

	// Colour it red to make it easy to identify
	float lit = clamp(dot(lightDir, ninterp), 0.0, 1.0);
	colour = vec4((ambient.rgb + vec3(lit,lit,lit)) * vec3(1.0,0.0,0.0), 1.0);
}
