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

        bool mUseBinaryFloatingPoint;
        uint8 mCurrentBinFloat;
        uint8 mCurrentBinDouble;
        char mFloatBinTmpString[24][64];
        char mDoubleBinTmpString[4][384];

        String      mDecalsTexNames[3];
        TexturePtr  mDecalsTex[3];
        bool        mDecalsTexManaged[3];

        static const char* toQuotedStr( bool value );
        static void toQuotedStr( LwString &jsonStr, Light::LightTypes lightType );

        static uint32 encodeFloatBin( float value );
        static uint64 encodeDoubleBin( double value );
        /// Warning: not thread safe. Returned pointer gets modified with next call
        /// encodeFloat works by using a pool of temporary strings (mFloatBinTmpString)
        /// to store the encoded value either as uint or float. We need a pool because
        /// if the user calls: someFunc( encodeFloat(x), encodeFloat(x), encodeFloat(x) );
        /// we need all three strings to remain valid when someFunc is executed, otherwise
        /// the successive calls to encodeFloat overwrite the previous returned value
        /// while it may still be used.
        const char* encodeFloat( float value );
        /// Warning: not thread safe. Returned pointer gets modified with next call
        const char* encodeDouble( double value );
        inline void rewindFloatBinStringPool( uint8 rewindAmount );
        void encodeVector( LwString &jsonStr, const Vector2 &value );
        void encodeVector( LwString &jsonStr, const Vector3 &value );
        void encodeVector( LwString &jsonStr, const Vector4 &value );
        void encodeQuaternion( LwString &jsonStr, const Quaternion &value );
        void encodeColour( LwString &jsonStr, const ColourValue &value );
        void encodeAabb( LwString &jsonStr, const Aabb &aabb );
        void encodeMatrix( LwString &jsonStr, const Matrix3 &aabb );

        static inline void flushLwString( LwString &jsonStr, String &outJson );

        void exportNode( LwString &jsonStr, String &outJson, Node *node );
        void exportSceneNode( LwString &jsonStr, String &outJson, SceneNode *sceneNode );
        void exportRenderable( LwString &jsonStr, String &outJson, Renderable *renderable );
        void exportMovableObject( LwString &jsonStr, String &outJson, MovableObject *movableObject );
        void exportItem( LwString &jsonStr, String &outJson, Item *item, bool exportMesh );
        void exportLight( LwString &jsonStr, String &outJson, Light *light );
        void exportEntity( LwString &jsonStr, String &outJson, v1::Entity *entity, bool exportMesh );
        void exportDecalTex( LwString &jsonStr, String &outJson, const DecalTex &decalTex,
                             set<String>::type &savedTextures, uint32 exportFlags, int texTypeIndex );
        void exportDecal( LwString &jsonStr, String &outJson, Decal *decal,
                          set<String>::type &savedTextures, uint32 exportFlags );
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
        void _exportScene( String &outJson, set<String>::type &savedTextures,
                           uint32 exportFlags=~0u );

    public:
        SceneFormatExporter( Root *root, SceneManager *sceneManager,
                             InstantRadiosity *instantRadiosity );
        ~SceneFormatExporter();

        /** By default we export floating point values as uint32 using their binary encoding.
            This allows us to preserve the identical value as text when importing. Otherwise
            very small errors could causes inconsistencies between the imported and the original
            scene.
            For example the number "1.0f" gets exported as 1065353216 (0x3f800000)
            The only problem with this is that the value we generate is not really user readable
            or editable (unless you aid yourself with some binary converter tool)

            In many cases this exact bit preservation isn't needed, and you can opt instead for
            us to export floating point as literal numbers, thus we'll just write "1.0" which
            is easy to read and easy to edit.
            Note however, not all floating point numbers can be accurately be represented as
            strings and then back as floats, there can be small rounding errors (i.e.
            NaNs, irrational numbers & numbers with repeating decimals)
        @param useBinaryFp
            True to export preserving exact binary representation.
            False to export as a user-friendly number
            Default: true.
        */
        void setUseBinaryFloatingPoint( bool useBinaryFp );
        bool getUseBinaryFloatingPoint(void);

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
