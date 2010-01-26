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

/* Stable headers which will be used for precompilation if the compiler
   supports it. Add entries here when headers are unlikely to change.
   NB: a change to any of these headers will result in a full rebuild,
   so don't add things to this lightly.
*/

#ifndef __OgreStableHeaders__
#define __OgreStableHeaders__

#include "OgrePlatform.h"

#if (OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && !defined(__MINGW32__)) || OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
#include "OgreArchive.h"
#include "OgreAxisAlignedBox.h"
#include "OgreBitwise.h"
#include "OgreCamera.h"
#include "OgreColourValue.h"
#include "OgreCommon.h"
#include "OgreDataStream.h"
#include "OgreException.h"
#include "OgreFileSystem.h"
#include "OgreLog.h"
#include "OgreMath.h"
#include "OgreMatrix3.h"
#include "OgreMatrix4.h"
#include "OgreMovableObject.h"
#include "OgreNode.h"
#include "OgrePlane.h"
#include "OgrePrerequisites.h"
#include "OgreQuaternion.h"
#include "OgreResource.h"
#include "OgreSerializer.h"
#include "OgreSharedPtr.h"
#include "OgreSimpleRenderable.h"
#include "OgreSimpleSpline.h"
#include "OgreSingleton.h"
#include "OgreSphere.h"
#include "OgreStdHeaders.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreStringInterface.h"
#include "OgreStringVector.h"
#include "OgreUserObjectBindings.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreWireBoundingBox.h"
#if OGRE_NO_ZIP_ARCHIVE == 0
#	include "OgreZip.h"
#endif

#endif

#endif 
