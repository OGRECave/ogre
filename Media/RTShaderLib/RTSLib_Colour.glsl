// This file is part of the OGRE project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at https://www.ogre3d.org/licensing.
// SPDX-License-Identifier: MIT

#ifdef USE_LINEAR_COLOURS
#define ENABLE_LINEAR_COLOUR(colour) colour.rgb = pow(colour.rgb, vec3_splat(2.2))
#else
#define ENABLE_LINEAR_COLOUR(colour)
#endif

#if defined(USE_LINEAR_COLOURS) && !defined(TARGET_CONSUMES_LINEAR)
#define COLOUR_TRANSFER(colour) colour.rgb = pow(colour.rgb, vec3_splat(1.0/2.2))
#else
#define COLOUR_TRANSFER(colour)
#endif