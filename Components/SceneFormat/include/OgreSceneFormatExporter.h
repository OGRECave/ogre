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
#ifndef _OgreSceneFormatExporter_H_
#define _OgreSceneFormatExporter_H_

#include "OgreSceneFormatBase.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    class LwString;
    class InstantRadiosity;

    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    /**
    */
    class _OgreSceneFormatExport SceneFormatExporter : public SceneFormatBase
    {
    protected:
        InstantRadiosity        *mInstantRadiosity;
        String                  mCurrentExportFolder;

        typedef map<Node*, uint32>::type NodeToIdxMap;
        NodeToIdxMap            mNodeToIdxMap;

        typedef set<const Mesh*>::type MeshSet;
        typedef set<const v1::Mesh*>::type MeshV1Set;
        MeshSet     mExportedMeshes;
        MeshV1Set   mExportedMeshesV1;

        static const char* toQuotedStr( bool value );
        static void toQuotedStr( LwString &jsonStr, Light::LightTypes lightType );

        static uint32 encodeFloat( float value );
        static uint64 encodeDouble( double value );
        static void encodeVector( LwString &jsonStr, const Vector2 &value );
        static void encodeVector( LwString &jsonStr, const Vector3 &value );
        static void encodeVector( LwString &jsonStr, const Vector4 &value );
        static void encodeQuaternion( LwString &jsonStr, const Quaternion &value );
        static void encodeColour( LwString &jsonStr, const ColourValue &value );
        static void encodeAabb( LwString &jsonStr, const Aabb &aabb );
        static void encodeMatrix( LwString &jsonStr, const Matrix3 &aabb );

        static inline void flushLwString( LwString &jsonStr, String &outJson );

        void exportNode( LwString &jsonStr, String &outJson, Node *node );
        void exportSceneNode( LwString &jsonStr, String &outJson, SceneNode *sceneNode );
        void exportRenderable( LwString &jsonStr, String &outJson, Renderable *renderable );
        void exportMovableObject( LwString &jsonStr, String &outJson, MovableObject *movableObject );
        void exportItem( LwString &jsonStr, String &outJson, Item *item, bool exportMesh );
        void exportLight( LwString &jsonStr, String &outJson, Light *light );
        void exportEntity( LwString &jsonStr, String &outJson, v1::Entity *entity, bool exportMesh );
        void exportInstantRadiosity( LwString &jsonStr, String &outJson );
        void exportPcc( LwString &jsonStr, String &outJson );
        void exportSceneSettings( LwString &jsonStr, String &outJson, uint32 exportFlags );

        /**
        @param outJson
        @param exportFlags
            Combination of SceneFlags::SceneFlags, to know what to export and what to exclude.
            Default to exporting everything.
            Note that excluding scene nodes can cause issues later during import.
        */
        void _exportScene( String &outJson, uint32 exportFlags=~0u );

    public:
        SceneFormatExporter( Root *root, SceneManager *sceneManager,
                             InstantRadiosity *instantRadiosity );
        ~SceneFormatExporter();

        /**
        @param outJson
        @param exportFlags
            Combination of SceneFlags::SceneFlags, to know what to export and what to exclude.
            Defaults to exporting everything.
            Note that excluding scene nodes can cause issues later during import.
        */
        void exportScene( String &outJson, uint32 exportFlags=~SceneFlags::TexturesOriginal );

        void exportSceneToFile( const String &folderPath,
                                uint32 exportFlags=~SceneFlags::TexturesOriginal );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
