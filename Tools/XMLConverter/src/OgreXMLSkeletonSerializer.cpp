/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#include "OgreXMLSkeletonSerializer.h"
#include "OgreSkeleton.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgreBone.h"
#include "OgreString.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "Ogre.h"

#include <map>

namespace Ogre {

    
    //---------------------------------------------------------------------
    XMLSkeletonSerializer::XMLSkeletonSerializer()
    {
    }
    //---------------------------------------------------------------------
    XMLSkeletonSerializer::~XMLSkeletonSerializer()
    {
    }

    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::importSkeleton(const String& filename, Skeleton* pSkeleton)
    {   
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer: reading XML data from " + filename + "...");

        pugi::xml_document mXMLDoc;
        mXMLDoc.load_file(filename.c_str());

        pugi::xml_node elem;

        pugi::xml_node rootElem = mXMLDoc.document_element();
        
        // Optional blend mode
        const char* blendModeStr = rootElem.attribute("blendmode").as_string(NULL);
        if (blendModeStr)
        {
            if (String(blendModeStr) == "cumulative")
                pSkeleton->setBlendMode(ANIMBLEND_CUMULATIVE);
            else 
                pSkeleton->setBlendMode(ANIMBLEND_AVERAGE);

        }
        

        // Bones
        elem = rootElem.child("bones");
        if (elem)
        {
            readBones(pSkeleton, elem);         
            elem = rootElem.child("bonehierarchy");

            if (elem)
            {
                createHierarchy(pSkeleton, elem) ;
                elem = rootElem.child("bones");
                if (elem)
                {
                    readBones2(pSkeleton, elem);
                    elem = rootElem.child("animations");
                    if (elem)
                    {
                        readAnimations(pSkeleton, elem);
                    }
                    elem = rootElem.child("animationlinks");
                    if (elem)
                    {
                        readSkeletonAnimationLinks(pSkeleton, elem);
                    }
                }
            }
        }
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Finished. Running SkeletonSerializer..." );
    }
    

    //---------------------------------------------------------------------
    // sets names
    void XMLSkeletonSerializer::readBones(Skeleton* skel, pugi::xml_node& mBonesNode)
    {
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Reading Bones name...");
        
        Quaternion quat ;

        int max_id = -1;

        for (pugi::xml_node& bonElem : mBonesNode.children())
        {
            String name = bonElem.attribute("name").value();
            int id = StringConverter::parseInt(bonElem.attribute("id").value());
            skel->createBone(name,id) ;

            max_id = std::max(id, max_id);
        }

        OgreAssert(size_t(max_id + 1) == skel->getBones().size(), "Bone ids must be consecutive in range [0; N)");
    }
    // ---------------------------------------------------------
    // set positions and orientations.
    void XMLSkeletonSerializer::readBones2(Skeleton* skel, pugi::xml_node& mBonesNode)
    {
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Reading Bones data...");
        
        Bone* btmp ;
        Quaternion quat ;

        for (pugi::xml_node& bonElem : mBonesNode.children())
        {
            String name = bonElem.attribute("name").value();
//          int id = StringConverter::parseInt(bonElem.attribute("id").c_str();

            pugi::xml_node posElem = bonElem.child("position");
            pugi::xml_node rotElem = bonElem.child("rotation");
            pugi::xml_node axisElem = rotElem.child("axis");
            pugi::xml_node scaleElem = bonElem.child("scale");
            
            Vector3 pos;
            Vector3 axis;
            Radian angle ;
            Vector3 scale;

            pos.x = StringConverter::parseReal(posElem.attribute("x").value());
            pos.y = StringConverter::parseReal(posElem.attribute("y").value());
            pos.z = StringConverter::parseReal(posElem.attribute("z").value());
            
            angle = Radian(StringConverter::parseReal(rotElem.attribute("angle").value()));

            axis.x = StringConverter::parseReal(axisElem.attribute("x").value());
            axis.y = StringConverter::parseReal(axisElem.attribute("y").value());
            axis.z = StringConverter::parseReal(axisElem.attribute("z").value());
            
            // Optional scale
            if (scaleElem)
            {
                // Uniform scale or per axis?
                const char* factorAttrib = scaleElem.attribute("factor").as_string(NULL);
                if (factorAttrib)
                {
                    // Uniform scale
                    Real factor = StringConverter::parseReal(factorAttrib);
                    scale = Vector3(factor, factor, factor);
                }
                else
                {
                    // axis scale
                    scale = Vector3::UNIT_SCALE;
                    const char* factorString = scaleElem.attribute("x").as_string(NULL);
                    if (factorString)
                    {
                        scale.x = StringConverter::parseReal(factorString);
                    }
                    factorString = scaleElem.attribute("y").value();
                    if (factorString)
                    {
                        scale.y = StringConverter::parseReal(factorString);
                    }
                    factorString = scaleElem.attribute("z").value();
                    if (factorString)
                    {
                        scale.z = StringConverter::parseReal(factorString);
                    }
                }
            }
            else
            {
                scale = Vector3::UNIT_SCALE;
            }

            /*LogManager::getSingleton().logMessage("bone " + name + " : position("
                + StringConverter::toString(pos.x) + "," + StringConverter::toString(pos.y) + "," + StringConverter::toString(pos.z) + ")"
                + " - angle: " + StringConverter::toString(angle) +" - axe: "
                + StringConverter::toString(axis.x) + "," + StringConverter::toString(axis.y) + "," + StringConverter::toString(axis.z) );
            */      
            
            btmp = skel->getBone(name) ;

            btmp -> setPosition(pos);
            quat.FromAngleAxis(angle,axis);
            btmp -> setOrientation(quat) ;
            btmp -> setScale(scale);

        } // bones
    }
    //-------------------------------------------------------------------
    void XMLSkeletonSerializer::createHierarchy(Skeleton* skel, pugi::xml_node& mHierNode) {
        
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Reading Hierarchy data...");
        
        Bone* bone ;
        Bone* parent ;
        String boneName ;
        String parentName ;

        for (pugi::xml_node& hierElem : mHierNode.children())
        {
            boneName = hierElem.attribute("bone").value();
            parentName = hierElem.attribute("parent").value();
            bone = skel->getBone(boneName);
            parent = skel->getBone(parentName);
            parent ->addChild(bone) ;
            //LogManager::getSingleton().logMessage("XMLSkeletonSerialiser: lien: " + parent->getName() + "->" + bone->getName().c_str();
            
        }
    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::readAnimations(Skeleton* skel, pugi::xml_node& mAnimNode) {
        
        Animation * anim ;
        NodeAnimationTrack * track ;
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Reading Animations data...");

        for (pugi::xml_node& animElem : mAnimNode.children("animation"))
        {
            String name = animElem.attribute("name").value();
            Real length = StringConverter::parseReal(animElem.attribute("length").value());
            anim = skel->createAnimation(name,length);
            anim->setInterpolationMode(Animation::IM_LINEAR) ;

            
            //LogManager::getSingleton().logMessage("Animation: nom: " + name + " et longueur: "
            //  + StringConverter::toString(length) );
            pugi::xml_node baseInfoNode = animElem.child("baseinfo");
            if (baseInfoNode)
            {
                String baseName = baseInfoNode.attribute("baseanimationname").value();
                Real baseTime = StringConverter::parseReal(baseInfoNode.attribute("basekeyframetime").value());
                anim->setUseBaseKeyFrame(true, baseTime, baseName);
            }
            
            
            
            // lecture des tracks
            int trackIndex = 0;
            pugi::xml_node tracksNode = animElem.child("tracks");
            
            for (pugi::xml_node& trackElem : tracksNode.children("track"))
            {
                String boneName = trackElem.attribute("bone").value();

                //LogManager::getSingleton().logMessage("Track sur le bone: " + boneName );

                track = anim->createNodeTrack(trackIndex++,skel->getBone(boneName));
                readKeyFrames(track, trackElem.child("keyframes"));
            }
            
        }


    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::readKeyFrames(NodeAnimationTrack* track, const pugi::xml_node& mKeyfNode) {
        
        TransformKeyFrame* kf ;
        Quaternion q ;

        for (pugi::xml_node& keyfElem : mKeyfNode.children("keyframe"))
        {
            Vector3 trans;
            Vector3 axis;
            Radian angle;
            Real time;

            // Get time and create keyframe
            time = StringConverter::parseReal(keyfElem.attribute("time").value());
            kf = track->createNodeKeyFrame(time);
            // Optional translate
            pugi::xml_node transElem = keyfElem.child("translate");
            if (transElem)
            {
                trans.x = StringConverter::parseReal(transElem.attribute("x").value());
                trans.y = StringConverter::parseReal(transElem.attribute("y").value());
                trans.z = StringConverter::parseReal(transElem.attribute("z").value());
                kf->setTranslate(trans) ;
            }
            // Optional rotate
            pugi::xml_node rotElem = keyfElem.child("rotate");
            if (rotElem)
            {
                pugi::xml_node axisElem = rotElem.child("axis");
                if (!axisElem)
                {
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Missing 'axis' element "
                    "expected under parent 'rotate'", "MXLSkeletonSerializer::readKeyFrames");
                }
                angle = Radian(StringConverter::parseReal(rotElem.attribute("angle").value()));

                axis.x = StringConverter::parseReal(axisElem.attribute("x").value());
                axis.y = StringConverter::parseReal(axisElem.attribute("y").value());
                axis.z = StringConverter::parseReal(axisElem.attribute("z").value());

                q.FromAngleAxis(angle,axis);
                kf->setRotation(q) ;

            }
            // Optional scale
            pugi::xml_node scaleElem = keyfElem.child("scale");
            if (scaleElem)
            {
                // Uniform scale or per axis?
                const char* factorAttrib = scaleElem.attribute("factor").as_string(NULL);
                if (factorAttrib)
                {
                    // Uniform scale
                    Real factor = StringConverter::parseReal(factorAttrib);
                    kf->setScale(Vector3(factor, factor, factor));
                }
                else
                {
                    // axis scale
                    Real xs = 1.0f, ys = 1.0f, zs=1.0f;
                    const char* factorString = scaleElem.attribute("x").as_string(NULL);
                    if(factorString)
                    {
                        xs = StringConverter::parseReal(factorString);
                    }
                    factorString = scaleElem.attribute("y").value();
                    if(factorString)
                    {
                        ys = StringConverter::parseReal(factorString);
                    }
                    factorString = scaleElem.attribute("z").value();
                    if(factorString)
                    {
                        zs = StringConverter::parseReal(factorString);
                    }
                    kf->setScale(Vector3(xs, ys, zs));
                    
                }
            }

            
            /*
            LogManager::getSingleton().logMessage("Keyframe: translation("
                + StringConverter::toString(trans.x) + "," + StringConverter::toString(trans.y) + "," + StringConverter::toString(trans.z) + ")"
                + " - angle: " + StringConverter::toString(angle) +" - axe: "
                + StringConverter::toString(axis.x) + "," + StringConverter::toString(axis.y) + "," + StringConverter::toString(axis.z) );
            */
            

        }
    }

    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::exportSkeleton(const Skeleton* pSkeleton, 
        const String& filename)
    {
        
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer writing "
            " skeleton data to " + filename + "...");

        pugi::xml_document mXMLDoc;
        pugi::xml_node rootNode = mXMLDoc.append_child("skeleton");

        LogManager::getSingleton().logMessage("Populating DOM...");


        // Write main skeleton data
        LogManager::getSingleton().logMessage("Exporting bones..");
        writeSkeleton(pSkeleton, rootNode);
        LogManager::getSingleton().logMessage("Bones exported.");
        
        // Write all animations
        unsigned short numAnims = pSkeleton->getNumAnimations();
        String msg = "Exporting animations, count=" + StringConverter::toString(numAnims);
        LogManager::getSingleton().logMessage(msg);

        pugi::xml_node animsNode = rootNode.append_child("animations");

        for (unsigned short i = 0; i < numAnims; ++i)
        {
            Animation* pAnim = pSkeleton->getAnimation(i);
            msg = "Exporting animation: " + pAnim->getName();
            LogManager::getSingleton().logMessage(msg);
            writeAnimation(animsNode, pAnim);
            LogManager::getSingleton().logMessage("Animation exported.");

        }

        // Write links
        if (!pSkeleton->getLinkedSkeletonAnimationSources().empty())
        {
            LogManager::getSingleton().logMessage("Exporting animation links.");
            pugi::xml_node linksNode = rootNode.append_child("animationlinks");
            for(const auto& link : pSkeleton->getLinkedSkeletonAnimationSources())
            {
                writeSkeletonAnimationLink(linksNode, link);
            }
        }

        LogManager::getSingleton().logMessage("DOM populated, writing XML file..");

        // Write out to a file
        if(! mXMLDoc.save_file(filename.c_str()) )
        {
            LogManager::getSingleton().logMessage("XMLSkeletonSerializer failed writing the XML file.", LML_CRITICAL);
        }
        else
        {
            LogManager::getSingleton().logMessage("XMLSkeletonSerializer export successful.");
        }
    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeSkeleton(const Skeleton* pSkel, pugi::xml_node& rootNode)
    {
        // Blend mode
        String blendModeStr = pSkel->getBlendMode() == ANIMBLEND_CUMULATIVE ? "cumulative" : "average";
        rootNode.append_attribute("blendmode") = blendModeStr.c_str();

        pugi::xml_node bonesElem = rootNode.append_child("bones");

        unsigned short numBones = pSkel->getNumBones();
        LogManager::getSingleton().logMessage("There are " + StringConverter::toString(numBones) + " bones.");
        unsigned short i;
        for (i = 0; i < numBones; ++i)
        {
            LogManager::getSingleton().logMessage("   Exporting Bone number " + StringConverter::toString(i));
            Bone* pBone = pSkel->getBone(i);
            writeBone(bonesElem, pBone);
        }

        // Write parents
        pugi::xml_node hierElem = rootNode.append_child("bonehierarchy");
        for (i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            String name = pBone->getName() ;

            if ((pBone->getParent())!=NULL) // root bone
            {
                Bone* pParent = (Bone*)pBone->getParent();
                writeBoneParent(hierElem, name, pParent->getName());
            }
        }


    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeBone(pugi::xml_node& bonesElement, const Bone* pBone)
    {
        pugi::xml_node boneElem = bonesElement.append_child("bone");

        // Bone name & handle
        boneElem.append_attribute("id") = StringConverter::toString(pBone->getHandle()).c_str();
        boneElem.append_attribute("name") = pBone->getName().c_str();

        // Position
        pugi::xml_node subNode = boneElem.append_child("position");
        Vector3 pos = pBone->getPosition();
        subNode.append_attribute("x") = StringConverter::toString(pos.x).c_str();
        subNode.append_attribute("y") = StringConverter::toString(pos.y).c_str();
        subNode.append_attribute("z") = StringConverter::toString(pos.z).c_str();
        
        // Orientation 
        subNode = 
            boneElem.append_child("rotation");
        // Show Quaternion as angle / axis
        Radian angle;
        Vector3 axis;
        pBone->getOrientation().ToAngleAxis(angle, axis);
        pugi::xml_node axisNode = subNode.append_child("axis");
        subNode.append_attribute("angle") = StringConverter::toString(angle.valueRadians()).c_str();
        axisNode.append_attribute("x") = StringConverter::toString(axis.x).c_str();
        axisNode.append_attribute("y") = StringConverter::toString(axis.y).c_str();
        axisNode.append_attribute("z") = StringConverter::toString(axis.z).c_str();

        // Scale optional
        Vector3 scale = pBone->getScale();
        if (scale != Vector3::UNIT_SCALE)
        {
            pugi::xml_node scaleNode = boneElem.append_child("scale");
            scaleNode.append_attribute("x") = StringConverter::toString(scale.x).c_str();
            scaleNode.append_attribute("y") = StringConverter::toString(scale.y).c_str();
            scaleNode.append_attribute("z") = StringConverter::toString(scale.z).c_str();
        }


    }
    //---------------------------------------------------------------------
    // 
    // Modifications effectu�es:
    //
    // on stoque les noms et pas les Id. c'est plus lisibles.


    void XMLSkeletonSerializer::writeBoneParent(pugi::xml_node& boneHierarchyNode,
        String boneName, String parentName)
    {
        pugi::xml_node boneParentNode = boneHierarchyNode.append_child("boneparent");
        /*
        boneParentNode.append_attribute("boneid") = StringConverter::toString(boneId).c_str();
        boneParentNode.append_attribute("parentid") = StringConverter::toString(parentId).c_str();
        */
        // Modifications: on stoque les noms./ 
        boneParentNode.append_attribute("bone") = boneName.c_str();
        boneParentNode.append_attribute("parent") = parentName.c_str();

    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeAnimation(pugi::xml_node& animsNode,
        const Animation* anim)
    {
        pugi::xml_node animNode = animsNode.append_child("animation");

        animNode.append_attribute("name") = anim->getName().c_str();
        animNode.append_attribute("length") = StringConverter::toString(anim->getLength()).c_str();
        
        // Optional base keyframe information
        if (anim->getUseBaseKeyFrame())
        {
            pugi::xml_node baseInfoNode = animNode.append_child("baseinfo");
            baseInfoNode.append_attribute("baseanimationname") = anim->getBaseKeyFrameAnimationName().c_str();
            baseInfoNode.append_attribute("basekeyframetime") = StringConverter::toString(anim->getBaseKeyFrameTime()).c_str();
        }

        // Write all tracks
        pugi::xml_node tracksNode = animNode.append_child("tracks");

        for (const auto& it : anim->_getNodeTrackList())
        {
            writeAnimationTrack(tracksNode, it.second);
        }

    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeAnimationTrack(pugi::xml_node& tracksNode,
        const NodeAnimationTrack* track)
    {
        pugi::xml_node trackNode = tracksNode.append_child("track");

        // unsigned short boneIndex     : Index of bone to apply to
        Bone* bone = (Bone*)track->getAssociatedNode();
        //unsigned short boneid = bone->getHandle();
        String boneName = bone->getName();
        trackNode.append_attribute("bone") = boneName.c_str();

        // Write all keyframes
        pugi::xml_node keysNode =
            trackNode.append_child("keyframes");
        for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
        {
            writeKeyFrame(keysNode, track->getNodeKeyFrame(i));
        }
    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeKeyFrame(pugi::xml_node& keysNode,
        const TransformKeyFrame* key)
    {
        pugi::xml_node keyNode = keysNode.append_child("keyframe");

        keyNode.append_attribute("time") = StringConverter::toString(key->getTime()).c_str();

        pugi::xml_node transNode =
            keyNode.append_child("translate");
        Vector3 trans = key->getTranslate();
        transNode.append_attribute("x") = StringConverter::toString(trans.x).c_str();
        transNode.append_attribute("y") = StringConverter::toString(trans.y).c_str();
        transNode.append_attribute("z") = StringConverter::toString(trans.z).c_str();

        pugi::xml_node rotNode = keyNode.append_child("rotate");
        // Show Quaternion as angle / axis
        Radian angle;
        Vector3 axis;
        key->getRotation().ToAngleAxis(angle, axis);
        pugi::xml_node axisNode = rotNode.append_child("axis");
        rotNode.append_attribute("angle") = StringConverter::toString(angle.valueRadians()).c_str();
        axisNode.append_attribute("x") = StringConverter::toString(axis.x).c_str();
        axisNode.append_attribute("y") = StringConverter::toString(axis.y).c_str();
        axisNode.append_attribute("z") = StringConverter::toString(axis.z).c_str();

        // Scale optional
        if (key->getScale() != Vector3::UNIT_SCALE)
        {
            pugi::xml_node scaleNode = keyNode.append_child("scale");

            scaleNode.append_attribute("x") = StringConverter::toString(key->getScale().x).c_str();
            scaleNode.append_attribute("y") = StringConverter::toString(key->getScale().y).c_str();
            scaleNode.append_attribute("z") = StringConverter::toString(key->getScale().z).c_str();
        }

    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeSkeletonAnimationLink(pugi::xml_node& linksNode,
        const LinkedSkeletonAnimationSource& link)
    {
        pugi::xml_node linkNode = linksNode.append_child("animationlink");
        linkNode.append_attribute("skeletonName") = link.skeletonName.c_str();
        linkNode.append_attribute("scale") = StringConverter::toString(link.scale).c_str();

    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::readSkeletonAnimationLinks(Skeleton* skel, 
        pugi::xml_node& linksNode)
    {
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Reading Animations links...");

        for (pugi::xml_node& linkElem : linksNode.children("animationlink"))
        {
            String skelName = linkElem.attribute("skeletonName").value();
            const char* strScale = linkElem.attribute("scale").as_string(NULL);
            Real scale;
            // Scale optional
            if (strScale == 0)
            {
                scale = 1.0f;
            }
            else
            {
                scale = StringConverter::parseReal(strScale);
            }
            skel->addLinkedSkeletonAnimationSource(skelName, scale);

        }
    }
}


