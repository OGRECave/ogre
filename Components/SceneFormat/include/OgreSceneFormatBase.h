/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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
#ifndef _OgreSceneFormatBase_H_
#define _OgreSceneFormatBase_H_

#include "OgreSceneFormatPrerequisites.h"
#include "OgreLight.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    namespace SceneFlags
    {
        enum SceneFlags
        {
            SceneNodes              = 1u << 0u,
            Items                   = 1u << 1u,
            Entities                = 1u << 2u,
            Lights                  = 1u << 3u,
            Cameras                 = 1u << 4u,
            Materials               = 1u << 5u,
            Textures                = 1u << 6u,
            Meshes                  = 1u << 7u,
            MeshesV1                = 1u << 8u,
            SceneSettings           = 1u << 9u,
            InstantRadiosity        = 1u << 10u,
            /// Only used for importing. Has no effect if InstantRadiosity is not set.
            /// If this flag is present, InstantRadiosity will be build.
            BuildInstantRadiosity   = 1u << 11u,
            /// Warning: Importing w/ both BuildInstantRadiosity and LightsVpl can result
            /// in an incorrect scene (VPLs will exist twice).
            LightsVpl               = 1u << 12u,
        };
    }

    /**
    */
    class _OgreSceneFormatExport SceneFormatBase
    {
    protected:
        Root                    *mRoot;
        SceneManager            *mSceneManager;

        static const char* c_lightTypes[Light::NUM_LIGHT_TYPES+1u];

    public:
        SceneFormatBase( Root *root, SceneManager *sceneManager );
        ~SceneFormatBase();
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
