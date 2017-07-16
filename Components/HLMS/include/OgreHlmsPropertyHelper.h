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

#pragma once

#include "OgreIdString.h" 
#include "OgreHlmsPrerequisites.h"

namespace Ogre
{
	/** \addtogroup Optional
	*  @{
	*/
	/** \addtogroup Hlms
	*  @{
	*/
	class _OgreHlmsExport DefaultProperties
	{
	public:
		/// These are "default" or "Base" properties common to many implementations and thus defined here.
		static const IdString Skeleton;
		static const IdString BonesPerVertex;
		static const IdString Pose;

		static const IdString Normal;
		static const IdString QTangent;
		static const IdString Tangent;

		static const IdString Colour;

		static const IdString UvCount;
		static const IdString UvCount0;
		static const IdString UvCount1;
		static const IdString UvCount2;
		static const IdString UvCount3;
		static const IdString UvCount4;
		static const IdString UvCount5;
		static const IdString UvCount6;
		static const IdString UvCount7;

		//Change per frame (grouped together with scene pass)
		static const IdString LightsDirectional;
		static const IdString LightsPoint;
		static const IdString LightsSpot;
		static const IdString LightsAttenuation;
		static const IdString LightsSpotParams;

		//Change per scene pass
		static const IdString DualParaboloidMapping;
		static const IdString NumShadowMaps;
		static const IdString PssmSplits;
		static const IdString ShadowCaster;

		//Change per material (hash can be cached on the renderable)
		static const IdString AlphaTest;

		static const IdString *UvCountPtrs[8];
	};
}

