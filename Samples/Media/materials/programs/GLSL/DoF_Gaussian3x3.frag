// "Depth of Field" demo for Ogre
// Copyright (C) 2006  Christian Lindequist Larsen
//
// This code is in the public domain. You may do whatever you want with it.

uniform vec4 vpSize;
uniform sampler2D source;

varying vec2 oUv0;

#define KERNEL_SIZE 9

float weights[KERNEL_SIZE];

vec2 offsets[KERNEL_SIZE];

void main()
{
    vec2 pixelSize = vpSize.zw;
    weights[0] = 1.0/16.0; weights[1] = 2.0/16.0; weights[2] = 1.0/16.0;
    weights[3] = 2.0/16.0; weights[4] = 4.0/16.0; weights[5] = 2.0/16.0;
    weights[6] = 1.0/16.0; weights[7] = 2.0/16.0; weights[8] = 1.0/16.0;

    offsets[0] = vec2(-pixelSize.x, -pixelSize.y);
    offsets[1] = vec2(0, -pixelSize.y);
    offsets[2] = vec2(pixelSize.x, -pixelSize.y);
    offsets[3] = vec2(-pixelSize.x, 0);
    offsets[4] = vec2(0, 0);
    offsets[5] = vec2(pixelSize.x, 0);
    offsets[6] = vec2(-pixelSize.x, pixelSize.y);
    offsets[7] = vec2(0,  pixelSize.y);
    offsets[8] = vec2(pixelSize.x, pixelSize.y);

    vec4 sum = vec4(0.0);

    for (int i = 0; i < KERNEL_SIZE; ++i)
        sum += weights[i] * texture2D(source, oUv0 + offsets[i]);

    gl_FragColor = sum;
}
