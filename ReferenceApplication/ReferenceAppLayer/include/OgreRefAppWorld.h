/*
-----------------------------------------------------------------------------
This source file is part of the OGRE Reference Application, a layer built
on top of OGRE(Object-oriented Graphics Rendering Engine)
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
#ifndef __REFAPP_WORLD_H__
#define __REFAPP_WORLD_H__

#include "OgreRefAppPrerequisites.h"
#include "OgreRefAppJoint.h"
#include <OgreSingleton.h>

namespace OgreRefApp {

    class _OgreRefAppExport World : public Singleton<World>
    {
    public:
        /// World type, you'll want to extend this for your own apps
        enum WorldType {
            WT_REFAPP_GENERIC,
            WT_REFAPP_BSP
        };
    protected:
        /// Pointer to OGRE's scene manager
        SceneManager* mSceneMgr;

        typedef map<String, ApplicationObject*>::type ObjectMap;
        /// Main list of objects
        ObjectMap mObjects;

        typedef map<String, Joint*>::type JointMap;
        JointMap mJoints;

        typedef set<ApplicationObject*>::type ObjectSet;
        /// Set of dynamics objects (those to perform physics on)
        ObjectSet mDynamicsObjects;

        // ODE world object
        dWorld* mOdeWorld;

        /// Contact joint group
        dJointGroup* mOdeContactGroup;

        Vector3 mGravity;

        IntersectionSceneQuery* mIntersectionQuery;

        /// The step size of the collision / physics simulation
        Real mSimulationStepSize;

        /// The type of world we're dealing with
        WorldType mWorldType;

    public:
        /** Creates an instance of the world. 
        @param sceneMgr Pointer to the scene manager which will manage the scene
        @param worldType The type of world being used
        */
        World(SceneManager* sceneMgr, WorldType worldType = WT_REFAPP_GENERIC);
        ~World();

        /// Get the scene manager for this world
        SceneManager* getSceneManager(void);

        /** Create an OGRE head object. */
        OgreHead* createOgreHead(const String& name, const Vector3& pos = Vector3::ZERO, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Create a plane object. */
        FinitePlane* createPlane(const String& name, Real width, Real height, const Vector3& pos = Vector3::ZERO, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Create a ball object. */
        Ball* createBall(const String& name, Real radius, const Vector3& pos = Vector3::ZERO, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Create a box object. */
        Box* createBox(const String& name, Real width, Real height, Real depth,
            const Vector3& pos = Vector3::ZERO, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Create a camera which interacts with the world. */
        CollideCamera* createCamera(const String& name, 
            const Vector3& pos = Vector3::ZERO, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Clears the scene. */
        void clear(void);

        dWorld* getOdeWorld(void);
        dJointGroup* getOdeContactJointGroup(void);

        /** Detects all the collisions in the world and acts on them.
        @remarks
            This method performs the appropriate queries to detect all the colliding objects
            in the world, tells the objects about it and adds the appropriate physical simulation
            constructs required to apply collision response when applyDynamics is called.
        @par This method is called automatically by World::simulationStep()
        */
        void _applyCollision(void);

        /** Updates the world simulation. 
        @par This method is called automatically by World::simulationStep()
        */
        void _applyDynamics(Real timeElapsed);

        /** Internal method for notifying the world of a change in the dynamics status of an object. */
        void _notifyDynamicsStateForObject(ApplicationObject* obj, bool dynamicsEnabled);

        /** Sets the gravity vector, units are in m/s^2.
        @remarks
            The world defaults to no gravity.
            Tip: Earth gravity is Vector3(0, -9.81, 0);
        */
        void setGravity(const Vector3& vec);

        /** Gets the gravity vector. */
        const Vector3& getGravity(void);

        /** Creates a Joint object for linking objects together in the world. 
        @param name The name of the Joint.
        @param jtype The type of joint, see Joint::JointType.
        @param obj1 The first object to attach, or NULL to attach to the static world.
        @param obj2 The second object to attach, or NULL to attach to the static world.
        */
        Joint* createJoint(const String& name, Joint::JointType jtype,
            ApplicationObject* obj1, ApplicationObject* obj2);

        /** Sets the step size of the simulation.
        @remarks
            This parameter allows you to alter the accuracy of the simulation. 
            This is the interval at which collision and physics are performed,
            such that in high frame rate scenarios these operations are
            not done every single frame, and in low frame rate situations more
            steps are performed per frame to ensure the stability of the
            simulation.
        @par
            The default value for this parameter is 0.01s.
        */
        void setSimulationStepSize(Real step);
        /** Returns the size of the simulation step. */
        Real getSimulationStepSize(void);

        /** Performs a simulation step, ie applies collision and physics.
        @remarks
            Collision events will cause callbacks to your ApplicationObject
            instances to notify them of the collisions; this is for information,
            dynamics are applied automatically if turned on for the objects so you
            do not need to handle physics yourself if you do not wish to.
        @par
            Note that if the timeElapsed parameter is greater than the simulation
            step size (as set using setSimulationStepSize), more than one collision
            and dynamics step will take place during this call. Similarly, no step
            may occur if the time elapsed has not reached the simulation step
            size yet.
        */
        void simulationStep(Real timeElapsed);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static World& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static World* getSingletonPtr(void);

    };


}

#endif

