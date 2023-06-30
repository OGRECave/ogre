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

#ifndef _LodPrerequisites_H__
#define _LodPrerequisites_H__

#include "OgrePrerequisites.h"

namespace Ogre
{
    // forward decls
    class LodCollapseCost;
    class LodCollapseCostCurvature;
    class LodCollapseCostOutside;
    class LodCollapseCostProfiler;

    class LodInputProvider;
    class LodInputProviderMesh;
    class LodInputProviderBuffer;
    class LodOutputProvider;
    class LodOutputProviderMesh;
    class LodOutputProviderBuffer;
    class LodOutsideMarker;


    class LodCollapser;
    struct LodConfig;
    struct LodLevel;
    class LodConfigSerializer;
    class MeshLodGenerator;
    class LodWorkQueueWorker;
    class LodWorkQueueInjector;
    struct LodWorkQueueRequest;
    class LodWorkQueueInjectorListener;
    struct LodData;

    typedef shared_ptr<LodCollapseCost> LodCollapseCostPtr;
    typedef shared_ptr<LodCollapser> LodCollapserPtr;
    typedef shared_ptr<LodData> LodDataPtr;
    typedef shared_ptr<LodInputProvider> LodInputProviderPtr;
    typedef shared_ptr<LodOutputProvider> LodOutputProviderPtr;

    typedef GeneralAllocatedObject MeshLodAlloc;
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WINRT
#   if defined( OGRE_STATIC_LIB )
#       define _OgreLodExport
#   else
#       if defined( OGRE_MESHLODGENERATOR_EXPORTS )
#           define _OgreLodExport __declspec( dllexport )
#       else
#           if defined( __MINGW32__ )
#               define _OgreLodExport
#           else
#               define _OgreLodExport __declspec( dllimport )
#           endif
#       endif
#   endif
#elif defined ( OGRE_GCC_VISIBILITY )
#   define _OgreLodExport __attribute__ ((visibility("default")))
#else
#   define _OgreLodExport
#endif 

#endif 
