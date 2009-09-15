/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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
-----------------------------------------------------------------------------
*/

#ifndef __NumericSolvers_H__
#define __NumericSolvers_H__

#include "OgrePrerequisites.h"


namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Math
	*  @{
	*/
	/** Real type for numeric solvers */
	typedef double PreciseReal;


    /** Provides numeric solvers for Ogre
        @remarks
            Handles linear algebra numerics.
    */
	class _OgreExport NumericSolver
	{
	public:
		/** Solves a full rank NxN linear system of equations 
		    @remarks
			   This implements a naive Gaussian elimination algorithm.
			   The algorithm is destructive, so there are side effects in coeff and col.
	    */
		static bool solveNxNLinearSysDestr(int n, PreciseReal **coeff, PreciseReal *col);
	};
	/** @} */
	/** @} */

}

#endif
