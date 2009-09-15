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

#include "OgreStableHeaders.h"
#include "OgreCommon.h"
#include "OgreNumerics.h"


namespace Ogre
{
	bool NumericSolver::solveNxNLinearSysDestr(int n, PreciseReal **coeff, PreciseReal *col)
	{
		// we'll use standard row reduction; since we only care about systems with unique
		// solutions, our job is slightly easier.  more can probably be done later to improve
		// precision versus this naive method

		int i, j;

		for(j=0; j<n; j++)
		{
			// look for a row with a leading coefficient we can use to cancel the rest
			int nonzeroIndex = -1;
			for(i=j; i<n; i++) {
				if (coeff[i][j] != 0.0) {
					nonzeroIndex = i;
					break;
				}
			}
			if (nonzeroIndex < 0) 
				return false;
			PreciseReal *tptr = coeff[j];
			coeff[j] = coeff[nonzeroIndex];
			coeff[nonzeroIndex] = tptr;
			PreciseReal tval = col[j];
			col[j] = col[nonzeroIndex];
			col[nonzeroIndex] = tval;
			nonzeroIndex = j;

			// normalize row to have leading coeff of 1 and kill other rows' entries
			PreciseReal invelt = 1.0 / coeff[nonzeroIndex][j];
			int k;
			for (k=j; k<n; k++)
				coeff[nonzeroIndex][k] *= invelt;
			col[nonzeroIndex] *= invelt;
			for (i=0; i<n; i++) {
				if (i==nonzeroIndex || coeff[i][j] == 0.0)
					continue;
				PreciseReal temp = coeff[i][j];
				for (k=j; k<n; k++)
					coeff[i][k] -= temp * coeff[nonzeroIndex][k];
				col[i] -= temp * col[nonzeroIndex];
			}
		}

		return true;
	}

}
