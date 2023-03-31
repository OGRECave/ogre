// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#define M_PI 3.14159265359

float getDistanceAttenuation(const vec3 params, float distance)
{
    return 1.0 / (params.x + params.y * distance + params.z * distance * distance);
}

float getAngleAttenuation(const vec3 params, const vec3 lightDir, const vec3 toLight)
{
    float rho		= dot(-lightDir, toLight);
    float fSpotE	= saturate((rho - params.y) / (params.x - params.y));
    return pow(fSpotE, params.z);
}