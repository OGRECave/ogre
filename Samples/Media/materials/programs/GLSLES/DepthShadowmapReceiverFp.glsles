#version 100

precision highp int;
precision highp float;

uniform float inverseShadowmapSize;
uniform float fixedDepthBias;
uniform float gradientClamp;
uniform float gradientScaleBias;
#if FUZZY_TEST
uniform float shadowFuzzyWidth;
#endif
uniform sampler2D shadowMap;

varying vec4 oShadowUV;
varying vec4 vColour;

void main()
{
	vec4 shadowUV = oShadowUV;
	// point on shadowmap
#if LINEAR_RANGE
	shadowUV.xy = shadowUV.xy / shadowUV.w;
#else
	shadowUV = shadowUV / shadowUV.w;
#endif

	float centerdepth = texture2D(shadowMap, shadowUV.xy).x;
    
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

#if FUZZY_TEST
	// Fuzzy test - introduces some ghosting in result and doesn't appear to be needed?
	//float visibility = saturate(1 + delta_z / (gradient * shadowFuzzyWidth));
	float visibility = saturate(1.0 + (finalCenterDepth - shadowUV.z) * shadowFuzzyWidth * shadowUV.w);

	gl_FragColor = vColour * visibility;
#else
	// Hard test
#if PCF
	// Use depths from prev, calculate diff
	depths += depthAdjust;
	float final = (finalCenterDepth > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.x > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.y > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.z > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.w > shadowUV.z) ? 1.0 : 0.0;
	
	final *= 0.2;

	gl_FragColor = vec4(vColour.xyz * final, 1.0);
	
#else
	gl_FragColor = (finalCenterDepth > shadowUV.z) ? vColour : vec4(0.0, 0.0, 0.0, 1.0);
#endif

#endif
   
}
