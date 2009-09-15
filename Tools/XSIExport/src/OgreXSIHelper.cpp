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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreXSIHelper.h"
#include <fstream>

namespace Ogre {

	//-----------------------------------------------------------------------
	void copyFile(const String& src, const String& dst)
	{
		std::ifstream in(src.c_str(), std::ios::in | std::ios::binary);
		std::ofstream out(dst.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

		if (!in || !out)
		{
			LogOgreAndXSI("Unable to copy texture '" + src + "' to '" + dst + "'");
			return;
		}

		char tmpBuf[2048];

		while (!in.eof())
		{
			in.read(tmpBuf, 2048);

			std::streamsize c = in.gcount();

			out.write(tmpBuf, c);

		}

		in.close();
		out.close();



	}
	//-----------------------------------------------------------------------
	template<> ProgressManager* Singleton<ProgressManager>::ms_Singleton = 0;
	ProgressManager* ProgressManager::getSingletonPtr(void)
	{
		return ms_Singleton;
	}
	ProgressManager& ProgressManager::getSingleton(void)
	{  
		assert( ms_Singleton );  return ( *ms_Singleton );  
	}
	//-----------------------------------------------------------------------------
	ProgressManager::ProgressManager(size_t numberOfStages)
		:mNumberOfStages(numberOfStages), mProgress(0)
	{
		XSI::Application app;

		mProgressBar = app.GetUIToolkit().GetProgressBar();
		mProgressBar.PutMaximum(numberOfStages);
		mProgressBar.PutStep(1);
		mProgressBar.PutVisible(true);
		mProgressBar.PutCaption(L"Exporting");
	}
	//-----------------------------------------------------------------------------
	ProgressManager::~ProgressManager()
	{
	}
	//-----------------------------------------------------------------------------
	void ProgressManager::progress(void)
	{
		++mProgress;
		mProgressBar.Increment();
		
	}

}