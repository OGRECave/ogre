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
#include "OgreString.h"
#include "OgreStringConverter.h"

#include <assimp/version.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultIOSystem.h>
#include <assimp/GltfMaterial.h>

#include <Ogre.h>

#include <OgreCodec.h>

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
#include <OgreShaderGenerator.h>
#endif

namespace Ogre
{

namespace
{
struct OgreLogStream : public Assimp::LogStream
{
    LogMessageLevel _lml;
    OgreLogStream(LogMessageLevel lml) : _lml(lml) {}

    void write(const char* message) override
    {
        String msg(message);
        StringUtil::trim(msg);
        LogManager::getSingleton().logMessage("Assimp: " + msg, _lml);
    }
};

struct OgreIOStream : public Assimp::IOStream
{
    DataStreamPtr stream;

    OgreIOStream(const DataStreamPtr& _stream) : stream(_stream) {}

    size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override
    {
        size_t bytes = stream->read(pvBuffer, pSize * pCount);
        return bytes / pSize;
    }
    size_t Tell() const override { return stream->tell(); }
    size_t FileSize() const override { return stream->size(); }

    size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override { return 0; }
    aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override
    {
        if (pOrigin != aiOrigin_SET)
            return AI_FAILURE;

        stream->seek(pOffset);
        return AI_SUCCESS;
    }
    void Flush() override {}
};

struct OgreIOSystem : public Assimp::IOSystem
{
    DataStreamPtr source;
    std::vector<OgreIOStream*> streams;
    String _group;

    OgreIOSystem(const DataStreamPtr& _source, const String& group) : source(_source), _group(group) {}

    bool Exists(const char* pFile) const override
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
        {
            res = source;
            // glTF2 importer wants to open files multiple times
            // while this is not fully correct, this makes it happy
            source->seek(0);
        }
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
    // reverse_iterator(it) already refers to the element BEFORE it, so the
    // search starts there: advancing first would skip the immediate
    // predecessor, and a channel with a single leading key (a constant
    // glTF track) would then find no key at all - the caller falls back
    // to zero, which reads as the inverse bind pose leaking into the track
    front = KeyframesMap::reverse_iterator(it);
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
            // key times are in TICKS while the merged map's times are in
            // SECONDS: convert the lookup time to ticks for the lerp factor
            // (the old form also divided the finished factor by ticksPerSecond
            // again, shrinking it to ~0 - interpolated keys held the front key)
            float prop = (float)(Math::inverseLerp<double>(frontKey->mTime, backKey->mTime, it->first * ticksPerSecond));
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
            // key times are in TICKS while the merged map's times are in
            // SECONDS: convert the lookup time to ticks for the lerp factor
            // (the old form also divided the finished factor by ticksPerSecond
            // again, shrinking it to ~0 - interpolated keys held the front key)
            float prop = (float)(Math::inverseLerp<double>(frontKey->mTime, backKey->mTime, it->first * ticksPerSecond));
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
            // key times are in TICKS while the merged map's times are in
            // SECONDS: convert the lookup time to ticks for the lerp factor
            // (the old form also divided the finished factor by ticksPerSecond
            // again, shrinking it to ~0 - interpolated keys held the front key)
            float prop = (float)(Math::inverseLerp<double>(frontKey->mTime, backKey->mTime, it->first * ticksPerSecond));
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

/// component-wise division, leaving the numerator untouched on a zero denominator
Vector3 componentwiseDivide(const Vector3& num, const Vector3& den)
{
    return Vector3(den.x != 0 ? num.x / den.x : num.x,
                   den.y != 0 ? num.y / den.y : num.y,
                   den.z != 0 ? num.z / den.z : num.z);
}
} // namespace

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
    mAnimationSpeedModifier = options.animationSpeedModifier;
    mLoaderParams = options.params;
    mCustomAnimationName = options.customAnimationName;

    float maxEdgeAngle = options.maxEdgeAngle;
    int postProcessSteps = options.postProcessSteps;
    auto optsAny = mesh->getUserObjectBindings().getUserAny("_AssimpLoaderOptions");
    if(optsAny.has_value())
    {
        auto strOpts = any_cast<BinaryOptionList>(optsAny);
        if(strOpts.find("quiet") != strOpts.end())
            mLoaderParams |= LP_QUIET_MODE;
        if(strOpts.find("customAnimationName") != strOpts.end())
            mCustomAnimationName = strOpts["customAnimationName"];
        if(strOpts.find("animationSpeedModifier") != strOpts.end())
            StringConverter::parse(strOpts["animationSpeedModifier"], mAnimationSpeedModifier);
        if(strOpts.find("maxEdgeAngle") != strOpts.end())
            StringConverter::parse(strOpts["maxEdgeAngle"], maxEdgeAngle);
        if(strOpts.find("postProcessSteps") != strOpts.end())
            StringConverter::parse(strOpts["postProcessSteps"], postProcessSteps);
        if(strOpts.find("cutAnimation") != strOpts.end())
            mLoaderParams |= LP_CUT_ANIMATION_WHERE_NO_FURTHER_CHANGE;
    }

    uint32 flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_TransformUVCoords | aiProcess_FlipUVs;
    flags &= ~(aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace); // optimize for fast loading

    flags |= postProcessSteps;

    if((flags & (aiProcess_GenSmoothNormals | aiProcess_GenNormals)) != aiProcess_GenNormals)
        flags &= ~aiProcess_GenNormals; // prefer smooth normals

    importer.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", maxEdgeAngle);
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_EMBEDDED_TEXTURES_LEGACY_NAMING, true);

    const aiScene* scene = importer.ReadFile(name, flags);

    // If the import failed, report it
    if (!scene)
    {
        LogManager::getSingleton().logError("Assimp failed - " + String(importer.GetErrorString()));
        return false;
    }

    mQuietMode = mLoaderParams & LP_QUIET_MODE;
    mNodeDerivedTransformByName.clear();
    mBoneBindWorldByName.clear();
    mBoneParentSpaceByName.clear();

    String basename, extension;
    StringUtil::splitBaseFilename(mesh->getName(), basename, extension);

    grabNodeNamesFromNode(scene, scene->mRootNode);
    computeNodesDerivedTransform(scene, scene->mRootNode, scene->mRootNode->mTransformation);
    grabBoneNamesFromNode(scene, scene->mRootNode);

    if (mBonesByName.size())
    {
        mSkeleton = SkeletonManager::getSingleton().create(basename + ".skeleton", mesh->getGroup(), true);

        createBonesFromNode(scene, scene->mRootNode);
        createBoneHiearchy(scene, scene->mRootNode);

        if (scene->HasAnimations())
        {
            for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
            {
                parseAnimation(scene, i, scene->mAnimations[i]);
            }
        }
    }

    // Create embedded textures
    for (unsigned int i = 0; i < scene->mNumTextures; ++i)
    {
        const aiTexture* tex = scene->mTextures[i];
        auto texname =
            StringUtil::format("%s%s%d.%s", mesh->getName().c_str(), tex->mFilename.C_Str(), i, tex->achFormatHint);
        if (TextureManager::getSingleton().resourceExists(texname, mesh->getGroup()))
            continue;

        Image img;
        if(tex->mHeight == 0)
        {
            auto stream = std::make_shared<MemoryDataStream>(tex->pcData, tex->mWidth, false);
            try
            {
                auto format = String(tex->achFormatHint);
                if (format == "web") // fixup assimp truncation
                    format = "webp";
                img.load(stream, format);
            }
            catch (Exception& e)
            {
                LogManager::getSingleton().logError("Could not load embedded image - " + e.getDescription());
                continue;
            }
        }
        else
        {
            img.loadDynamicImage((uchar*)tex->pcData, tex->mWidth, tex->mHeight, PF_A8R8G8B8);
        }

        TextureManager::getSingleton().loadImage(texname, mesh->getGroup(), img);
    }

    auto aabb = loadDataFromNode(scene, scene->mRootNode, mesh);
    // We must indicate the bounding box
    mesh->_setBounds(aabb);
    mesh->_setBoundingSphereRadius((aabb.getMaximum() - aabb.getMinimum()).length() / 2);

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
    mBoneBindWorldByName.clear();
    mBoneParentSpaceByName.clear();
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

            // Animation channels are relative to the node's direct parent. If
            // the skeleton skips non-bone nodes, parentSpace maps the node-local
            // transform into the space of the nearest skeleton ancestor.
            Affine3 parentSpace = Affine3::IDENTITY;
            auto parentSpaceIt = mBoneParentSpaceByName.find(boneName);
            if (parentSpaceIt != mBoneParentSpaceByName.end())
                parentSpace = parentSpaceIt->second;

            // Components without keys keep the node's rest value. This must not
            // be the bone's bind value: with offset matrices the two can differ.
            aiVector3D restPos(0, 0, 0);
            aiVector3D restScale(1, 1, 1);
            aiQuaternion restRot;
            auto boneNodeIt = mBoneNodesByName.find(boneName);
            if (boneNodeIt != mBoneNodesByName.end() && boneNodeIt->second)
                boneNodeIt->second->mTransformation.Decompose(restScale, restRot, restPos);

            NodeAnimationTrack* track = animation->createNodeTrack(bone->getHandle(), bone);

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
                    // A scale key with no coincident pos/rot keyframe must key
                    // the merged map by its OWN scaling time; keying it by the
                    // rotation time inserts garbage times and reads out of
                    // bounds when the channel has fewer rotation than scale keys.
                    keyframes[(Real)node_anim->mScalingKeys[j].mTime / mTicksPerSecond] =
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

                    // A component whose channel carries NO keys keeps the node's
                    // rest value. Using the bone's bind value here would be
                    // wrong when the offset matrices describe a different bind
                    // pose than the node hierarchy.
                    if (node_anim->mNumPositionKeys == 0)
                        trans = Vector3(restPos.x, restPos.y, restPos.z);
                    if (node_anim->mNumRotationKeys == 0)
                        rot = Quaternion(restRot.w, restRot.x, restRot.y, restRot.z);
                    if (node_anim->mNumScalingKeys == 0)
                        scale = Vector3(restScale.x, restScale.y, restScale.z);

                    Affine3 fullTransform;
                    fullTransform.makeTransform(trans, scale, rot);
                    Affine3 targetTransform = parentSpace * fullTransform;

                    Vector3 targetTrans, targetScale;
                    Quaternion targetRot;
                    targetTransform.decomposition(targetTrans, targetScale, targetRot);

                    keyframe = track->createNodeKeyFrame(Real(it->first));

                    // Ogre applies keyframes as deltas to the bone's binding pose:
                    // translation is added in parent space, while rotation and
                    // scale are composed in bone-local space (see
                    // NodeAnimationTrack::applyToNode).  Do NOT decompose
                    // defBonePose.Inverse() * fullTransform: that yields a
                    // translation expressed in bind-bone axes, which makes Ogre
                    // rotate animated translations by the inverse bind
                    // orientation.  The old code worked around this for the first
                    // root bone only.
                    keyframe->setTranslate(targetTrans - bone->getPosition());
                    keyframe->setRotation(bone->getOrientation().Inverse() * targetRot);
                    keyframe->setScale(componentwiseDivide(targetScale, bone->getScale()));
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

const aiNode* AssimpLoader::findSkeletonParent(const aiNode* pNode) const
{
    const aiNode* parent = pNode->mParent;
    while (parent)
    {
        auto it = boneMap.find(String(parent->mName.data));
        if (it != boneMap.end() && it->second)
            return parent;
        parent = parent->mParent;
    }
    return nullptr;
}

Affine3 AssimpLoader::getBoneBindWorld(const String& name) const
{
    auto bindIt = mBoneBindWorldByName.find(name);
    if (bindIt != mBoneBindWorldByName.end())
        return bindIt->second;

    auto nodeIt = mNodeDerivedTransformByName.find(name);
    if (nodeIt != mNodeDerivedTransformByName.end())
        return nodeIt->second;

    return Affine3::IDENTITY;
}

void AssimpLoader::createBonesFromNode(const aiScene* mScene, const aiNode* pNode)
{
    if (isNodeNeeded(pNode->mName.data))
    {
        const String boneName(pNode->mName.data);
        Bone* bone = mSkeleton->createBone(boneName);

        // The skeleton may skip nodes that are not bones.  Reconstruct the
        // bone's transform relative to the nearest skeleton ancestor, and
        // remember the rest transform of the skipped segment so animation
        // channels, which are relative to the direct node parent, can be
        // mapped into the same space.
        const aiNode* skeletonParent = findSkeletonParent(pNode);
        Affine3 ancestorRestWorld = Affine3::IDENTITY;
        if (skeletonParent)
        {
            auto it = mNodeDerivedTransformByName.find(skeletonParent->mName.data);
            if (it != mNodeDerivedTransformByName.end())
                ancestorRestWorld = it->second;
        }

        Affine3 parentSpace = Affine3::IDENTITY;
        if (pNode->mParent)
        {
            auto it = mNodeDerivedTransformByName.find(pNode->mParent->mName.data);
            if (it != mNodeDerivedTransformByName.end())
                parentSpace = ancestorRestWorld.inverse() * it->second;
        }
        mBoneParentSpaceByName[boneName] = parentSpace;

        // Prefer the bind pose described by aiBone::mOffsetMatrix.  Nodes that
        // are only part of the hierarchy (not weighted) fall back to their node
        // rest transform.
        Affine3 bindWorld = getBoneBindWorld(boneName);

        // The local bind pose is relative to the ancestor's bind pose, not to
        // its rest pose: the ancestor may itself be bound using an offset
        // matrix that differs from its node transform.
        Affine3 ancestorBindWorld = Affine3::IDENTITY;
        if (skeletonParent)
            ancestorBindWorld = getBoneBindWorld(skeletonParent->mName.data);

        Affine3 localBind = ancestorBindWorld.inverse() * bindWorld;

        Vector3 pos = Vector3::ZERO;
        Vector3 scale = Vector3::UNIT_SCALE;
        Quaternion rot = Quaternion::IDENTITY;
        localBind.decomposition(pos, scale, rot);
        bone->setPosition(pos);
        bone->setOrientation(rot);
        bone->setScale(scale);

        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage(StringConverter::toString(bone->getHandle()) +
                                                  ") Creating bone '" + boneName + "'");
        }
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
        Bone* child = 0;
        Bone* parent = 0;

        // Attach to the nearest skeleton ancestor, skipping nodes that are not
        // represented by a bone (their transforms are folded into the local
        // bind transform above).
        const aiNode* skeletonParent = findSkeletonParent(pNode);
        if (skeletonParent && mSkeleton->hasBone(skeletonParent->mName.data))
        {
            parent = mSkeleton->getBone(skeletonParent->mName.data);
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
    if (pNode->mNumMeshes > 0)
    {
        for (unsigned int idx = 0; idx < pNode->mNumMeshes; ++idx)
        {
            aiMesh* pAIMesh = mScene->mMeshes[pNode->mMeshes[idx]];

            if (pAIMesh->HasBones())
            {
                // Some importers (e.g. ASE) create aiBones but never fill in
                // mOffsetMatrix, leaving it at identity.  Only trust the offset
                // matrices if at least one of them was actually set.
                bool hasBindPose = false;
                for (uint32 i = 0; i < pAIMesh->mNumBones && !hasBindPose; ++i)
                    hasBindPose = pAIMesh->mBones[i] && !pAIMesh->mBones[i]->mOffsetMatrix.IsIdentity();

                for (uint32 i = 0; i < pAIMesh->mNumBones; ++i)
                {
                    aiBone* pAIBone = pAIMesh->mBones[i];
                    if (NULL != pAIBone)
                    {
                        const String boneName(pAIBone->mName.data);
                        mBonesByName[boneName] = pAIBone;

                        // mOffsetMatrix maps vertices from mesh space into this
                        // bone's space in bind pose. Vertices are loaded in world
                        // space (see computeNodesDerivedTransform), so the bind
                        // pose in that space is meshWorld * offset^-1.  This is
                        // authoritative: node rest transforms are not guaranteed
                        // to match the bind pose stored by the file (e.g. glTF
                        // inverseBindMatrices).
                        auto meshWorld = mNodeDerivedTransformByName.find(pNode->mName.data);
                        if (hasBindPose && meshWorld != mNodeDerivedTransformByName.end() &&
                            mBoneBindWorldByName.find(boneName) == mBoneBindWorldByName.end())
                        {
                            Affine3 offset(pAIBone->mOffsetMatrix[0]);
                            if (Math::Abs(offset.linear().determinant()) > 1e-12f)
                                mBoneBindWorldByName[boneName] = meshWorld->second * offset.inverse();
                        }

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
                            if (node->mName == pNode->mName)
                            {
                                flagNodeAsNeeded(node->mName.data);
                                break;
                            }
                            if (node->mName == pNode->mParent->mName)
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

static bool getTextureName(const aiMaterial* mat, aiTextureType type, const aiScene* scene, const String& meshName,
                           String& basename)
{
    aiString path;
    String outPath;
    if (mat->GetTexture(type, 0, &path) == AI_SUCCESS)
    {
        const aiTexture* tex = scene->GetEmbeddedTexture(path.C_Str());
        if(tex)
        {
            basename = StringUtil::format("%s%s%s.%.8s", meshName.c_str(), tex->mFilename.C_Str(), path.C_Str() + 1,
                                          tex->achFormatHint);
            return true;
        }

        StringUtil::splitFilename(String(path.data), basename, outPath);
        return true;
    }
    return false;
}

static MaterialPtr createMaterial(const aiMaterial* mat, const Ogre::String &group, const String& meshName, const aiScene* scene, bool verbose)
{
    static int dummyMatCount = 0;

    MaterialManager* omatMgr = MaterialManager::getSingletonPtr();
    enum aiTextureType type = aiTextureType_DIFFUSE;
    static aiString path;
    unsigned int uvindex = 0; // the texture uv index channel

    aiString szPath;
    if (AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_NAME, &szPath))
    {
        if (verbose)
        {
            LogManager::getSingleton().logMessage("Using aiGetMaterialString : Name " +
                                                  String(szPath.data));
        }
    }

    String matName = meshName;
    if(szPath.length)
    {
        matName += String(szPath.data);
        matName = ReplaceSpaces(matName);
    }
    else
    {
        matName += StringUtil::format("dummyMat%d", dummyMatCount++);
    }

    auto status = omatMgr->createOrRetrieve(matName, group);
    auto omat = static_pointer_cast<Material>(status.first);

    if (!status.second)
        return omat;

    if (verbose)
    {
        LogManager::getSingleton().logMessage("Creating " + matName);
    }

    aiString tmp;
    String alphaMode;
    if (AI_SUCCESS == aiGetMaterialString(mat, "$mat.gltf.alphaMode", 0, 0, &tmp))
    {
        alphaMode = String(tmp.data);
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

    if (clr.a < 1.0f || alphaMode == "BLEND")
    {
        omat->setSceneBlending(SBT_TRANSPARENT_ALPHA);
        omat->setDepthWriteEnabled(false);
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

    float alphaCutoff = 1.0f;
    if (AI_SUCCESS == mat->Get("$mat.gltf.alphaCutoff", 0, 0, alphaCutoff) && alphaMode == "MASK")
    {
        omat->getTechnique(0)->getPass(0)->setAlphaRejectSettings(CMPF_GREATER, alphaCutoff * 255.0f);
    }

    String basename;
    if (getTextureName(mat, type, scene, meshName, basename))
    {
        if (verbose)
        {
            LogManager::getSingleton().logMessage("Found texture " + basename + " for channel " +
                                                  StringConverter::toString(uvindex));
        }
        omat->getTechnique(0)->getPass(0)->createTextureUnitState(basename);
    }

    if (getTextureName(mat, aiTextureType_EMISSIVE, scene, meshName, basename))
    {
        if (verbose)
        {
            LogManager::getSingleton().logMessage("Found emissive map: " + basename);
        }
        auto tus = omat->getTechnique(0)->getPass(0)->createTextureUnitState(basename);
        tus->setColourOperation(LBO_ADD);
        omat->setSelfIllumination(0, 0, 0); // assimp assumes emissive is modulated, whereas Ogre adds up
    }

#ifdef OGRE_BUILD_COMPONENT_RTSHADERSYSTEM
    auto shaderGen = RTShader::ShaderGenerator::getSingletonPtr();

    if(!shaderGen)
        return omat;

    if (getTextureName(mat, aiTextureType_NORMALS, scene, meshName, basename))
    {
        if (verbose)
        {
            LogManager::getSingleton().logMessage("Found normal map: " + basename);
        }

        auto tus = omat->getTechnique(0)->getPass(0)->createTextureUnitState(basename);

        // mark as normal map
        shaderGen->_markNonFFP(tus);
        uint16 texureIdx = tus->getParent()->getNumTextureUnitStates() - 1;

        shaderGen->createShaderBasedTechnique(omat->getTechnique(0), MSN_SHADERGEN);
        auto rs = shaderGen->getRenderState(MSN_SHADERGEN, *omat, 0);
        auto srs = shaderGen->createSubRenderState("NormalMap");

        srs->setParameter("texture_index", std::to_string(texureIdx));
        rs->addTemplateSubRenderState(srs);
    }

    aiTextureType metalRoughnessType = aiTextureType_UNKNOWN;
    // regards to assimp devs for this nice macro, requiring this statement to unpack
    if(metalRoughnessType = AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE) {}

    if (getTextureName(mat, metalRoughnessType, scene, meshName, basename))
    {
        if (verbose)
        {
            LogManager::getSingleton().logMessage("Found metal roughness map: " + basename);
        }

        shaderGen->createShaderBasedTechnique(omat->getTechnique(0), MSN_SHADERGEN);
        auto rs = shaderGen->getRenderState(MSN_SHADERGEN, *omat, 0);
        auto srs = shaderGen->createSubRenderState(RTShader::SRS_COOK_TORRANCE_LIGHTING);

        srs->setParameter("texture", basename);
        rs->addTemplateSubRenderState(srs);

        srs = shaderGen->createSubRenderState(RTShader::SRS_TEXTURING);
        srs->setParameter("late_add_blend", "true");
        rs->addTemplateSubRenderState(srs);
    }
#endif

    return omat;
}

bool AssimpLoader::createSubMesh(const String& name, int index, const aiNode* pNode, const aiMesh* mesh,
                                 const MaterialPtr& matptr, Mesh* mMesh, AxisAlignedBox& mAAB)
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

    // now begin the object definition
    // We create a submesh per material
    SubMesh* submesh = mMesh->createSubMesh(name + StringConverter::toString(index));

    // prime pointers to vertex related data
    aiVector3D* vec = mesh->mVertices;
    aiVector3D* norm = mesh->mNormals;
    aiVector3D* tang = mesh->mTangents;
    aiColor4D *col = mesh->mColors[0];

    if(!norm)
        matptr->setLightingEnabled(false);

    // We must create the vertex data, indicating how many vertices there will be
    submesh->createVertexData();
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

    for (int uvindex = 0; uvindex < AI_MAX_NUMBER_OF_TEXTURECOORDS; uvindex++)
    {
        aiVector3D* uv = mesh->mTextureCoords[uvindex];
        if (!uv)
            break;
        if (!mQuietMode)
        {
            LogManager::getSingleton().logMessage(StringUtil::format("%d uvs", mesh->mNumVertices));
        }
        offset += declaration->addElement(source, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, uvindex).getSize();
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
        for (int uvindex = 0; uvindex < AI_MAX_NUMBER_OF_TEXTURECOORDS; uvindex++)
        {
            aiVector3D* uv = mesh->mTextureCoords[uvindex];
            if (!uv)
                break;
            *vdata++ = uv[i].x;
            *vdata++ = uv[i].y;
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

AxisAlignedBox AssimpLoader::loadDataFromNode(const aiScene* mScene, const aiNode* pNode, Mesh* mesh)
{
    AxisAlignedBox aabb;
    if (pNode->mNumMeshes > 0)
    {
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
            MaterialPtr matptr = createMaterial(pAIMaterial, mesh->getGroup(), mesh->getName(), mScene, !mQuietMode);
            createSubMesh(pNode->mName.data, idx, pNode, pAIMesh, matptr, mesh, aabb);
        }
    }

    // Traverse all child nodes of the current node instance
    for (unsigned int childIdx = 0; childIdx < pNode->mNumChildren; childIdx++)
    {
        const aiNode* pChildNode = pNode->mChildren[childIdx];
        aabb.merge(loadDataFromNode(mScene, pChildNode, mesh));
    }

    return aabb;
}

static std::vector<std::unique_ptr<Codec> > registeredCodecs;
struct AssimpCodec : public Codec
{
    String mType;

    AssimpCodec(const String& type) : mType(type) {}

    String getType() const override { return mType; }
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
        String version = StringUtil::format("Assimp - %d.%d.%x - Open-Asset-Importer", aiGetVersionMajor(),
                                            aiGetVersionMinor(), aiGetVersionRevision());
        LogManager::getSingleton().logMessage(version);

        String extensions;
        Assimp::Importer tmp;
        tmp.GetExtensionList(extensions);

        String blacklist[] = {"mesh", "mesh.xml", "raw", "mdc", "bsp"};
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
