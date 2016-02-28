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
OgrePCZSceneManagerDll.cpp  -  description
-----------------------------------------------------------------------------
begin                : Wed Feb 21 2007
author               : Eric Cha
email                : ericc@xenopi.com
-----------------------------------------------------------------------------
*/

#include "OgrePCZPrerequisites.h"
#include "OgreRoot.h"
#include "OgrePCZPlugin.h"

#ifndef OGRE_STATIC_LIB

namespace Ogre
{
    PCZPlugin* pczPlugin;

    extern "C" void _OgrePCZPluginExport dllStartPlugin( void )
    {
        // Create new scene manager
        pczPlugin = OGRE_NEW PCZPlugin();

        // Register
        Root::getSingleton().installPlugin(pczPlugin);

    }
    extern "C" void _OgrePCZPluginExport dllStopPlugin( void )
    {
        Root::getSingleton().uninstallPlugin(pczPlugin);
        OGRE_DELETE pczPlugin;
    }
}

#endif
