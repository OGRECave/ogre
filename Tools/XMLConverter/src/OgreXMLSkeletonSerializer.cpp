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

		mXMLDoc = new TiXmlDocument(filename);
        mXMLDoc->LoadFile();

		TiXmlElement* elem;

        TiXmlElement* rootElem = mXMLDoc->RootElement();
		
		// Optional blend mode
		const char* blendModeStr = rootElem->Attribute("blendmode");
		if (blendModeStr)
		{
			if (String(blendModeStr) == "cumulative")
				pSkeleton->setBlendMode(ANIMBLEND_CUMULATIVE);
			else 
				pSkeleton->setBlendMode(ANIMBLEND_AVERAGE);

		}
		

        // Bones
        elem = rootElem->FirstChildElement("bones");
        if (elem)
		{
            readBones(pSkeleton, elem);			
			elem = rootElem->FirstChildElement("bonehierarchy");

			if (elem)
			{
				createHierarchy(pSkeleton, elem) ;
				elem = rootElem->FirstChildElement("bones");
				if (elem)
				{
					readBones2(pSkeleton, elem);
					elem = rootElem->FirstChildElement("animations");
					if (elem)
					{
						readAnimations(pSkeleton, elem);
					}
					elem = rootElem->FirstChildElement("animationlinks");
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
	void XMLSkeletonSerializer::readBones(Skeleton* skel, TiXmlElement* mBonesNode)
    {
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Reading Bones name...");
		
		Quaternion quat ;

        for (TiXmlElement* bonElem = mBonesNode->FirstChildElement();
            bonElem != 0; bonElem = bonElem->NextSiblingElement())
        {
            String name = bonElem->Attribute("name");
			int id = StringConverter::parseInt(bonElem->Attribute("id"));				
			skel->createBone(name,id) ;
        }
    }
	// ---------------------------------------------------------
	// set positions and orientations.
	void XMLSkeletonSerializer::readBones2(Skeleton* skel, TiXmlElement* mBonesNode)
    {
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Reading Bones data...");
		
		Bone* btmp ;
		Quaternion quat ;

        for (TiXmlElement* bonElem = mBonesNode->FirstChildElement();
            bonElem != 0; bonElem = bonElem->NextSiblingElement())
        {
            String name = bonElem->Attribute("name");
//			int id = StringConverter::parseInt(bonElem->Attribute("id"));

			TiXmlElement* posElem = bonElem->FirstChildElement("position");
			TiXmlElement* rotElem = bonElem->FirstChildElement("rotation");
			TiXmlElement* axisElem = rotElem->FirstChildElement("axis");
            TiXmlElement* scaleElem = bonElem->FirstChildElement("scale");
			
			Vector3 pos;
			Vector3 axis;
			Radian angle ;
            Vector3 scale;

			pos.x = StringConverter::parseReal(posElem->Attribute("x"));
			pos.y = StringConverter::parseReal(posElem->Attribute("y"));
			pos.z = StringConverter::parseReal(posElem->Attribute("z"));
			
			angle = Radian(StringConverter::parseReal(rotElem->Attribute("angle")));

			axis.x = StringConverter::parseReal(axisElem->Attribute("x"));
			axis.y = StringConverter::parseReal(axisElem->Attribute("y"));
			axis.z = StringConverter::parseReal(axisElem->Attribute("z"));
			
            // Optional scale
            if (scaleElem)
            {
                // Uniform scale or per axis?
                const char* factorAttrib = scaleElem->Attribute("factor");
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
                    const char* factorString = scaleElem->Attribute("x");
                    if (factorString)
                    {
                        scale.x = StringConverter::parseReal(factorString);
                    }
                    factorString = scaleElem->Attribute("y");
                    if (factorString)
                    {
                        scale.y = StringConverter::parseReal(factorString);
                    }
                    factorString = scaleElem->Attribute("z");
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
	void XMLSkeletonSerializer::createHierarchy(Skeleton* skel, TiXmlElement* mHierNode) {
		
		LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Reading Hierarchy data...");
		
		Bone* bone ;
		Bone* parent ;
		String boneName ;
		String parentName ;

		for (TiXmlElement* hierElem = mHierNode->FirstChildElement() ; hierElem != 0; hierElem = hierElem->NextSiblingElement())
        {
			boneName = hierElem->Attribute("bone");
			parentName = hierElem->Attribute("parent");
			bone = skel->getBone(boneName);
			parent = skel->getBone(parentName);
			parent ->addChild(bone) ;
			//LogManager::getSingleton().logMessage("XMLSkeletonSerialiser: lien: " + parent->getName() + "->" + bone->getName());
			
		}
	}
	//---------------------------------------------------------------------
	void XMLSkeletonSerializer::readAnimations(Skeleton* skel, TiXmlElement* mAnimNode) {
		
		Animation * anim ;
		NodeAnimationTrack * track ;
		LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Reading Animations data...");

		for (TiXmlElement* animElem = mAnimNode->FirstChildElement("animation"); animElem != 0; animElem = animElem->NextSiblingElement())
        {
            String name = animElem->Attribute("name");
			Real length = StringConverter::parseReal(animElem->Attribute("length"));
			anim = skel->createAnimation(name,length);
			anim->setInterpolationMode(Animation::IM_LINEAR) ;

			
			//LogManager::getSingleton().logMessage("Animation: nom: " + name + " et longueur: "
			//	+ StringConverter::toString(length) );
			TiXmlElement* baseInfoNode = animElem->FirstChildElement("baseinfo");
			if (baseInfoNode)
			{
				String baseName = baseInfoNode->Attribute("baseanimationname");
				Real baseTime = StringConverter::parseReal(baseInfoNode->Attribute("basekeyframetime"));
				anim->setUseBaseKeyFrame(true, baseTime, baseName);
			}
			
			
			
			// lecture des tracks
			int trackIndex = 0;
			TiXmlElement* tracksNode = animElem->FirstChildElement("tracks");
			
			for (TiXmlElement* trackElem = tracksNode->FirstChildElement("track"); trackElem != 0; trackElem = trackElem->NextSiblingElement())
			{
				String boneName = trackElem->Attribute("bone");

				//LogManager::getSingleton().logMessage("Track sur le bone: " + boneName );

				track = anim->createNodeTrack(trackIndex++,skel->getBone(boneName));
				readKeyFrames(track, trackElem->FirstChildElement("keyframes"));
			}
			
		}


	}
	//---------------------------------------------------------------------
	void XMLSkeletonSerializer::readKeyFrames(NodeAnimationTrack* track, TiXmlElement* mKeyfNode) {
		
		TransformKeyFrame* kf ;
		Quaternion q ;

		for (TiXmlElement* keyfElem = mKeyfNode->FirstChildElement("keyframe"); keyfElem != 0; keyfElem = keyfElem->NextSiblingElement())
        {
			Vector3 trans;
			Vector3 axis;
			Radian angle;
			Real time;

            // Get time and create keyframe
			time = StringConverter::parseReal(keyfElem->Attribute("time"));
			kf = track->createNodeKeyFrame(time);
            // Optional translate
			TiXmlElement* transElem = keyfElem->FirstChildElement("translate");
            if (transElem)
            {
			    trans.x = StringConverter::parseReal(transElem->Attribute("x"));
			    trans.y = StringConverter::parseReal(transElem->Attribute("y"));
			    trans.z = StringConverter::parseReal(transElem->Attribute("z"));
			    kf->setTranslate(trans) ;
            }
            // Optional rotate
			TiXmlElement* rotElem = keyfElem->FirstChildElement("rotate");
            if (rotElem)
            {
                TiXmlElement* axisElem = rotElem->FirstChildElement("axis");
                if (!axisElem)
                {
                    OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Missing 'axis' element "
                    "expected under parent 'rotate'", "MXLSkeletonSerializer::readKeyFrames");
                }
			    angle = Radian(StringConverter::parseReal(rotElem->Attribute("angle")));

			    axis.x = StringConverter::parseReal(axisElem->Attribute("x"));
			    axis.y = StringConverter::parseReal(axisElem->Attribute("y"));
			    axis.z = StringConverter::parseReal(axisElem->Attribute("z"));

			    q.FromAngleAxis(angle,axis);
			    kf->setRotation(q) ;

            }
            // Optional scale
			TiXmlElement* scaleElem = keyfElem->FirstChildElement("scale");
            if (scaleElem)
            {
                // Uniform scale or per axis?
				const char* factorAttrib = scaleElem->Attribute("factor");
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
                    const char* factorString = scaleElem->Attribute("x");
                    if(factorString)
                    {
                        xs = StringConverter::parseReal(factorString);
                    }
                    factorString = scaleElem->Attribute("y");
                    if(factorString)
                    {
                        ys = StringConverter::parseReal(factorString);
                    }
                    factorString = scaleElem->Attribute("z");
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

        mXMLDoc = new TiXmlDocument();
        mXMLDoc->InsertEndChild(TiXmlElement("skeleton"));
        TiXmlElement* rootNode = mXMLDoc->RootElement();

        LogManager::getSingleton().logMessage("Populating DOM...");


        // Write main skeleton data
        LogManager::getSingleton().logMessage("Exporting bones..");
        writeSkeleton(pSkeleton);
        LogManager::getSingleton().logMessage("Bones exported.");
		
        // Write all animations
        unsigned short numAnims = pSkeleton->getNumAnimations();
		String msg = "Exporting animations, count=" + StringConverter::toString(numAnims);
        LogManager::getSingleton().logMessage(msg);

        TiXmlElement* animsNode = 
            rootNode->InsertEndChild(TiXmlElement("animations"))->ToElement();
        
        for (unsigned short i = 0; i < numAnims; ++i)
        {
            Animation* pAnim = pSkeleton->getAnimation(i);
            msg = "Exporting animation: " + pAnim->getName();
            LogManager::getSingleton().logMessage(msg);
            writeAnimation(animsNode, pAnim);
            LogManager::getSingleton().logMessage("Animation exported.");

        }

		// Write links
		Skeleton::LinkedSkeletonAnimSourceIterator linkIt = 
			pSkeleton->getLinkedSkeletonAnimationSourceIterator();
		if (linkIt.hasMoreElements())
		{
			LogManager::getSingleton().logMessage("Exporting animation links.");
			TiXmlElement* linksNode = 
				rootNode->InsertEndChild(TiXmlElement("animationlinks"))->ToElement();
			while(linkIt.hasMoreElements())
			{
				const LinkedSkeletonAnimationSource& link = linkIt.getNext();
				writeSkeletonAnimationLink(linksNode, link);
			}
		}

        LogManager::getSingleton().logMessage("DOM populated, writing XML file..");

        // Write out to a file
        if(! mXMLDoc->SaveFile(filename) )
        {
            LogManager::getSingleton().logMessage("XMLSkeletonSerializer failed writing the XML file.", LML_CRITICAL);
        }
        else
        {
            LogManager::getSingleton().logMessage("XMLSkeletonSerializer export successful.");
        }

        delete mXMLDoc;

    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeSkeleton(const Skeleton* pSkel)
    {
        TiXmlElement* rootNode = mXMLDoc->RootElement();
		
		// Blend mode
		String blendModeStr = pSkel->getBlendMode() == ANIMBLEND_CUMULATIVE ? "cumulative" : "average";
		rootNode->SetAttribute("blendmode", blendModeStr);

        TiXmlElement* bonesElem = 
            rootNode->InsertEndChild(TiXmlElement("bones"))->ToElement();

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
        TiXmlElement* hierElem = 
            rootNode->InsertEndChild(TiXmlElement("bonehierarchy"))->ToElement();
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
    void XMLSkeletonSerializer::writeBone(TiXmlElement* bonesElement, const Bone* pBone)
    {
        TiXmlElement* boneElem = 
            bonesElement->InsertEndChild(TiXmlElement("bone"))->ToElement();

        
        // Bone name & handle
        boneElem->SetAttribute("id", 
            StringConverter::toString(pBone->getHandle()));
        boneElem->SetAttribute("name", pBone->getName());

        // Position
        TiXmlElement* subNode = 
            boneElem->InsertEndChild(TiXmlElement("position"))->ToElement();
        Vector3 pos = pBone->getPosition();
        subNode->SetAttribute("x", StringConverter::toString(pos.x));
        subNode->SetAttribute("y", StringConverter::toString(pos.y));
        subNode->SetAttribute("z", StringConverter::toString(pos.z));
        
        // Orientation 
        subNode = 
            boneElem->InsertEndChild(TiXmlElement("rotation"))->ToElement();
        // Show Quaternion as angle / axis
        Radian angle;
        Vector3 axis;
        pBone->getOrientation().ToAngleAxis(angle, axis);
        TiXmlElement* axisNode = 
            subNode->InsertEndChild(TiXmlElement("axis"))->ToElement();
        subNode->SetAttribute("angle", StringConverter::toString(angle.valueRadians()));
        axisNode->SetAttribute("x", StringConverter::toString(axis.x));
        axisNode->SetAttribute("y", StringConverter::toString(axis.y));
        axisNode->SetAttribute("z", StringConverter::toString(axis.z));

        // Scale optional
        Vector3 scale = pBone->getScale();
        if (scale != Vector3::UNIT_SCALE)
        {
            TiXmlElement* scaleNode =
                boneElem->InsertEndChild(TiXmlElement("scale"))->ToElement();
            scaleNode->SetAttribute("x", StringConverter::toString(scale.x));
            scaleNode->SetAttribute("y", StringConverter::toString(scale.y));
            scaleNode->SetAttribute("z", StringConverter::toString(scale.z));
        }


    }
    //---------------------------------------------------------------------
	// 
	// Modifications effectuées:
	//
	// on stoque les noms et pas les Id. c'est plus lisibles.


    void XMLSkeletonSerializer::writeBoneParent(TiXmlElement* boneHierarchyNode, 
        String boneName, String parentName)
    {
        TiXmlElement* boneParentNode = 
            boneHierarchyNode->InsertEndChild(TiXmlElement("boneparent"))->ToElement();
		/*
	    boneParentNode->SetAttribute("boneid", StringConverter::toString(boneId));
        boneParentNode->SetAttribute("parentid", StringConverter::toString(parentId));
		*/
		// Modifications: on stoque les noms./ 
		boneParentNode->SetAttribute("bone", boneName);
        boneParentNode->SetAttribute("parent", parentName);

    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeAnimation(TiXmlElement* animsNode, 
        const Animation* anim)
    {
        TiXmlElement* animNode = 
            animsNode->InsertEndChild(TiXmlElement("animation"))->ToElement();

        animNode->SetAttribute("name", anim->getName());
        animNode->SetAttribute("length", StringConverter::toString(anim->getLength()));
		
		// Optional base keyframe information
		if (anim->getUseBaseKeyFrame())
		{
			TiXmlElement* baseInfoNode = 
				animNode->InsertEndChild(TiXmlElement("baseinfo"))->ToElement();
			baseInfoNode->SetAttribute("baseanimationname", anim->getBaseKeyFrameAnimationName());
			baseInfoNode->SetAttribute("basekeyframetime", StringConverter::toString(anim->getBaseKeyFrameTime()));
		}

        // Write all tracks
        TiXmlElement* tracksNode = 
            animNode->InsertEndChild(TiXmlElement("tracks"))->ToElement();

        Animation::NodeTrackIterator trackIt = anim->getNodeTrackIterator();
        while (trackIt.hasMoreElements())
        {
            writeAnimationTrack(tracksNode, trackIt.getNext());
        }

    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeAnimationTrack(TiXmlElement* tracksNode, 
        const NodeAnimationTrack* track)
    {
        TiXmlElement* trackNode = 
            tracksNode->InsertEndChild(TiXmlElement("track"))->ToElement();
        
        
        // unsigned short boneIndex     : Index of bone to apply to
        Bone* bone = (Bone*)track->getAssociatedNode();
        //unsigned short boneid = bone->getHandle();
		String boneName = bone->getName();
        trackNode->SetAttribute("bone", boneName);

        // Write all keyframes
        TiXmlElement* keysNode = 
            trackNode->InsertEndChild(TiXmlElement("keyframes"))->ToElement();
        for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
        {
            writeKeyFrame(keysNode, track->getNodeKeyFrame(i));
        }
    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeKeyFrame(TiXmlElement* keysNode, 
		const TransformKeyFrame* key)
    {
        TiXmlElement* keyNode = 
            keysNode->InsertEndChild(TiXmlElement("keyframe"))->ToElement();

        keyNode->SetAttribute("time", StringConverter::toString(key->getTime()));

        TiXmlElement* transNode = 
            keyNode->InsertEndChild(TiXmlElement("translate"))->ToElement();
        Vector3 trans = key->getTranslate();
        transNode->SetAttribute("x", StringConverter::toString(trans.x));
        transNode->SetAttribute("y", StringConverter::toString(trans.y));
        transNode->SetAttribute("z", StringConverter::toString(trans.z));

        TiXmlElement* rotNode = 
            keyNode->InsertEndChild(TiXmlElement("rotate"))->ToElement();
        // Show Quaternion as angle / axis
        Radian angle;
        Vector3 axis;
        key->getRotation().ToAngleAxis(angle, axis);
        TiXmlElement* axisNode = 
            rotNode->InsertEndChild(TiXmlElement("axis"))->ToElement();
        rotNode->SetAttribute("angle", StringConverter::toString(angle.valueRadians()));
        axisNode->SetAttribute("x", StringConverter::toString(axis.x));
        axisNode->SetAttribute("y", StringConverter::toString(axis.y));
        axisNode->SetAttribute("z", StringConverter::toString(axis.z));

        // Scale optional
        if (key->getScale() != Vector3::UNIT_SCALE)
        {
            TiXmlElement* scaleNode = 
                keyNode->InsertEndChild(TiXmlElement("scale"))->ToElement();

            scaleNode->SetAttribute("x", StringConverter::toString(key->getScale().x));
            scaleNode->SetAttribute("y", StringConverter::toString(key->getScale().y));
            scaleNode->SetAttribute("z", StringConverter::toString(key->getScale().z));
        }

    }
    //---------------------------------------------------------------------
	void XMLSkeletonSerializer::writeSkeletonAnimationLink(TiXmlElement* linksNode, 
		const LinkedSkeletonAnimationSource& link)
	{
		TiXmlElement* linkNode = 
			linksNode->InsertEndChild(TiXmlElement("animationlink"))->ToElement();
		linkNode->SetAttribute("skeletonName", link.skeletonName);
		linkNode->SetAttribute("scale", StringConverter::toString(link.scale));

	}
	//---------------------------------------------------------------------
	void XMLSkeletonSerializer::readSkeletonAnimationLinks(Skeleton* skel, 
		TiXmlElement* linksNode)
	{
		LogManager::getSingleton().logMessage("XMLSkeletonSerializer: Reading Animations links...");

		for (TiXmlElement* linkElem = linksNode->FirstChildElement("animationlink"); 
			linkElem != 0; linkElem = linkElem->NextSiblingElement())
		{
			String skelName = linkElem->Attribute("skeletonName");
			const char* strScale = linkElem->Attribute("scale");
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


