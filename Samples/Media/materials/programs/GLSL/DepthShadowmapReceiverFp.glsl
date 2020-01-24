uniform float inverseShadowmapSize;
uniform float fixedDepthBias;
uniform float gradientClamp;
uniform float gradientScaleBias;

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

    // gradient calculation
  	float pixeloffset = inverseShadowmapSize;
    vec4 depths = vec4(
        texture2D(shadowMap, shadowUV.xy + vec2(-pixeloffset, 0.0)).x,
        texture2D(shadowMap, shadowUV.xy + vec2(+pixeloffset, 0.0)).x,
        texture2D(shadowMap, shadowUV.xy + vec2(0.0, -pixeloffset)).x,
        texture2D(shadowMap, shadowUV.xy + vec2(0.0, +pixeloffset)).x);

	vec2 differences = abs( depths.yw - depths.xz );
	float gradient = min(gradientClamp, max(differences.x, differences.y));
	float gradientFactor = gradient * gradientScaleBias;

	// visibility function
	float depthAdjust = gradientFactor + (fixedDepthBias * centerdepth);
	float finalCenterDepth = centerdepth + depthAdjust;

	// shadowUV.z contains lightspace position of current object
#if PCF
	// use depths from prev, calculate diff
	depths += depthAdjust;
	float final = (finalCenterDepth > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.x > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.y > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.z > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.w > shadowUV.z) ? 1.0 : 0.0;
	
	final *= 0.2;

	gl_FragColor = vec4(outColor.xyz * final, 1.0);
	
#else
	gl_FragColor = (finalCenterDepth > shadowUV.z) ? vec4(outColor.xyz,1) : vec4(0,0,0,1);
#endif
}

