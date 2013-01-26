/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

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
#ifndef __Win32ConfigDialog_H__
#define __Win32ConfigDialog_H__

#include "../OgrePrerequisites.h"

#define WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // required to stop windows.h messing up std::min
#endif
#include "windows.h"

namespace Ogre
{
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/

	/** Defines the behaviour of an automatic renderer configuration dialog.
	@remarks
		OGRE comes with it's own renderer configuration dialog, which
		applications can use to easily allow the user to configure the
		settings appropriate to their machine. This class defines the
		interface to this standard dialog. Because dialogs are inherently
		tied to a particular platform's windowing system, there will be a
		different subclass for each platform.
	@author
		Steven J. Streeting
	*/
	class _OgreExport ConfigDialog : public UtilityAlloc
    {
    public:
        ConfigDialog();
		~ConfigDialog();
        
		/** Displays the dialog.
		@remarks
			This method displays the dialog and from then on the dialog
			interacts with the user independently. The dialog will be
			calling the relevant OGRE rendering systems to query them for
			options and to set the options the user selects. The method
			returns when the user closes the dialog.
		@returns
			If the user accepted the dialog, <b>true</b> is returned.
		@par
			If the user cancelled the dialog (indicating the application
			should probably terminate), <b>false</b> is returned.
		@see
			RenderSystem
		*/
        bool display();

    protected:
		RenderSystem* mSelectedRenderSystem;
	};
	/** @} */
	/** @} */
}
#endif
