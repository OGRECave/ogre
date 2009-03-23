/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef _HardwareOcclusionQuery__
#define _HardwareOcclusionQuery__

// Precompiler options
#include "OgrePrerequisites.h"

namespace Ogre {



	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup RenderSystem
	*  @{
	*/
/**
  * This is a abstract class that that provides the interface for the query class for 
  * hardware occlusion.
  *
  * @author Lee Sandberg
  * Updated on 13/8/2005 by Tuan Kuranes email: tuan.kuranes@free.fr
  */
	class _OgreExport HardwareOcclusionQuery : public RenderSysAlloc
{
//----------------------------------------------------------------------
// Public methods
//--
public:
	/**
	  * Object public member functions
	  */

	/**
	  * Default object constructor
	  * 
	  */
	HardwareOcclusionQuery();

	/**
	  * Object destructor
	  */
	virtual ~HardwareOcclusionQuery();

	/**
	  * Starts the hardware occlusion query
	  * @Remarks	Simple usage: Create one or more OcclusionQuery object one per outstanding query or one per tested object 
	  *				OcclusionQuery* m_pOcclusionQuery;
	  *				createOcclusionQuery( &m_pOcclusionQuery );
	  *				In the rendering loop:
	  *				Draw all occluders
	  *				m_pOcclusionQuery->startOcclusionQuery();
	  *				Draw the polygons to be tested
	  *				m_pOcclusionQuery->endOcclusionQuery();
	  *
	  *				Results must be pulled using:
	  *				UINT	m_uintNumberOfPixelsVisable;
	  *				pullOcclusionQuery( &m_dwNumberOfPixelsVisable );
	  *			
	  */
	virtual void beginOcclusionQuery() = 0;

	/**
	  * Ends the hardware occlusion test
	  */
	virtual void endOcclusionQuery() = 0;

	/**
      * Pulls the hardware occlusion query.
	  * @note Waits until the query result is available; use isStillOutstanding
	  *		if just want to test if the result is available.
      * @retval NumOfFragments will get the resulting number of fragments.
      * @return True if success or false if not.
      */
	virtual bool pullOcclusionQuery(unsigned int* NumOfFragments) = 0;

	/**
	  * Let's you get the last pixel count with out doing the hardware occlusion test
	  * @return The last fragment count from the last test.
	  * Remarks This function won't give you new values, just the old value.
	  */
	unsigned int getLastQuerysPixelcount() const { return mPixelCount; }

	/**
	  * Lets you know when query is done, or still be processed by the Hardware
	  * @return true if query isn't finished.
	  */
	 virtual bool isStillOutstanding(void) = 0; 


    //----------------------------------------------------------------------
    // protected members
    //--
    protected :
        // numbers of visible pixels determined by last query
        unsigned int mPixelCount;
        // is query hasn't yet returned a result.
		bool		 mIsQueryResultStillOutstanding;
};

	/** @} */
	/** @} */
}
#endif

