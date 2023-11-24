#define PCF

#ifdef PCF
uniform float inverseShadowmapSize;
#endif
uniform sampler2D shadowMap;

varying	vec4 oUv;
varying	vec4 outColor;

void main()
{
	vec4 shadowUV = oUv;
	// point on shadowmap
	shadowUV = shadowUV / shadowUV.w;
	float centerdepth = texture2D(shadowMap, shadowUV.xy).x;

#ifndef OGRE_REVERSED_Z
	shadowUV.z = shadowUV.z * 0.5 + 0.5; // convert -1..1 to 0..1
#endif

	// shadowUV.z contains lightspace position of current object
#ifdef PCF
  	float pixeloffset = inverseShadowmapSize;
    vec4 depths = vec4(
        texture2D(shadowMap, shadowUV.xy + vec2(-pixeloffset, 0.0)).x,
        texture2D(shadowMap, shadowUV.xy + vec2(+pixeloffset, 0.0)).x,
        texture2D(shadowMap, shadowUV.xy + vec2(0.0, -pixeloffset)).x,
        texture2D(shadowMap, shadowUV.xy + vec2(0.0, +pixeloffset)).x);

	// use depths from prev, calculate diff
	float final = (centerdepth > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.x > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.y > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.z > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.w > shadowUV.z) ? 1.0 : 0.0;
	
	final *= 0.2;

	gl_FragColor = vec4(outColor.xyz * final, 1.0);
	
#else
	gl_FragColor = (centerdepth > shadowUV.z) ? vec4(outColor.xyz,1) : vec4(0,0,0,1);
#endif
}

