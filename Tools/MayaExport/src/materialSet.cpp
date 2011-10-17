////////////////////////////////////////////////////////////////////////////////
// materialSet.cpp
// Author     : Francesco Giordana
// Sponsored by : Anygma N.V. (http://www.nazooka.com)
// Start Date : January 13, 2005
// Copyright  : (C) 2006 by Francesco Giordana
// Email      : fra.giordana@tiscali.it
////////////////////////////////////////////////////////////////////////////////

/*********************************************************************************
*                                                                                *
*   This program is free software; you can redistribute it and/or modify         *
*   it under the terms of the GNU Lesser General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or            *
*   (at your option) any later version.                                          *
*                                                                                *
**********************************************************************************/

#include "materialSet.h"

namespace OgreMayaExporter
{
	template<> MaterialSet* Singleton<MaterialSet>::msSingleton = 0;
};	//end namespace