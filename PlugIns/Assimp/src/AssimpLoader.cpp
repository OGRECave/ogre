/*
-----------------------------------------------------------------------------
This source file is part of
                                    _
  ___   __ _ _ __ ___  __ _ ___ ___(_)_ __ ___  _ __
 / _ \ / _` | '__/ _ \/ _` / __/ __| | '_ ` _ \| '_ \
| (_) | (_| | | |  __/ (_| \__ \__ \ | | | | | | |_) |
 \___/ \__, |_|  \___|\__,_|___/___/_|_| |_| |_| .__/
       |___/                                   |_|

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
#include "OgreAssimpLoader.h"

#include <assimp/version.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultIOSystem.h>

#include <Ogre.h>

#include <OgreCodec.h>

namespace Ogre
{

namespace
{
struct OgreLogStream : public Assimp::LogStream
{
    LogMessageLevel _lml;
    OgreLogStream(LogMessageLevel lml) : _lml(lml) {}

    void write(const char* message)
    {
        String msg(message);
        StringUtil::trim(msg);
        LogManager::getSingleton().logMessage("Assimp: " + msg, _lml);
    }
};

struct OgreIOStream : public Assimp::IOStream
{
    DataStreamPtr stream;

    OgreIOStream(DataStreamPtr _stream) : stream(_stream) {}

    size_t Read(void* pvBuffer, size_t pSize, size_t pCount)
    {
        return stream->read(pvBuffer, pSize * pCount);
    }
    size_t Tell() const { return stream->tell(); }
    size_t FileSize() const { return stream->size(); }

    size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) { return 0; }
    aiReturn Seek(size_t pOffset, aiOrigin pOrigin)
    {
        if (pOrigin != aiOrigin_SET)
            return AI_FAILURE;

        stream->seek(pOffset);
        return AI_SUCCESS;
    }
    void Flush() {}
};

struct OgreIOSystem : public Assimp::IOSystem
{
    DataStreamPtr source;
    std::vector<OgreIOStream*> streams;
    String _group;

    OgreIOSystem(const DataStreamPtr& _source, const String& group) : source(_source), _group(group) {}

    bool Exists(const char* pFile) const
    {
        String file = StringUtil::normalizeFilePath(pFile, false);
        if (file == source->getName())
            return true;
        return ResourceGroupManager::getSingleton().resourceExists(_group, file);
    }
    char getOsSeparator() const override { return '/'; }

    Assimp::IOStream* Open(const char* pFile, const char* pMode) override
    {
        String file = StringUtil::normalizeFilePath(pFile, false);
        DataStreamPtr res;
        if (file == source->getName())
            res = source;

        if (!res)
            res = ResourceGroupManager::getSingleton().openResource(file, _group, NULL, false);

        if (res)
        {
            streams.emplace_back(new OgreIOStream(res));
            return streams.back();
        }
        return NULL;
    }
    void Close(Assimp::IOStream* pFile) override
    {
        auto it = std::find(streams.begin(), streams.end(), pFile);
        if (it != streams.end())
        {
            delete pFile;
            streams.erase(it);
        }
    }
};

/** translation, rotation, scale */
typedef std::tuple<aiVectorKey*, aiQuatKey*, aiVectorKey*> KeyframeData;
typedef std::map<Real, KeyframeData> KeyframesMap;

template <int _i>
void GetInterpolationIterators(KeyframesMap& keyframes, KeyframesMap::iterator it,
                               KeyframesMap::reverse_iterator& front, KeyframesMap::iterator& back)
{
    front = KeyframesMap::reverse_iterator(it);

    front++;
    for (; front != keyframes.rend(); front++)
    {
        if (std::get<_i>(front->second) != NULL)
        {
            break;
        }
    }

    back = it;
    back++;
    for (; back != keyframes.end(); back++)
    {
        if (std::get<_i>(back->second) != NULL)
        {
            break;
        }
    }
}

aiVector3D getTranslate(aiNodeAnim* node_anim, KeyframesMap& keyframes, KeyframesMap::iterator it,
                        Real ticksPerSecond)
{
    aiVectorKey* translateKey = std::get<0>(it->second);
    aiVector3D vect;
    if (translateKey)
    {
        vect = translateKey->mValue;
    }
    else
    {
        KeyframesMap::reverse_iterator front;
        KeyframesMap::iterator back;

        GetInterpolationIterators<0>(keyframes, it, front, back);

        KeyframesMap::reverse_iterator rend = keyframes.rend();
        KeyframesMap::iterator end = keyframes.end();
        aiVectorKey* frontKey = NULL;
        aiVectorKey* backKey = NULL;

        if (front != rend)
            frontKey = std::get<0>(front->second);

        if (back != end)
            backKey = std::get<0>(back->second);

        // got 2 keys can interpolate
        if (frontKey && backKey)
        {
            float prop =
                (float)(((double)it->first - frontKey->mTime) / (backKey->mTime - frontKey->mTime));
            prop /= ticksPerSecond;
            vect = Math::lerp(frontKey->mValue, backKey->mValue, prop);
        }

        else if (frontKey)
        {
            vect = frontKey->mValue;
        }
        else if (backKey)
        {
            vect = backKey->mValue;
        }
    }

    return vect;
}

aiQuaternion getRotate(aiNodeAnim* node_anim, KeyframesMap& keyframes, KeyframesMap::iterator it,
                       Real ticksPerSecond)
{
    aiQuatKey* rotationKey = std::get<1>(it->second);
    aiQuaternion rot;
    if (rotationKey)
    {
        rot = rotationKey->mValue;
    }
    else
    {
        KeyframesMap::reverse_iterator front;
        KeyframesMap::iterator back;

        GetInterpolationIterators<1>(keyframes, it, front, back);

        KeyframesMap::reverse_iterator rend = keyframes.rend();
        KeyframesMap::iterator end = keyframes.end();
        aiQuatKey* frontKey = NULL;
        aiQuatKey* backKey = NULL;

        if (front != rend)
            frontKey = std::get<1>(front->second);

        if (back != end)
            backKey = std::get<1>(back->second);

        // got 2 keys can interpolate
        if (frontKey && backKey)
        {
            float prop =
                (float)(((double)it->first - frontKey->mTime) / (backKey->mTime - frontKey->mTime));
            prop /= ticksPerSecond;
            aiQuaternion::Interpolate(rot, frontKey->mValue, backKey->mValue, prop);
        }

        else if (frontKey)
        {
            rot = frontKey->mValue;
        }
        else if (backKey)
        {
            rot = backKey->mValue;
        }
    }

    return rot;
}

aiVector3D getScale(aiNodeAnim* node_anim, KeyframesMap& keyframes, KeyframesMap::iterator it,
                    Real ticksPerSecond)
{
    aiVectorKey* scaleKey = std::get<2>(it->second);
    aiVector3D vect(1, 1, 1);
    if (scaleKey)
    {
        vect = scaleKey->mValue;
    }
    else
    {
        KeyframesMap::reverse_iterator front;
        KeyframesMap::iterator back;

        GetInterpolationIterators<2>(keyframes, it, front, back);

        KeyframesMap::reverse_iterator rend = keyframes.rend();
        KeyframesMap::iterator end = keyframes.end();
        aiVectorKey* frontKey = NULL;
        aiVectorKey* backKey = NULL;

        if (front != rend)
            frontKey = std::get<2>(front->second);

        if (back != end)
            backKey = std::get<2>(back->second);

        // got 2 keys can interpolate
        if (frontKey && backKey)
        {
            float prop =
                (float)(((double)it->first - frontKey->mTime) / (backKey->mTime - frontKey->mTime));
            prop /= ticksPerSecond;
            vect = Math::lerp(frontKey->mValue, backKey->mValue, prop);
        }

        else if (frontKey)
        {
            vect = frontKey->mValue;
        }
        else if (backKey)
        {
            vect = backKey->mValue;
        }
    }

    return vect;
}

String ReplaceSpaces(const String& s)
{
    String res(s);
    replace(res.begin(), res.end(), ' ', '_');

    return res;
}
} // namespace

int AssimpLoader::msBoneCount = 0;

AssimpLoader::AssimpLoader()
{
    Assimp::DefaultLogger::create("");
    Assimp::DefaultLogger::get()->attachStream(new OgreLogStream(LML_NORMAL), ~Assimp::DefaultLogger::Err);
    Assimp::DefaultLogger::get()->attachStream(new OgreLogStream(LML_CRITICAL), Assimp::DefaultLogger::Err);
}

AssimpLoader::~AssimpLoader() {}

bool AssimpLoader::load(const DataStreamPtr& source, Mesh* mesh, SkeletonPtr& skeletonPtr,
                        const Options& options)
{
    Assimp::Importer importer;
    importer.SetIOHandler(new OgreIOSystem(source, mesh->getGroup()));
    _load(source->getName().c_str(), importer, mesh, skeletonPtr, options);
    return true;
}

bool AssimpLoader::load(const String& source, Mesh* mesh, SkeletonPtr& skeletonPtr,
                        const AssimpLoader::Options& options)
{
    Assimp::Importer importer;
    _load(source.c_str(), importer, mesh, skeletonPtr, options);
    return true;
}

bool AssimpLoader::_load(const char* name, Assimp::Importer& importer, Mesh* mesh, SkeletonPtr& skeletonPtr,
                         const Options& options)
{
    uint32 flags = aiProcessPreset_TargetRealtime_Quality | aiProcess_TransformUVCoords | aiProcess_FlipUVs;
    importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", options.maxEdgeAngle);
    const aiScene* scene = importer.ReadFile(name, flags);

    // If the import failed, report it
    if (!scene)
    {
        LogManager::getSingleton().logError("Assimp failed - " + String(importer.GetErrorString()));
        return false;
    }

    mAnimationSpeedModifier = options.animationSpeedModifier;
    mLoaderParams = options.params;
    mQuietMode = ((mLoaderParams & LP_QUIET_MODE) == 0) ? false : true;
    mCustomAnimationName = options.customAnimationName;
    mNodeDerivedTransformByName.clear();

    String basename, extension;
    StringUtil::splitBaseFilename(mesh->getName(), basename, extension);

    grabNodeNamesFromNode(scene, scene->mRootNode);
    grabBoneNamesFromNode(scene, scene->mRootNode);

    computeNodesDerivedTransform(scene, scene->mRootNode, scene->mRootNode->mTransformation);

    if (mBonesByName.size())
    {
        mSkeleton = SkeletonManager::getSingleton().create(basename + ".skeleton", RGN_DEFAULT, true);

        msBoneCount = 0;
        createBonesFromNode(scene, scene->mRootNode);
        msBoneCount = 0;
        createBoneHiearchy(scene, scene->mRootNode);

        if (scene->HasAnimations())
        {
            for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
            {
                parseAnimation(scene, i, scene->mAnimations[i]);
            }
        }
    }

    loadDataFromNode(scene, scene->mRootNode, mesh);

    Assimp::DefaultLogger::kill();

    if (mSkeleton)
    {

        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage("Root bone: " + mSkeleton->getRootBones()[0]->getName());
        }

        skeletonPtr = mSkeleton;
        mesh->setSkeletonName(mSkeleton->getName());
    }

    for (auto sm : mesh->getSubMeshes())
    {
        if (!sm->useSharedVertices)
        {

            VertexDeclaration* newDcl = sm->vertexData->vertexDeclaration->getAutoOrganisedDeclaration(
                mesh->hasSkeleton(), mesh->hasVertexAnimation(), false);

            if (*newDcl != *(sm->vertexData->vertexDeclaration))
            {
                sm->vertexData->reorganiseBuffers(newDcl);
            }
        }
    }

    // clean up
    mBonesByName.clear();
    mBoneNodesByName.clear();
    boneMap.clear();
    mSkeleton.reset();

    mCustomAnimationName = "";

    return true;
}

void AssimpLoader::parseAnimation(const aiScene* mScene, int index, aiAnimation* anim)
{
    // DefBonePose a matrix that represents the local bone transform (can build from Ogre bone components)
    // PoseToKey a matrix representing the keyframe translation
    // What assimp stores aiNodeAnim IS the decomposed form of the transform (DefBonePose * PoseToKey)
    // To get PoseToKey which is what Ogre needs we'ed have to build the transform from components in
    // aiNodeAnim and then DefBonePose.Inverse() * aiNodeAnim(generated transform) will be the right
    // transform

    String animName;
    if (mCustomAnimationName != "")
    {
        animName = mCustomAnimationName;
        if (index >= 1)
        {
            animName += StringConverter::toString(index);
        }
    }
    else
    {
        animName = String(anim->mName.data);
    }
    if (animName.length() < 1)
    {
        animName = "Animation" + StringConverter::toString(index);
    }

    if (!mQuietMode)
    {
        LogManager::getSingleton().logMessage("Animation name = '" + animName + "'");
        LogManager::getSingleton().logMessage("duration = " +
                                              StringConverter::toString(Real(anim->mDuration)));
        LogManager::getSingleton().logMessage("tick/sec = " +
                                              StringConverter::toString(Real(anim->mTicksPerSecond)));
        LogManager::getSingleton().logMessage("channels = " +
                                              StringConverter::toString(anim->mNumChannels));
    }
    Animation* animation;
    mTicksPerSecond = (Real)((0 == anim->mTicksPerSecond) ? 24 : anim->mTicksPerSecond);
    mTicksPerSecond *= mAnimationSpeedModifier;

    Real cutTime = 0.0;
    if (mLoaderParams & LP_CUT_ANIMATION_WHERE_NO_FURTHER_CHANGE)
    {
        for (int i = 1; i < (int)anim->mNumChannels; i++)
        {
            aiNodeAnim* node_anim = anim->mChannels[i];

            // times of the equality check
            Real timePos = 0.0;
            Real timeRot = 0.0;

            for (unsigned int j = 1; j < node_anim->mNumPositionKeys; j++)
            {
                if (node_anim->mPositionKeys[j] != node_anim->mPositionKeys[j - 1])
                {
                    timePos = (Real)node_anim->mPositionKeys[j].mTime;
                    timePos /= mTicksPerSecond;
                }
            }

            for (unsigned int j = 1; j < node_anim->mNumRotationKeys; j++)
            {
                if (node_anim->mRotationKeys[j] != node_anim->mRotationKeys[j - 1])
                {
                    timeRot = (Real)node_anim->mRotationKeys[j].mTime;
                    timeRot /= mTicksPerSecond;
                }
            }

            if (timePos > cutTime)
            {
                cutTime = timePos;
            }
            if (timeRot > cutTime)
            {
                cutTime = timeRot;
            }
        }

        animation = mSkeleton->createAnimation(String(animName), cutTime);
    }
    else
    {
        cutTime = Math::POS_INFINITY;
        animation = mSkeleton->createAnimation(String(animName), Real(anim->mDuration / mTicksPerSecond));
    }

    animation->setInterpolationMode(Animation::IM_LINEAR); // FIXME: Is this always true?

    if (!mQuietMode)
    {
        LogManager::getSingleton().logMessage("Cut Time " + StringConverter::toString(cutTime));
    }

    for (int i = 0; i < (int)anim->mNumChannels; i++)
    {
        TransformKeyFrame* keyframe;

        aiNodeAnim* node_anim = anim->mChannels[i];
        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage("Channel " + StringConverter::toString(i));
            LogManager::getSingleton().logMessage("affecting node: " + String(node_anim->mNodeName.data));
            // LogManager::getSingleton().logMessage("position keys: " +
            // StringConverter::toString(node_anim->mNumPositionKeys));
            // LogManager::getSingleton().logMessage("rotation keys: " +
            // StringConverter::toString(node_anim->mNumRotationKeys));
            // LogManager::getSingleton().logMessage("scaling keys: " +
            // StringConverter::toString(node_anim->mNumScalingKeys));
        }

        String boneName = String(node_anim->mNodeName.data);

        if (mSkeleton->hasBone(boneName))
        {
            Bone* bone = mSkeleton->getBone(boneName);
            Affine3 defBonePoseInv;
            defBonePoseInv.makeInverseTransform(bone->getPosition(), bone->getScale(),
                                                bone->getOrientation());

            NodeAnimationTrack* track = animation->createNodeTrack(i, bone);

            // Ogre needs translate rotate and scale for each keyframe in the track
            KeyframesMap keyframes;

            for (unsigned int j = 0; j < node_anim->mNumPositionKeys; j++)
            {
                keyframes[(Real)node_anim->mPositionKeys[j].mTime / mTicksPerSecond] =
                    KeyframeData(&(node_anim->mPositionKeys[j]), NULL, NULL);
            }

            for (unsigned int j = 0; j < node_anim->mNumRotationKeys; j++)
            {
                KeyframesMap::iterator it =
                    keyframes.find((Real)node_anim->mRotationKeys[j].mTime / mTicksPerSecond);
                if (it != keyframes.end())
                {
                    std::get<1>(it->second) = &(node_anim->mRotationKeys[j]);
                }
                else
                {
                    keyframes[(Real)node_anim->mRotationKeys[j].mTime / mTicksPerSecond] =
                        KeyframeData(NULL, &(node_anim->mRotationKeys[j]), NULL);
                }
            }

            for (unsigned int j = 0; j < node_anim->mNumScalingKeys; j++)
            {
                KeyframesMap::iterator it =
                    keyframes.find((Real)node_anim->mScalingKeys[j].mTime / mTicksPerSecond);
                if (it != keyframes.end())
                {
                    std::get<2>(it->second) = &(node_anim->mScalingKeys[j]);
                }
                else
                {
                    keyframes[(Real)node_anim->mRotationKeys[j].mTime / mTicksPerSecond] =
                        KeyframeData(NULL, NULL, &(node_anim->mScalingKeys[j]));
                }
            }

            KeyframesMap::iterator it = keyframes.begin();
            KeyframesMap::iterator it_end = keyframes.end();
            for (; it != it_end; ++it)
            {
                if (it->first < cutTime) // or should it be <=
                {
                    aiVector3D aiTrans = getTranslate(node_anim, keyframes, it, mTicksPerSecond);

                    Vector3 trans(aiTrans.x, aiTrans.y, aiTrans.z);

                    aiQuaternion aiRot = getRotate(node_anim, keyframes, it, mTicksPerSecond);
                    Quaternion rot(aiRot.w, aiRot.x, aiRot.y, aiRot.z);

                    aiVector3D aiScale = getScale(node_anim, keyframes, it, mTicksPerSecond);
                    Vector3 scale(aiScale.x, aiScale.y, aiScale.z);

                    Vector3 transCopy = trans;

                    Affine3 fullTransform;
                    fullTransform.makeTransform(trans, scale, rot);

                    Affine3 poseTokey = defBonePoseInv * fullTransform;
                    poseTokey.decomposition(trans, scale, rot);

                    keyframe = track->createNodeKeyFrame(Real(it->first));

                    // weirdness with the root bone, But this seems to work
                    if (mSkeleton->getRootBones()[0]->getName() == boneName)
                    {
                        trans = transCopy - bone->getPosition();
                    }

                    keyframe->setTranslate(trans);
                    keyframe->setRotation(rot);
                    keyframe->setScale(scale);
                }
            }

        } // if bone exists

    } // loop through channels

    mSkeleton->optimiseAllAnimations();
}

void AssimpLoader::markAllChildNodesAsNeeded(const aiNode* pNode)
{
    flagNodeAsNeeded(pNode->mName.data);
    // Traverse all child nodes of the current node instance
    for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
    {
        const aiNode* pChildNode = pNode->mChildren[childIdx];
        markAllChildNodesAsNeeded(pChildNode);
    }
}

void AssimpLoader::grabNodeNamesFromNode(const aiScene* mScene, const aiNode* pNode)
{
    boneMap.emplace(String(pNode->mName.data), false);
    mBoneNodesByName[pNode->mName.data] = pNode;
    if (!mQuietMode)
    {
        LogManager::getSingleton().logMessage("Node " + String(pNode->mName.data) + " found.");
    }

    // Traverse all child nodes of the current node instance
    for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
    {
        const aiNode* pChildNode = pNode->mChildren[childIdx];
        grabNodeNamesFromNode(mScene, pChildNode);
    }
}

void AssimpLoader::computeNodesDerivedTransform(const aiScene* mScene, const aiNode* pNode,
                                                const aiMatrix4x4& accTransform)
{
    if (mNodeDerivedTransformByName.find(pNode->mName.data) == mNodeDerivedTransformByName.end())
    {
        mNodeDerivedTransformByName[pNode->mName.data] = Affine3(accTransform[0]);
    }
    for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
    {
        const aiNode* pChildNode = pNode->mChildren[childIdx];
        computeNodesDerivedTransform(mScene, pChildNode, accTransform * pChildNode->mTransformation);
    }
}

void AssimpLoader::createBonesFromNode(const aiScene* mScene, const aiNode* pNode)
{
    if (isNodeNeeded(pNode->mName.data))
    {
        Bone* bone = mSkeleton->createBone(String(pNode->mName.data), msBoneCount);

        aiQuaternion rot;
        aiVector3D pos;
        aiVector3D scale;

        const aiMatrix4x4& aiM = pNode->mTransformation;

        if (!aiM.IsIdentity())
        {
            aiM.Decompose(scale, rot, pos);
            bone->setPosition(pos.x, pos.y, pos.z);
            bone->setOrientation(rot.w, rot.x, rot.y, rot.z);
        }

        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage(StringConverter::toString(msBoneCount) +
                                                  ") Creating bone '" + String(pNode->mName.data) + "'");
        }
        msBoneCount++;
    }
    // Traverse all child nodes of the current node instance
    for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; ++childIdx)
    {
        const aiNode* pChildNode = pNode->mChildren[childIdx];
        createBonesFromNode(mScene, pChildNode);
    }
}

void AssimpLoader::createBoneHiearchy(const aiScene* mScene, const aiNode* pNode)
{
    if (isNodeNeeded(pNode->mName.data))
    {
        Bone* parent = 0;
        Bone* child = 0;
        if (pNode->mParent)
        {
            if (mSkeleton->hasBone(pNode->mParent->mName.data))
            {
                parent = mSkeleton->getBone(pNode->mParent->mName.data);
            }
        }
        if (mSkeleton->hasBone(pNode->mName.data))
        {
            child = mSkeleton->getBone(pNode->mName.data);
        }
        if (parent && child)
        {
            parent->addChild(child);
        }
    }
    // Traverse all child nodes of the current node instance
    for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; childIdx++)
    {
        const aiNode* pChildNode = pNode->mChildren[childIdx];
        createBoneHiearchy(mScene, pChildNode);
    }
}

void AssimpLoader::flagNodeAsNeeded(const char* name)
{
    boneMapType::iterator iter = boneMap.find(String(name));
    if (iter != boneMap.end())
    {
        iter->second = true;
    }
}

bool AssimpLoader::isNodeNeeded(const char* name)
{
    boneMapType::iterator iter = boneMap.find(String(name));
    if (iter != boneMap.end())
    {
        return iter->second;
    }
    return false;
}

void AssimpLoader::grabBoneNamesFromNode(const aiScene* mScene, const aiNode* pNode)
{
    static int meshNum = 0;
    meshNum++;
    if (pNode->mNumMeshes > 0)
    {
        for (unsigned int idx = 0; idx < pNode->mNumMeshes; ++idx)
        {
            aiMesh* pAIMesh = mScene->mMeshes[pNode->mMeshes[idx]];

            if (pAIMesh->HasBones())
            {
                for (uint32 i = 0; i < pAIMesh->mNumBones; ++i)
                {
                    aiBone* pAIBone = pAIMesh->mBones[i];
                    if (NULL != pAIBone)
                    {
                        mBonesByName[pAIBone->mName.data] = pAIBone;

                        if (!mQuietMode)
                        {
                            LogManager::getSingleton().logMessage(
                                StringConverter::toString(i) +
                                ") REAL BONE with name : " + String(pAIBone->mName.data));
                        }

                        // flag this node and all parents of this node as needed, until we reach the node
                        // holding the mesh, or the parent.
                        aiNode* node = mScene->mRootNode->FindNode(pAIBone->mName.data);
                        while (node)
                        {
                            if (node->mName.data == pNode->mName.data)
                            {
                                flagNodeAsNeeded(node->mName.data);
                                break;
                            }
                            if (node->mName.data == pNode->mParent->mName.data)
                            {
                                flagNodeAsNeeded(node->mName.data);
                                break;
                            }

                            // Not a root node, flag this as needed and continue to the parent
                            flagNodeAsNeeded(node->mName.data);
                            node = node->mParent;
                        }

                        // Flag all children of this node as needed
                        node = mScene->mRootNode->FindNode(pAIBone->mName.data);
                        markAllChildNodesAsNeeded(node);

                    } // if we have a valid bone
                }     // loop over bones
            }         // if this mesh has bones
        }             // loop over meshes
    }                 // if this node has meshes

    // Traverse all child nodes of the current node instance
    for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; childIdx++)
    {
        const aiNode* pChildNode = pNode->mChildren[childIdx];
        grabBoneNamesFromNode(mScene, pChildNode);
    }
}

MaterialPtr AssimpLoader::createMaterial(const aiMaterial* mat, const Ogre::String &group)
{
    static int dummyMatCount = 0;

    MaterialManager* omatMgr = MaterialManager::getSingletonPtr();
    enum aiTextureType type = aiTextureType_DIFFUSE;
    static aiString path;
    unsigned int uvindex = 0; // the texture uv index channel

    aiString szPath;
    if (AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_NAME, &szPath))
    {
        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage("Using aiGetMaterialString : Name " +
                                                  String(szPath.data));
        }
    }

    MaterialPtr omat;
    if(szPath.length)
    {
        auto status = omatMgr->createOrRetrieve(ReplaceSpaces(szPath.data), group);
        omat = static_pointer_cast<Material>(status.first);

        if (!status.second)
            return omat;

        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage("Creating " + String(szPath.data));
        }
    }
    else
    {
        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage("Unnamed material encountered...");
        }

        omat = omatMgr->getDefaultMaterial(false)->clone("dummyMat" +
                                                         StringConverter::toString(dummyMatCount++), group);
    }

    // ambient
    aiColor4D clr(1.0f, 1.0f, 1.0f, 1.0);
    // Ambient is usually way too low! FIX ME!
    if (mat->GetTexture(type, 0, &path) != AI_SUCCESS)
        aiGetMaterialColor(mat, AI_MATKEY_COLOR_AMBIENT, &clr);
    omat->setAmbient(clr.r, clr.g, clr.b);

    // diffuse
    clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &clr))
    {
        omat->setDiffuse(clr.r, clr.g, clr.b, clr.a);
    }

    // specular
    clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_SPECULAR, &clr))
    {
        omat->setSpecular(clr.r, clr.g, clr.b, clr.a);
    }

    // emissive
    clr = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
    if (AI_SUCCESS == aiGetMaterialColor(mat, AI_MATKEY_COLOR_EMISSIVE, &clr))
    {
        omat->setSelfIllumination(clr.r, clr.g, clr.b);
    }

    float fShininess;
    if (AI_SUCCESS == aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &fShininess))
    {
        omat->setShininess(Real(fShininess));
    }

    int shade = aiShadingMode_NoShading;
    if (AI_SUCCESS == mat->Get(AI_MATKEY_SHADING_MODEL, shade) && shade != aiShadingMode_NoShading)
    {
        switch (shade)
        {
        case aiShadingMode_Phong: // Phong shading mode was added to opengl and directx years ago to be
                                  // ready for gpus to support it (in fixed function pipeline), but no gpus
                                  // ever did, so it has never done anything. From directx 10 onwards it was
                                  // removed again.
        case aiShadingMode_Gouraud:
            omat->setShadingMode(SO_GOURAUD);
            break;
        case aiShadingMode_Flat:
            omat->setShadingMode(SO_FLAT);
            break;
        default:
            break;
        }
    }

    if (mat->GetTexture(type, 0, &path) == AI_SUCCESS)
    {
        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage("Found texture " + String(path.data) + " for channel " +
                                                  StringConverter::toString(uvindex));
        }
        if (AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_DIFFUSE(0), &szPath))
        {
            if (!mQuietMode)
            {
                LogManager::getSingleton().logMessage("Using aiGetMaterialString : Found texture " +
                                                      String(szPath.data) + " for channel " +
                                                      StringConverter::toString(uvindex));
            }
        }

        String basename;
        String outPath;
        StringUtil::splitFilename(String(szPath.data), basename, outPath);
        omat->getTechnique(0)->getPass(0)->createTextureUnitState(basename);

        // TODO: save embedded images to file
    }

    return omat;
}

bool AssimpLoader::createSubMesh(const String& name, int index, const aiNode* pNode, const aiMesh* mesh,
                                 const aiMaterial* mat, Mesh* mMesh, AxisAlignedBox& mAAB)
{
    // if animated all submeshes must have bone weights
    if (mBonesByName.size() && !mesh->HasBones())
    {
        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage("Skipping Mesh " + String(mesh->mName.data) +
                                                  "with no bone weights");
        }
        return false;
    }

    MaterialPtr matptr = createMaterial(mat, mMesh->getGroup());

    // now begin the object definition
    // We create a submesh per material
    SubMesh* submesh = mMesh->createSubMesh(name + StringConverter::toString(index));

    // prime pointers to vertex related data
    aiVector3D* vec = mesh->mVertices;
    aiVector3D* norm = mesh->mNormals;
    aiVector3D* uv = mesh->mTextureCoords[0];
    aiVector3D* tang = mesh->mTangents;
    aiColor4D *col = mesh->mColors[0];

    // We must create the vertex data, indicating how many vertices there will be
    submesh->useSharedVertices = false;
    submesh->vertexData = new VertexData();
    submesh->vertexData->vertexStart = 0;
    submesh->vertexData->vertexCount = mesh->mNumVertices;

    switch(mesh->mPrimitiveTypes)
    {
    default:
    case aiPrimitiveType_TRIANGLE:
        submesh->operationType = RenderOperation::OT_TRIANGLE_LIST;
        break;
    case aiPrimitiveType_LINE:
        submesh->operationType = RenderOperation::OT_LINE_LIST;
        break;
    case aiPrimitiveType_POINT:
        submesh->operationType = RenderOperation::OT_POINT_LIST;
        break;
    }

    // We must now declare what the vertex data contains
    VertexDeclaration* declaration = submesh->vertexData->vertexDeclaration;
    static const unsigned short source = 0;
    size_t offset = 0;
    offset += declaration->addElement(source, offset, VET_FLOAT3, VES_POSITION).getSize();

    if (!mQuietMode)
    {
        LogManager::getSingleton().logMessage(StringUtil::format("%d vertices", mesh->mNumVertices));
    }
    if (norm)
    {
        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage(StringUtil::format("%d normals", mesh->mNumVertices));
        }
        offset += declaration->addElement(source, offset, VET_FLOAT3, VES_NORMAL).getSize();
    }

    if (uv)
    {
        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage(StringUtil::format("%d uvs", mesh->mNumVertices));
        }
        offset += declaration->addElement(source, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES).getSize();
    }

    if (tang)
    {
        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage(StringUtil::format("%d tangents", mesh->mNumVertices));
        }
        offset += declaration->addElement(source, offset, VET_FLOAT3, VES_TANGENT).getSize();
    }

    if (col)
    {
        matptr->getTechnique(0)->getPass(0)->setVertexColourTracking(TVC_DIFFUSE);
        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage(StringUtil::format("%d colours", mesh->mNumVertices));
        }
        offset += declaration->addElement(source, offset, VET_UBYTE4_NORM, VES_DIFFUSE).getSize();
    }

    // Finally we set a material to the submesh
    submesh->setMaterial(matptr);

    // We create the hardware vertex buffer
    HardwareVertexBufferSharedPtr vbuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
        declaration->getVertexSize(source), // == offset
        submesh->vertexData->vertexCount,   // == nbVertices
        HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    Affine3 aiM = mNodeDerivedTransformByName.find(pNode->mName.data)->second;

    Matrix3 normalMatrix = aiM.linear().inverse().transpose();

    // Now we get access to the buffer to fill it.  During so we record the bounding box.
    float* vdata = static_cast<float*>(vbuffer->lock(HardwareBuffer::HBL_DISCARD));
    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        // Position
        Vector3 vect(vec->x, vec->y, vec->z);
        vect = aiM * vect;

        /*
        if(NULL != mSkeletonRootNode)
        {
            vect *= mSkeletonRootNode->mTransformation;
        }
        */

        *vdata++ = vect.x;
        *vdata++ = vect.y;
        *vdata++ = vect.z;
        mAAB.merge(vect);
        vec++;

        // Normal
        if (norm)
        {
            vect = normalMatrix * Vector3(norm->x, norm->y, norm->z);
            vect.normalise();

            *vdata++ = vect.x;
            *vdata++ = vect.y;
            *vdata++ = vect.z;
            norm++;
        }

        // uvs
        if (uv)
        {
            *vdata++ = uv->x;
            *vdata++ = uv->y;
            uv++;
        }

        if(tang)
        {
            *vdata++ = tang->x;
            *vdata++ = tang->y;
            *vdata++ = tang->z;
            tang++;
        }

        if (col)
        {
            PixelUtil::packColour(col->r, col->g, col->b, col->a, PF_BYTE_RGBA, vdata++);
            col++;
        }
    }

    vbuffer->unlock();
    submesh->vertexData->vertexBufferBinding->setBinding(source, vbuffer);

    // set bone weigths
    if (mesh->HasBones())
    {
        for (uint32 i = 0; i < mesh->mNumBones; i++)
        {
            aiBone* pAIBone = mesh->mBones[i];
            if (NULL != pAIBone)
            {
                String bname = pAIBone->mName.data;
                for (uint32 weightIdx = 0; weightIdx < pAIBone->mNumWeights; weightIdx++)
                {
                    aiVertexWeight aiWeight = pAIBone->mWeights[weightIdx];

                    VertexBoneAssignment vba;
                    vba.vertexIndex = aiWeight.mVertexId;
                    vba.boneIndex = mSkeleton->getBone(bname)->getHandle();
                    vba.weight = aiWeight.mWeight;

                    submesh->addBoneAssignment(vba);
                }
            }
        }
    } // if mesh has bones

    if(mesh->mNumFaces == 0)
        return true;

    if (!mQuietMode)
    {
        LogManager::getSingleton().logMessage(StringConverter::toString(mesh->mNumFaces) + " faces");
    }

    aiFace* faces = mesh->mFaces;
    int faceSz = mesh->mPrimitiveTypes == aiPrimitiveType_LINE ? 2 : 3;

    // Creates the index data
    submesh->indexData->indexStart = 0;
    submesh->indexData->indexCount = mesh->mNumFaces * faceSz;

    if (mesh->mNumVertices >= 65536) // 32 bit index buffer
    {
        submesh->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_32BIT, submesh->indexData->indexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        uint32* indexData =
            static_cast<uint32*>(submesh->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

        for (size_t i = 0; i < mesh->mNumFaces; ++i)
        {
            for(int j = 0; j < faceSz; j++)
                *indexData++ = faces->mIndices[j];

            faces++;
        }
    }
    else // 16 bit index buffer
    {
        submesh->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, submesh->indexData->indexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        uint16* indexData =
            static_cast<uint16*>(submesh->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

        for (size_t i = 0; i < mesh->mNumFaces; ++i)
        {
            for(int j = 0; j < faceSz; j++)
                *indexData++ = faces->mIndices[j];

            faces++;
        }
    }

    submesh->indexData->indexBuffer->unlock();

    return true;
}

void AssimpLoader::loadDataFromNode(const aiScene* mScene, const aiNode* pNode, Mesh* mesh)
{
    if (pNode->mNumMeshes > 0)
    {
        AxisAlignedBox mAAB = mesh->getBounds();

        for (unsigned int idx = 0; idx < pNode->mNumMeshes; ++idx)
        {
            aiMesh* pAIMesh = mScene->mMeshes[pNode->mMeshes[idx]];
            if (!mQuietMode)
            {
                LogManager::getSingleton().logMessage("SubMesh " + StringConverter::toString(idx) +
                                                      " for mesh '" + String(pNode->mName.data) + "'");
            }

            // Create a material instance for the mesh.
            const aiMaterial* pAIMaterial = mScene->mMaterials[pAIMesh->mMaterialIndex];
            createSubMesh(pNode->mName.data, idx, pNode, pAIMesh, pAIMaterial, mesh, mAAB);
        }

        // We must indicate the bounding box
        mesh->_setBounds(mAAB);
        mesh->_setBoundingSphereRadius((mAAB.getMaximum() - mAAB.getMinimum()).length() / 2);
    }

    // Traverse all child nodes of the current node instance
    for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; childIdx++)
    {
        const aiNode* pChildNode = pNode->mChildren[childIdx];
        loadDataFromNode(mScene, pChildNode, mesh);
    }
}

static std::vector<std::unique_ptr<Codec> > registeredCodecs;
struct AssimpCodec : public Codec
{
    String mType;

    AssimpCodec(const String& type) : mType(type) {}

    String magicNumberToFileExt(const char* magicNumberPtr, size_t maxbytes) const { return ""; }
    String getType() const { return mType; }
    void decode(const DataStreamPtr& input, const Any& output) const override
    {
        Mesh* dst = any_cast<Mesh*>(output);

        AssimpLoader::Options opts;
        opts.params = AssimpLoader::LP_QUIET_MODE;
        SkeletonPtr skeleton;
        AssimpLoader loader;
        loader.load(input, dst, skeleton, opts);
    }

    static void startup()
    {
        String version = StringUtil::format("Assimp - %d.%d.%d - Open-Asset-Importer", aiGetVersionMajor(),
                                            aiGetVersionMinor(), aiGetVersionRevision());
        LogManager::getSingleton().logMessage(version);

        String extensions;
        Assimp::Importer tmp;
        tmp.GetExtensionList(extensions);

        String blacklist[] = {"mesh", "mesh.xml", "raw", "mdc"};
        auto stream = LogManager::getSingleton().stream();
        stream << "Supported formats:";
        // Register codecs
        for (const auto& dotext : StringUtil::split(extensions, ";"))
        {
            String ext = dotext.substr(2);
            if (std::find(begin(blacklist), std::end(blacklist), ext) != std::end(blacklist))
                continue;

            stream << " " << ext;
            Codec* codec = new AssimpCodec(ext);
            registeredCodecs.push_back(std::unique_ptr<Codec>(codec));
            Codec::registerCodec(codec);
        }

        stream << "\n";
    }
    static void shutdown()
    {
        for (const auto& c : registeredCodecs)
            Codec::unregisterCodec(c.get());
        registeredCodecs.clear();
    }
};

const String& AssimpPlugin::getName() const
{
    static String name = "Assimp";
    return name;
}
void AssimpPlugin::install()
{
    AssimpCodec::startup();
}
void AssimpPlugin::uninstall()
{
    AssimpCodec::shutdown();
}
#ifndef OGRE_STATIC_LIB
extern "C" void _OgreAssimpExport dllStartPlugin();
extern "C" void _OgreAssimpExport dllStopPlugin();

extern "C" void _OgreAssimpExport dllStartPlugin()
{
    AssimpCodec::startup();
}
extern "C" void _OgreAssimpExport dllStopPlugin()
{
    AssimpCodec::shutdown();
}
#endif
} // namespace Ogre