uniform float inverseShadowmapSize;
uniform vec4 lightColour;

uniform sampler2D shadowMap;
uniform sampler2D normalMap;

varying vec3 tangentLightDir;
varying	vec4 oUv;
varying	vec2 oUv2;

// Expand a range-compressed vector
vec3 expand(vec3 v)
{
	return (v - 0.5) * 2.0;
}

void main()
{

	// get the new normal and diffuse values
	vec3 normal = normalize(expand(texture2D(normalMap, oUv2).xyz));
	
	vec4 vertexColour = clamp(dot(normal, tangentLightDir),0.0,1.0) * lightColour;


	vec4 shadowUV = oUv;
	// point on shadowmap
	shadowUV = shadowUV / shadowUV.w;
	float centerdepth = texture2D(shadowMap, shadowUV.xy).x;

#ifndef OGRE_REVERSED_Z
	shadowUV.z = shadowUV.z * 0.5 + 0.5; // convert -1..1 to 0..1
#endif

	// shadowUV.z contains lightspace position of current object
#if PCF
  	float pixeloffset = inverseShadowmapSize;
    vec4 depths = vec4(
    	texture2D(shadowMap, shadowUV.xy + vec2(-pixeloffset, 0)).x,
    	texture2D(shadowMap, shadowUV.xy + vec2(+pixeloffset, 0)).x,
    	texture2D(shadowMap, shadowUV.xy + vec2(0, -pixeloffset)).x,
    	texture2D(shadowMap, shadowUV.xy + vec2(0, +pixeloffset)).x);

	// use depths from prev, calculate diff
	float final = (centerdepth > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.x > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.y > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.z > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.w > shadowUV.z) ? 1.0 : 0.0;
	
	final *= 0.2;

	gl_FragColor = vec4(vertexColour.xyz * final, 1.0);
	
#else
	gl_FragColor = (centerdepth > shadowUV.z) ? vertexColour : vec4(0.0, 0.0, 0.0, 1.0);
#endif
}

