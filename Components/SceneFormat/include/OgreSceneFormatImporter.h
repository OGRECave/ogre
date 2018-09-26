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
#ifndef _OgreSceneFormatImporter_H_
#define _OgreSceneFormatImporter_H_

#include "OgreSceneFormatBase.h"
#include "OgreHeaderPrefix.h"

// Forward declaration for |Document|.
namespace rapidjson
{
    class CrtAllocator;
    template <typename> class MemoryPoolAllocator;
    template <typename> struct UTF8;
    template <typename, typename, typename> class GenericDocument;
    typedef GenericDocument< UTF8<char>, MemoryPoolAllocator<CrtAllocator>, CrtAllocator > Document;

    template <typename BaseAllocator> class MemoryPoolAllocator;
    template <typename Encoding, typename>  class GenericValue;
    typedef GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator> > Value;
}

namespace Ogre
{
    class LwString;
    class InstantRadiosity;
    class IrradianceVolume;

    /** \addtogroup Component
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /**
    */
    class _OgreSceneFormatExport SceneFormatImporter : public SceneFormatBase
    {
    protected:
        String mFilename;
        InstantRadiosity *mInstantRadiosity;
        IrradianceVolume *mIrradianceVolume;
        ParallaxCorrectedCubemap *mParallaxCorrectedCubemap;
        Matrix4 mSceneComponentTransform;
        String  mDefaultPccWorkspaceName;

        bool mUseBinaryFloatingPoint;
        bool mUsingOitd;

        LightArray mVplLights;

        typedef map<uint32, SceneNode*>::type IndexToSceneNodeMap;
        IndexToSceneNodeMap mCreatedSceneNodes;

        SceneNode *mRootNodes[NUM_SCENE_MEMORY_MANAGER_TYPES];
        SceneNode *mParentlessRootNodes[NUM_SCENE_MEMORY_MANAGER_TYPES];

        void destroyInstantRadiosity(void);
        void destroyParallaxCorrectedCubemap(void);

        static inline Light::LightTypes parseLightType( const char *value );
        inline bool isFloat( const rapidjson::Value &jsonValue ) const;
        inline bool isDouble( const rapidjson::Value &jsonValue ) const;
        inline float decodeFloat( const rapidjson::Value &jsonValue );
        inline double decodeDouble( const rapidjson::Value &jsonValue );
        inline Vector2 decodeVector2Array( const rapidjson::Value &jsonArray );
        inline Vector3 decodeVector3Array( const rapidjson::Value &jsonArray );
        inline Vector4 decodeVector4Array( const rapidjson::Value &jsonArray );
        inline Quaternion decodeQuaternionArray( const rapidjson::Value &jsonArray );
        inline ColourValue decodeColourValueArray( const rapidjson::Value &jsonArray );
        inline Aabb decodeAabbArray( const rapidjson::Value &jsonArray,
                                            const Aabb &defaultValue );
        inline Matrix3 decodeMatrix3Array( const rapidjson::Value &jsonArray );

        void importNode( const rapidjson::Value &nodeValue, Node *node );
        SceneNode* importSceneNode( const rapidjson::Value &sceneNodeValue, uint32 nodeIdx,
                                    const rapidjson::Value &sceneNodesJson );
        void importSceneNodes( const rapidjson::Value &json );
        void importMovableObject( const rapidjson::Value &movableObjectValue,
                                  MovableObject *movableObject );
        void importRenderable( const rapidjson::Value &renderableValue, Renderable *renderable );
        void importSubItem( const rapidjson::Value &subItemValue, SubItem *subItem );
        void importSubEntity( const rapidjson::Value &subEntityValue, v1::SubEntity *subEntity );
        void importItem( const rapidjson::Value &itemValue );
        void importItems( const rapidjson::Value &json );
        void importEntity( const rapidjson::Value &entityValue );
        void importEntities( const rapidjson::Value &json );
        void importLight( const rapidjson::Value &lightValue );
        void importLights( const rapidjson::Value &json );
        void importDecal( const rapidjson::Value &decalValue );
        void importDecals( const rapidjson::Value &json );
        void importInstantRadiosity( const rapidjson::Value &irValue );
        void importPcc( const rapidjson::Value &pccValue );
        void importSceneSettings( const rapidjson::Value &json, uint32 importFlags );

        void importScene( const String &filename, const rapidjson::Document &d,
                          uint32 importFlags=~SceneFlags::LightsVpl );

    public:
        /**
        @param root
        @param sceneManager
        @param defaultPccWorkspaceName
            When importing PCC, the original workspace definition may not be available.
            In such case, this allows you to use a fallback instead. If left blank,
            we won't import PCC if the original workspace def couldn't be found.
        */
        SceneFormatImporter( Root *root, SceneManager *sceneManager,
                             const String &defaultPccWorkspaceName );
        ~SceneFormatImporter();

        /** Set the nodes that act as the root nodes for the scene to import.
            By default these are nullptrs, which means we'll be using the real
            root scenenodes from SceneManager (see SceneManager::getRootSceneNode)

            This function allows you to define your own root nodes; which gives
            you the power to easily transform the whole scene (e.g. globally
            displace the scene, rotate it, scale it, etc)
        @remarks
            The scene root nodes may be modified during the import process.
            Any transform set to these nodes before import may be lost.
            Make sure to apply them after importing.
        @see
            setParentlessRootNodes
        @param dynamicRoot
            SceneNode to use as Root for SCENE_DYNAMIC nodes.
            Leave nullptr for the default one.
        @param staticRoot
            SceneNode to use as Root for SCENE_STATIC nodes.
            Leave nullptr for the default one.
        */
        void setRootNodes( SceneNode *dynamicRoot, SceneNode *staticRoot );

        /** Similar to setRootNodes.
            During export, it's possible some nodes were not attached to anything; thus
            they were exported like that. They're parentless.
            By default, importing the scene with such nodes means these nodes will b
            created without a parent, like in the original.

            This behavior may not always be desired, which is why you can control it
            via this function, and have these nodes be attached to a parent node of
            your choosing instead.
        @remarks
            Unlike setRootNodes, these nodes won't be modified during the import process.
        @param dynamicRoot
            SceneNode to use as parent for SCENE_DYNAMIC parentless nodes.
            Leave nullptr for none.
        @param staticRoot
            SceneNode to use as parent for SCENE_STATIC parentless nodes.
            Leave nullptr for none.
        */
        void setParentlessRootNodes( SceneNode *dynamicRoot, SceneNode *staticRoot );

        /** Set a 4x4 matrix to apply a transformation to all the PCC probes during
            import process. Also affects Areas of Interest for Instant Radiosity.
            This is useful if you need to rotate or translate a scene.
        @remarks
            While any affine matrix will work; the best results are achieved if the matrix
            is orthogonal (e.g. +- 90° and +- 180° changes)
        @param transform
            Must be affine. Default is identity matrix.
        */
        void setSceneComponentTransform( const Matrix4 &transform );

        /**
        @param outJson
        @param exportFlags
            Combination of SceneFlags::SceneFlags, to know what to export and what to exclude.
            Defaults to importing everything.
            Note that some combinations can cause issues:
                * Excluding scene nodes
                * Excluding meshes without excluding Items and Entities.
                * etc

            By default LightsVpl is not set so that InstantRadiosity is regenerated.
            By setting LightsVpl and unsetting SceneFlags::BuildInstantRadiosity, you can speed up
            import time because the cached results will be loaded instead.
        */
        void importScene( const String &filename, const char *jsonString,
                          uint32 importFlags=~SceneFlags::LightsVpl );

        void importSceneFromFile( const String &filename, uint32 importFlags=~SceneFlags::LightsVpl );

        /** Retrieve the InstantRadiosity pointer that may have been created while importing a scene
        @param releaseOwnership
            If true, we will return the InstantRadiosity & IrradianceVolume pointers and
            release ownership. Meaning further calls to this function will return null and
            you will be responsible for deleting it (otherwise it will leak).

            If false, we will return the InstantRadiosity & IrradianceVolume pointers but
            retain ownership. Meaning further calls to this function will still return the pointer,
            and we will delete the pointer when 'this' is destroyed, or if a new scene is imported.
        @param outInstantRadiosity [out]
            InstantRadiosity pointer. Input cannot be null. Output *outInstantRadiosity may be null
            May be null if the imported scene didn't have IR enabled,
            or if the ownership has already been released.
        @param outIrradianceVolume [out]
            IrradianceVolume pointer. Input cannot be null. Output *outIrradianceVolume may be null.
            May be null if the imported scene didn't have IR/IV enabled,
            or if the ownership has already been released.
        @return
            InstantRadiosity pointer.
        */
        void getInstantRadiosity( bool releaseOwnership,
                                  InstantRadiosity **outInstantRadiosity,
                                  IrradianceVolume **outIrradianceVolume );

        ParallaxCorrectedCubemap* getParallaxCorrectedCubemap( bool releaseOwnership );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
