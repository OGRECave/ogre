/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __XMLSkeletonSerializer_H__
#define __XMLSkeletonSerializer_H__

#include "OgreXMLPrerequisites.h"
#include "OgreMaterial.h"
#include "OgreSkeleton.h"


namespace Ogre {

    /** Class for serializing a Skeleton to/from XML.
    @remarks
        This class behaves the same way as SkeletonSerializer in the main project,
        but is here to allow conversions to / from XML. This class is 
        deliberately not included in the main project because <UL>
        <LI>Dependence on Xerces would unnecessarily bloat the main library</LI>
        <LI>Runtime use of XML is discouraged because of the parsing overhead</LI></UL>
        This class gives people the option of saving out a Skeleton as XML for examination
        and possible editing. It can then be converted back to the native format
        for maximum runtime efficiency.
    */
    class XMLSkeletonSerializer
    {
    public:

        XMLSkeletonSerializer();
        virtual ~XMLSkeletonSerializer();
        /** Imports a Skeleton from the given XML file.
        @param filename The name of the file to import, expected to be in XML format.
        @param pSkeleton The pre-created Skeleton object to be populated.
        */
        void importSkeleton(const String& filename, Skeleton* pSkeleton);

        /** Exports a skeleton to the named XML file. */
        void exportSkeleton(const Skeleton* pSkeleton, const String& filename);

    private:
        // State for export
        TiXmlDocument* mXMLDoc;
        // State for import
        Skeleton* mpSkel;

        void writeSkeleton(const Skeleton* pSkel);
        void writeBone(TiXmlElement* bonesElement, const Bone* pBone);
        void writeBoneParent(TiXmlElement* boneHierarchyNode, String boneName , String parentName);
        void writeAnimation(TiXmlElement* animsNode, const Animation* anim);
        void writeAnimationTrack(TiXmlElement* tracksNode, 
			const NodeAnimationTrack* track);
        void writeKeyFrame(TiXmlElement* keysNode, const TransformKeyFrame* key);
		void writeSkeletonAnimationLink(TiXmlElement* linksNode, 
			const LinkedSkeletonAnimationSource& link);
		
		void readBones(Skeleton* skel, TiXmlElement* mBonesNode);
		void readBones2(Skeleton* skel, TiXmlElement* mBonesNode);
		void createHierarchy(Skeleton* skel, TiXmlElement* mHierNode);
		void readKeyFrames(NodeAnimationTrack* track, TiXmlElement* mKeyfNode);
		void readAnimations(Skeleton* skel, TiXmlElement* mAnimNode) ;
		void readSkeletonAnimationLinks(Skeleton* skel, TiXmlElement* linksNode);

    };


}

#endif

