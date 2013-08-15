#version 150

uniform float inverseShadowmapSize;
uniform float fixedDepthBias;
uniform float gradientClamp;
uniform float gradientScaleBias;
uniform float shadowFuzzyWidth;

uniform sampler2D shadowMap;
out vec4 fragColour;
in vec4 shadowUV;
in vec4 oColour;

void main()
{
	float centerdepth = texture(shadowMap, shadowUV.xy).x;
    
    // gradient calculation
  	float pixeloffset = inverseShadowmapSize;
    vec4 depths = vec4(
    	texture(shadowMap, shadowUV.xy + vec2(-pixeloffset, 0)).x,
    	texture(shadowMap, shadowUV.xy + vec2(+pixeloffset, 0)).x,
    	texture(shadowMap, shadowUV.xy + vec2(0, -pixeloffset)).x,
    	texture(shadowMap, shadowUV.xy + vec2(0, +pixeloffset)).x);

	vec2 differences = abs( depths.yw - depths.xz );
	float gradient = min(gradientClamp, max(differences.x, differences.y));
	float gradientFactor = gradient * gradientScaleBias;

	// visibility function
	float depthAdjust = gradientFactor + (fixedDepthBias * centerdepth);
	float finalCenterDepth = centerdepth + depthAdjust;

	// shadowUV.z contains lightspace position of current object

#if FUZZY_TEST
	// fuzzy test - introduces some ghosting in result and doesn't appear to be needed?
	//float visibility = saturate(1 + delta_z / (gradient * shadowFuzzyWidth));
	float visibility = saturate(1 + (finalCenterDepth - shadowUV.z) * shadowFuzzyWidth * shadowUV.w);

	fragColour = vertexColour * visibility;
#else
	// hard test
#if PCF
	// use depths from prev, calculate diff
	depths += depthAdjust;
	float final = (finalCenterDepth > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.x > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.y > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.z > shadowUV.z) ? 1.0 : 0.0;
	final += (depths.w > shadowUV.z) ? 1.0 : 0.0;
	
	final *= 0.2;

	fragColour = vec4(oColour.xyz * final, 1);
	
#else
	fragColour = (finalCenterDepth > shadowUV.z) ? oColour : vec4(0.5,0,0,1);
#endif

#endif
   
}

