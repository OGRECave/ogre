/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgreHlmsPropertyHelper.h"

namespace Ogre
{
	//Change per mesh (hash can be cached on the renderable)
	const IdString DefaultProperties::Skeleton = IdString("hlms_skeleton");
	const IdString DefaultProperties::BonesPerVertex = IdString("hlms_bones_per_vertex");
	const IdString DefaultProperties::Pose = IdString("hlms_pose");

	const IdString DefaultProperties::Normal = IdString("hlms_normal");
	const IdString DefaultProperties::QTangent = IdString("hlms_qtangent");
	const IdString DefaultProperties::Tangent = IdString("hlms_tangent");

	const IdString DefaultProperties::Colour = IdString("hlms_colour");

	const IdString DefaultProperties::UvCount = IdString("hlms_uv_count");
	const IdString DefaultProperties::UvCount0 = IdString("hlms_uv_count0");
	const IdString DefaultProperties::UvCount1 = IdString("hlms_uv_count1");
	const IdString DefaultProperties::UvCount2 = IdString("hlms_uv_count2");
	const IdString DefaultProperties::UvCount3 = IdString("hlms_uv_count3");
	const IdString DefaultProperties::UvCount4 = IdString("hlms_uv_count4");
	const IdString DefaultProperties::UvCount5 = IdString("hlms_uv_count5");
	const IdString DefaultProperties::UvCount6 = IdString("hlms_uv_count6");
	const IdString DefaultProperties::UvCount7 = IdString("hlms_uv_count7");

	//Change per frame (grouped together with scene pass)
	const IdString DefaultProperties::LightsDirectional = IdString("hlms_lights_directional");
	const IdString DefaultProperties::LightsPoint = IdString("hlms_lights_point");
	const IdString DefaultProperties::LightsSpot = IdString("hlms_lights_spot");
	const IdString DefaultProperties::LightsAttenuation = IdString("hlms_lights_attenuation");
	const IdString DefaultProperties::LightsSpotParams = IdString("hlms_lights_spotparams");

	//Change per scene pass
	const IdString DefaultProperties::DualParaboloidMapping = IdString("hlms_dual_paraboloid_mapping");
	const IdString DefaultProperties::NumShadowMaps = IdString("hlms_num_shadow_maps");
	const IdString DefaultProperties::PssmSplits = IdString("hlms_pssm_splits");
	const IdString DefaultProperties::ShadowCaster = IdString("hlms_shadowcaster");

	//Change per material (hash can be cached on the renderable)
	const IdString DefaultProperties::AlphaTest = IdString("alpha_test");

	const IdString *DefaultProperties::UvCountPtrs[8] =
	{
		&DefaultProperties::UvCount0,
		&DefaultProperties::UvCount1,
		&DefaultProperties::UvCount2,
		&DefaultProperties::UvCount3,
		&DefaultProperties::UvCount4,
		&DefaultProperties::UvCount5,
		&DefaultProperties::UvCount6,
		&DefaultProperties::UvCount7
	};
}

