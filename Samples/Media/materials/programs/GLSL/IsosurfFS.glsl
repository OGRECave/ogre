#ifdef OGRE_GLSL
#version 120
#endif

// Ogre port of Nvidia's IsoSurf.cg file
// Modified code follows. See http://developer.download.nvidia.com/SDK/10/opengl/samples.html for original
//
// Cg port of Yury Uralsky's metaball FX shader
//
// Authors: Simon Green and Yury Urlasky
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <OgreUnifiedShader.h>

// Pixel shader
MAIN_PARAMETERS
IN(vec3 oNormal, TEXCOORD0)
MAIN_DECLARATION
{
    // Sanitize input
    vec3 N = normalize(oNormal);
    vec3 L = vec3(0, 0, 1);
    float nDotL = dot(N, L);

    vec3 materials[2];
    materials[0] = vec3(1, 1, 1);
    materials[1] = vec3(0, 0, 0.5);

    gl_FragColor = vec4(abs(nDotL) * materials[int(nDotL < 0.0)], 0.1);
}
