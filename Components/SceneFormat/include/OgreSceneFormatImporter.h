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
    //template <typename, typename, typename> class GenericDocument;
    //typedef GenericDocument< UTF8<char>, MemoryPoolAllocator<CrtAllocator>, CrtAllocator > Document;

    template <typename BaseAllocator> class MemoryPoolAllocator;
    template <typename Encoding, typename>  class GenericValue;
    typedef GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator> > Value;
}

namespace Ogre
{
    class LwString;

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

        typedef map<uint32, SceneNode*>::type IndexToSceneNodeMap;
        IndexToSceneNodeMap mCreatedSceneNodes;

        static inline Light::LightTypes parseLightType( const char *value );
        static inline float decodeFloat( const rapidjson::Value &jsonValue );
        static inline Vector2 decodeVector2Array( const rapidjson::Value &jsonArray );
        static inline Vector3 decodeVector3Array( const rapidjson::Value &jsonArray );
        static inline Vector4 decodeVector4Array( const rapidjson::Value &jsonArray );
        static inline Quaternion decodeQuaternionArray( const rapidjson::Value &jsonArray );
        static inline ColourValue decodeColourValueArray( const rapidjson::Value &jsonArray );
        static inline Aabb decodeAabbArray( const rapidjson::Value &jsonArray,
                                            const Aabb &defaultValue );

        void importNode( const rapidjson::Value &nodeValue, Node *node );
        SceneNode* importSceneNode( const rapidjson::Value &sceneNodeValue, uint32 nodeIdx,
                                    const rapidjson::Value &sceneNodesJson );
        void importSceneNodes( const rapidjson::Value &json );
        void importMovableObject( const rapidjson::Value &movableObjectValue,
                                  MovableObject *movableObject );
        void importRenderable( const rapidjson::Value &renderableValue, Renderable *renderable );
        void importSubItem( const rapidjson::Value &subItemValue, SubItem *subItem );
        void importItem( const rapidjson::Value &itemValue );
        void importItems( const rapidjson::Value &json );
        void importLight( const rapidjson::Value &lightValue );
        void importLights( const rapidjson::Value &json );

    public:
        SceneFormatImporter( Root *root, SceneManager *sceneManager );
        ~SceneFormatImporter();

        void importScene( const String &filename, const char *jsonString );
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
