/*
-----------------------------------------------------------------------------
This source file is part of
                                    _
  ___   __ _ _ __ ___  __ _ ___ ___(_)_ __ ___  _ __
 / _ \ / _` | '__/ _ \/ _` / __/ __| | '_ ` _ \| '_ \
| (_) | (_| | | |  __/ (_| \__ \__ \ | | | | | | |_) |
 \___/ \__, |_|  \___|\__,_|___/___/_|_| |_| |_| .__/
       |___/                                   |_|

For the latest info, see https://bitbucket.org/jacmoe/ogreassimp

Copyright (c) 2011 Jacob 'jacmoe' Moen

Licensed under the MIT license:

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
#ifndef __AssimpLoader_h__
#define __AssimpLoader_h__

#include <OgreMesh.h>
#include <OgrePlugin.h>
#include <OgreAssimpExports.h>

struct aiScene;
struct aiNode;
struct aiBone;
struct aiMesh;
struct aiMaterial;
struct aiAnimation;

template <typename TReal> class aiMatrix4x4t;
typedef aiMatrix4x4t<float> aiMatrix4x4;

namespace Assimp
{
class Importer;
}

namespace Ogre
{

/** \addtogroup Plugins Plugins
*  @{
*/
/** \defgroup AssimpCodec AssimpCodec
* %Codec for loading geometry using the [Open-Asset-Importer](https://github.com/assimp/assimp)
* @{
*/
class _OgreAssimpExport AssimpLoader
{
public:
    enum LoaderParams
    {
        // 3ds max exports the animation over a longer time frame than the animation actually plays for
        // this is a fix for that
        LP_CUT_ANIMATION_WHERE_NO_FURTHER_CHANGE = 1 << 0,

        // Quiet mode - don't output anything
        LP_QUIET_MODE = 1 << 1
    };

    struct Options
    {
        float animationSpeedModifier;
        int params;
        String customAnimationName;
        float maxEdgeAngle;

        Options() : animationSpeedModifier(1), params(0), maxEdgeAngle(30) {}
    };

    AssimpLoader();
    virtual ~AssimpLoader();

    bool load(const String& source, Mesh* mesh, SkeletonPtr& skeletonPtr,
              const Options& options = Options());

    bool load(const DataStreamPtr& source, Mesh* mesh, SkeletonPtr& skeletonPtr,
              const Options& options = Options());

private:
    bool _load(const char* name, Assimp::Importer& importer, Mesh* mesh, SkeletonPtr& skeletonPtr,
               const Options& options);
    bool createSubMesh(const String& name, int index, const aiNode* pNode, const aiMesh* mesh,
                       const aiMaterial* mat, Mesh* mMesh, AxisAlignedBox& mAAB);
    MaterialPtr createMaterial(const aiMaterial* mat, const Ogre::String &group);
    void grabNodeNamesFromNode(const aiScene* mScene, const aiNode* pNode);
    void grabBoneNamesFromNode(const aiScene* mScene, const aiNode* pNode);
    void computeNodesDerivedTransform(const aiScene* mScene, const aiNode* pNode,
                                      const aiMatrix4x4& accTransform);
    void createBonesFromNode(const aiScene* mScene, const aiNode* pNode);
    void createBoneHiearchy(const aiScene* mScene, const aiNode* pNode);
    void loadDataFromNode(const aiScene* mScene, const aiNode* pNode, Mesh* mesh);
    void markAllChildNodesAsNeeded(const aiNode* pNode);
    void flagNodeAsNeeded(const char* name);
    bool isNodeNeeded(const char* name);
    void parseAnimation(const aiScene* mScene, int index, aiAnimation* anim);
    typedef std::map<String, bool> boneMapType;
    boneMapType boneMap;
    // aiNode* mSkeletonRootNode;
    int mLoaderParams;

    String mCustomAnimationName;

    typedef std::map<String, const aiNode*> BoneNodeMap;
    BoneNodeMap mBoneNodesByName;

    typedef std::map<String, const aiBone*> BoneMap;
    BoneMap mBonesByName;

    typedef std::map<String, Affine3> NodeTransformMap;
    NodeTransformMap mNodeDerivedTransformByName;

    SkeletonPtr mSkeleton;

    static int msBoneCount;

    bool mQuietMode;
    Real mTicksPerSecond;
    Real mAnimationSpeedModifier;
};

class AssimpPlugin : public Plugin
{
public:
    const String& getName() const;
    void install();
    void uninstall();
    void initialise() {}
    void shutdown() {}
};
/** @} */
/** @} */
} // namespace Ogre

#endif // __AssimpLoader_h__
