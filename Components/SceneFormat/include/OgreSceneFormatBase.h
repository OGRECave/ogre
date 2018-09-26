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
#include "OgreHlmsJson.h"
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
            /// Always export the scene node even if its empty or its only attached
            /// objects are things we're not exporting (e.g. only lights are attached to a
            /// SceneNode but we're not exporting lights)
            /// See DefaultSceneFormatListener::exportSceneNode
            ForceAllSceneNodes      = 1u << 1u,
            Items                   = 1u << 2u,
            Entities                = 1u << 3u,
            Lights                  = 1u << 4u,
            Cameras                 = 1u << 5u,
            Materials               = 1u << 6u,
            /// See HlmsDatablock::saveTextures
            TexturesOitd            = 1u << 7u,
            /// See HlmsDatablock::saveTextures
            TexturesOriginal        = 1u << 8u,
            Meshes                  = 1u << 9u,
            MeshesV1                = 1u << 10u,
            SceneSettings           = 1u << 11u,
            InstantRadiosity        = 1u << 12u,
            /// Only used for importing. Has no effect if InstantRadiosity is not set.
            /// If this flag is present, InstantRadiosity will be build.
            BuildInstantRadiosity   = 1u << 13u,
            /// Warning: Importing w/ both BuildInstantRadiosity and LightsVpl can result
            /// in an incorrect scene (VPLs will exist twice).
            LightsVpl               = 1u << 14u,
            ParallaxCorrectedCubemap= 1u << 15u,
            AreaLightMasks          = 1u << 16u,
            Decals                  = 1u << 17u,
        };
    }

    class SceneFormatListener;
    class ParallaxCorrectedCubemap;
    class HlmsPbs;

    /**
    */
    class _OgreSceneFormatExport SceneFormatBase
    {
    public:
        enum Version
        {
            VERSION_0   = 0,
            VERSION_1,
            LATEST_VERSION = VERSION_1
        };
    protected:
        struct DecalTex
        {
            TexturePtr  texture;
            uint16      xIdx;
            const char  *texTypeName;
            DecalTex( const TexturePtr &_texture, uint16 _xIdx, const char *_texTypeName ) :
                texture( _texture ), xIdx( _xIdx ), texTypeName( _texTypeName ) {}
        };

        Root                    *mRoot;
        SceneManager            *mSceneManager;
        SceneFormatListener     *mListener;

        static const char* c_lightTypes[Light::NUM_LIGHT_TYPES+1u];

    public:
        SceneFormatBase( Root *root, SceneManager *sceneManager );
        ~SceneFormatBase();

        HlmsPbs* getPbs(void) const;

        /// Caller must delete the pointer. We won't do it for you.
        void setListener( SceneFormatListener *listener );
    };

    /** Override this listener if you want to filter which objects get exported.
        See DefaultSceneFormatListener
    */
    class _OgreSceneFormatExport SceneFormatListener :
            public HlmsJsonListener, public HlmsTextureExportListener
    {
    public:
        virtual void setSceneFlags( uint32 sceneFlags, SceneFormatBase *parent )    {}

        // Override any of these and return false if you don't want that particular object to be exported
        virtual bool exportSceneNode( const SceneNode *sceneNode )  { return true; }
        virtual bool exportMesh( const Mesh *mesh )                 { return true; }
        virtual bool exportMesh( const v1::Mesh *mesh )             { return true; }
        virtual bool exportItem( const Item *item )                 { return true; }
        virtual bool exportEntity( const v1::Entity *entity )       { return true; }
        virtual bool exportLight( const Light *light )              { return true; }
        virtual bool exportDecal( const Decal *decal )              { return true; }
    };

    /** Default implementation that prevents a SceneNode from being exported if
        the only attached objects
    */
    class _OgreSceneFormatExport DefaultSceneFormatListener : public SceneFormatListener
    {
        uint32 mSceneFlags;

        ParallaxCorrectedCubemap    *mParallaxCorrectedCubemap;

        bool hasNoAttachedObjectsOfType( const SceneNode *sceneNode );

    public:
        DefaultSceneFormatListener();

        virtual void setSceneFlags( uint32 sceneFlags, SceneFormatBase *parent );
        virtual bool exportSceneNode( const SceneNode *sceneNode );
        virtual bool exportItem( const Item *item );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
