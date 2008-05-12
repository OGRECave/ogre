/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

/* Stable headers which will be used for precompilation if the compiler
   supports it. Add entries here when headers are unlikely to change.
   NB: a change to any of these headers will result in a full rebuild,
   so don't add things to this lightly.
*/

#ifndef __OgreStableHeaders__
#define __OgreStableHeaders__

#include "OgrePlatform.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
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
#include "OgreUserDefinedObject.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreWireBoundingBox.h"
#include "OgreZip.h"
#endif

#endif 
