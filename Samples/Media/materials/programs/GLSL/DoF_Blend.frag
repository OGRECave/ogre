// "Depth of Field" demo for Ogre
// Copyright (C) 2006  Christian Lindequist Larsen
//
// This code is in the public domain. You may do whatever you want with it.

// based on "Advanced Depth of Field" by "Thorsten Scheuermann"
#include <OgreUnifiedShader.h>

#define NUM_TAPS 12						// number of taps the shader will use

uniform vec4 pixelSizeScene;			// pixel size of full resolution image

SAMPLER2D(scene, 0);				// full resolution image
SAMPLER2D(depth, 1);				// full resolution image with depth values
SAMPLER2D(blur,  2);				// downsampled and blurred image

// dofParams coefficients:
// x = near blur depth; y = focal plane depth; z = far blur depth
// w = blurriness cutoff constant
STATIC f32vec4 dofParams = vec4(0.9991, 0.9985, 0.9975, 1);

float getBlurAmount(float depth)
{
    float f;

    if (depth > dofParams.y)
    {
        // scale depth value between near blur distance and focal distance
        f = (dofParams.x - depth) / (dofParams.x - dofParams.y);
    }
    else
    {
        // scale depth value between focal distance and far blur distance
        f = (depth - dofParams.z) / (dofParams.y - dofParams.z);
    }

    return 1.0 - clamp(f, 0.0, dofParams.w);
}

MAIN_PARAMETERS
IN(vec2 oUv0, TEXCOORD0)
MAIN_DECLARATION
{
    vec2 CoC = vec2(5.0, 1.0);	                 // max and min circle of confusion (CoC) radius
    vec2 pixelSizeBlur = pixelSizeScene.zw*4.0; // pixel size of downsampled and blurred image

    vec2 poisson[NUM_TAPS];					     // contains poisson-distributed positions on the unit circle
    poisson[ 0] = vec2( 0.00,  0.00);
    poisson[ 1] = vec2( 0.07, -0.45);
    poisson[ 2] = vec2(-0.15, -0.33);
    poisson[ 3] = vec2( 0.35, -0.32);
    poisson[ 4] = vec2(-0.39, -0.26);
    poisson[ 5] = vec2( 0.10, -0.23);
    poisson[ 6] = vec2( 0.36, -0.12);
    poisson[ 7] = vec2(-0.31, -0.01);
    poisson[ 8] = vec2(-0.38,  0.22);
    poisson[ 9] = vec2( 0.36,  0.23);
    poisson[10] = vec2(-0.13,  0.29);
    poisson[11] = vec2( 0.14,  0.41);

    // Get depth of center tap and convert it into blur radius in pixels
    float centerDepth = texture2D(depth, oUv0).r;
    float centerBlur = getBlurAmount(centerDepth);
    float discRadius = max(0.0, centerBlur * CoC.x - CoC.y);

    vec4 sum = vec4_splat(0.0);

    for (int i = 0; i < NUM_TAPS; ++i)
    {
        // compute texture coordinates
        vec2 coordScene = oUv0 + (pixelSizeScene.zw * poisson[i] * discRadius);
        vec2 coordBlur = oUv0 + (pixelSizeBlur * poisson[i] * discRadius);

        // fetch taps and depth
        vec4 tapScene = texture2D(scene, coordScene);
        float tapDepth = texture2D(depth, coordScene).r;
        vec4 tapBlur = texture2D(blur, coordBlur);

        // mix low and high res. taps based on tap blurriness
        float blurAmount = getBlurAmount(tapDepth); // put blurriness into [0, 1]
        vec4 tap = mix(tapScene, tapBlur, blurAmount);

        // accumulate
        sum.rgb += tap.rgb * blurAmount;
        sum.a += blurAmount;
    }

    gl_FragColor = (sum / sum.a);
}
