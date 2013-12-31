/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
