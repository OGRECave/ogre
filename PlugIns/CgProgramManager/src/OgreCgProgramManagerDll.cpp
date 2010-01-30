/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "OgreCgPlugin.h"
#include "OgreCgProgram.h"
#include "OgreRoot.h"

namespace Ogre {

    CgPlugin* cgPlugin;
#ifndef OGRE_STATIC_LIB
    //-----------------------------------------------------------------------
    extern "C" void _OgreCgPluginExport dllStartPlugin(void)
    {
        // Create new plugin
        cgPlugin = OGRE_NEW CgPlugin();

        // Register
        Root::getSingleton().installPlugin(cgPlugin);


    }
    extern "C" void _OgreCgPluginExport dllStopPlugin(void)
    {
		Root::getSingleton().uninstallPlugin(cgPlugin);
		OGRE_DELETE cgPlugin;
    }
#endif

    void checkForCgError(const String& ogreMethod, const String& errorTextPrefix, CGcontext context)
    {
        CGerror error = cgGetError();
        if (error != CG_NO_ERROR)
        {
            String msg = errorTextPrefix + cgGetErrorString(error); 

            if (error == CG_COMPILER_ERROR)
            {
                // Get listing with full compile errors
                msg = msg + "\n" + cgGetLastListing(context);
            }
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, msg, ogreMethod);
        }
    }

}
