// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

float weight(float z, float a)
{
    // from https://casual-effects.blogspot.com/2015/03/implemented-weighted-blended-order.html
    return clamp(pow(min(1.0, a * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - z * 0.9, 3.0), 1e-2, 3e3);
}

void SGX_WBOIT(float depth, inout vec4 accum, out vec4 revealage)
{
    vec4 colour = accum;
    // Weighted Blended Order-Independent Transparency, Listing 4
    float w = weight(depth, colour.a);
    accum = vec4(colour.rgb * w * colour.a, colour.a);
    revealage = vec4_splat(colour.a * w);
}