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

#ifndef PCZ_SCENE_NODE_H
#define PCZ_SCENE_NODE_H

#include "OgrePCZPrerequisites.h"
#include "OgreSceneNode.h"
#include "OgreSceneManager.h"

namespace Ogre
{
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup PCZSceneManager
    *  @{
    */
    // forward declarations
    class PCZone;
    class ZoneData;
    class PCZCamera;
    typedef std::map<String, PCZone*> ZoneMap;
    typedef std::map<String, ZoneData*> ZoneDataMap;

    /**
        The PCZSceneNode is an extension used to store zone information and provide
        additional functionality for a given Ogre::SceneNode.  A PCZSceneNode contains
        a pointer to the home zone for the node and a list of all zones being visited by
        the node.  The PCZSceneManager contains a STD::MAP of PCZSceneNodes which are
        keyed by the name of each node (each PCZSceneNode has an identical name to the
        scene node which it is associated with).  This allows quick lookup of
        a given scenenode's PCZSceneNode by the scene manager.
     */
    class _OgrePCZPluginExport PCZSceneNode : public SceneNode
    {
    public:
        /** Standard constructor */
        PCZSceneNode( SceneManager* creator );
        /** Standard constructor */
        PCZSceneNode( SceneManager* creator, const String& name );
        /** Standard destructor */
        ~PCZSceneNode();
        void _update(bool updateChildren, bool parentHasChanged) override;
        void updateFromParentImpl() const override;

        SceneNode* createChildSceneNode(const Vector3& translate = Vector3::ZERO,
                                        const Quaternion& rotate = Quaternion::IDENTITY) override;
        SceneNode* createChildSceneNode(const String& name, const Vector3& translate = Vector3::ZERO,
                                        const Quaternion& rotate = Quaternion::IDENTITY) override;

        PCZone*     getHomeZone(void);
        void        setHomeZone(PCZone * zone);
        void        anchorToHomeZone(PCZone * zone);
        bool        isAnchored(void) {return mAnchored;}
        void        allowToVisit(bool yesno) {mAllowedToVisit = yesno;}
        bool        allowedToVisit(void) {return mAllowedToVisit;}
        void        addZoneToVisitingZonesMap(PCZone * zone);
        void        clearVisitingZonesMap(void);
        void        clearNodeFromVisitedZones( void );
        void        removeReferencesToZone(PCZone * zone);
        bool        isVisitingZone(PCZone * zone);
        void        _addToRenderQueue( Camera* cam, 
                                       RenderQueue *queue, 
                                       bool onlyShadowCasters, 
                                       VisibleObjectsBoundsInfo* visibleBounds );
        void        savePrevPosition(void);
        Vector3&    getPrevPosition(void) {return mPrevPosition;}
        unsigned long       getLastVisibleFrame(void) {return mLastVisibleFrame;}
        void        setLastVisibleFrame(unsigned long newLVF) {mLastVisibleFrame = newLVF;}
        void        setLastVisibleFromCamera(PCZCamera * camera) {mLastVisibleFromCamera = camera;}
        PCZCamera*  getLastVisibleFromCamera() {return mLastVisibleFromCamera;}
        void        setZoneData(PCZone * zone, ZoneData * zoneData);
        ZoneData*   getZoneData(PCZone * zone);
        void        updateZoneData(void);
        void        enable(bool yesno) {mEnabled = yesno;}
        bool        isEnabled(void) {return mEnabled;}
        bool        isMoved(void) {return mMoved;}
        void        setMoved(bool value) {mMoved = value;}
    protected:
        mutable Vector3 mNewPosition; 
        PCZone *        mHomeZone;
        bool            mAnchored;
        bool            mAllowedToVisit;
        ZoneMap         mVisitingZones;
        mutable Vector3 mPrevPosition;
        unsigned long   mLastVisibleFrame;
        PCZCamera*      mLastVisibleFromCamera;
        ZoneDataMap     mZoneData;
        bool            mEnabled;
        mutable bool    mMoved;
    };
    /** @} */
    /** @} */
}

#endif // PCZ_SCENE_NODE_H
