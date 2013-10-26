/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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

#ifndef __CompositorPassSceneDef_H__
#define __CompositorPassSceneDef_H__

#include "OgreHeaderPrefix.h"

#include "../OgreCompositorPassDef.h"

#include "OgreVisibilityFlags.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/

	enum ShadowNodeRecalculation
	{
		SHADOW_NODE_RECALCULATE,
		SHADOW_NODE_REUSE,
		SHADOW_NODE_FIRST_ONLY,
		SHADOW_NODE_CASTER_PASS		//Set automatically only when this pass is used by a ShadowNode
	};

	class _OgreExport CompositorPassSceneDef : public CompositorPassDef
	{
	public:
		/// Viewport's visibility mask while rendering our pass
		uint32					mVisibilityMask;
		IdString				mShadowNode;
		ShadowNodeRecalculation mShadowNodeRecalculation; //Only valid if mShadowNode is not empty
		IdString				mCameraName;

		/// First Render Queue ID to render. Inclusive
		uint8			mFirstRQ;
		/// Last Render Queue ID to render. Not inclusive
		uint8			mLastRQ;

		CompositorPassSceneDef() :
			CompositorPassDef( PASS_SCENE ),
			mVisibilityMask( VisibilityFlags::RESERVED_VISIBILITY_FLAGS ),
			mShadowNodeRecalculation( SHADOW_NODE_FIRST_ONLY ),
			mFirstRQ( 0 ),
			mLastRQ( -1 )
		{
			//Change base defaults
			mIncludeOverlays = true;
		}

		void setVisibilityMask( uint32 visibilityMask )
		{
			mVisibilityMask = visibilityMask & VisibilityFlags::RESERVED_VISIBILITY_FLAGS;
		}
	};

	/** @} */
	/** @} */
}

#include "OgreHeaderSuffix.h"

#endif
