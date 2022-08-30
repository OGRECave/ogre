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

#ifndef PCZLIGHT_H
#define PCZLIGHT_H

#include "OgreLight.h"
#include "OgrePCZPrerequisites.h"

namespace Ogre
{
    /** \addtogroup Plugins
    *  @{
    */
    /** \addtogroup PCZSceneManager
    *  @{
    */
    class PCZone;

    typedef std::list<PCZone*> ZoneList;

    /** Specialized version of Ogre::Light which caches which zones the light affects
    */

    class _OgrePCZPluginExport PCZLight : public Light
    {
    public:
        /** Default constructor (for Python mainly).
        */
        PCZLight();

        /** Normal constructor. Should not be called directly, but rather the SceneManager::createLight method should be used.
        */
        PCZLight(const String& name);

        /** Standard destructor.
        */
        ~PCZLight();

        /** Overridden from MovableObject */
        const String& getMovableType(void) const override;

        /** Clear the affectedZonesList 
        */
        void clearAffectedZones(void);

        /** Manually add a zone to the zones affected list
        */
        void addZoneToAffectedZonesList(PCZone * zone);

        /** Check if a zone is in the list of zones affected by the light
        */
        bool affectsZone(PCZone * zone);

        /** @return Flag indicating if the light affects a zone which is visible
        *   in the current frame
        */
        bool affectsVisibleZone(void) {return mAffectsVisibleZone;}

        /** Marks a light as affecting a visible zone */
        void setAffectsVisibleZone(bool affects) { mAffectsVisibleZone = affects; }

        /** Update the list of zones the light affects 
        */
        void updateZones(PCZone * defaultZone, unsigned long frameCount);

        /// Manually remove a zone from the affected list
        void removeZoneFromAffectedZonesList(PCZone * zone);

        /// MovableObject notified when SceneNode changes
        void _notifyMoved(void) override;

        /// Clear update flag
        void clearNeedsUpdate(void)   { mNeedsUpdate = false; } 

        /// Get status of need for update. this checks all affected zones
        bool getNeedsUpdate(void);   

    protected:
        /** Flag indicating if any of the zones in the affectedZonesList is 
        *   visible in the current frame
        */
        bool mAffectsVisibleZone;

        /** List of PCZones which are affected by the light
        */
        ZoneList affectedZonesList;

        /// Flag recording if light has moved, therefore affected list needs updating
        bool mNeedsUpdate;   
    };

    /** Factory object for creating PCZLight instances */
    class _OgrePCZPluginExport PCZLightFactory : public MovableObjectFactory
    {
    protected:
        MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params) override;
    public:
        PCZLightFactory() {}
        ~PCZLightFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const override;
    };
    /** @} */
    /** @} */

} // Namespace
#endif
