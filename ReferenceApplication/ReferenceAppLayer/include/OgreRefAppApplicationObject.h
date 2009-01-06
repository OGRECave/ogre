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
#ifndef __REFAPP_APPLICATIONOBJECT_H__
#define __REFAPP_APPLICATIONOBJECT_H__

#include "OgreRefAppPrerequisites.h"

namespace OgreRefApp {

    /** This object is the base class for all discrete objects in the application.
    @remarks
        This object holds a reference to the underlying OGRE entity / entities which 
        comprise it, plus links to the additional properties required to make it
        work in the application world.
    @remarks
        It extends the OGRE UserDefinedObject to allow reverse links from Ogre::Entity.
        Note that this class does not override the UserDefinedObject's getTypeId method 
        because this class is abstract.
    */
    class _OgreRefAppExport ApplicationObject : public UserDefinedObject
    {
    protected:
        // Visual component
        SceneNode* mSceneNode;
        Entity* mEntity;

        /// Dynamics properties, must be set up by subclasses if dynamics enabled
        dBody* mOdeBody;
        /// Mass parameters
        dMass mMass;


        /// Collision proxies, must be set up if collision enabled
        typedef list<dGeom*>::type CollisionProxyList;
        CollisionProxyList mCollisionProxies;


        bool mDynamicsEnabled;
        bool mReenableIfInteractedWith;
        bool mCollisionEnabled;

		Real mBounceCoeffRestitution;
		Real mBounceVelocityThreshold;
		Real mSoftness;
        Real mFriction;
        Real mLinearVelDisableThreshold;
        Real mAngularVelDisableThreshold;
        Real mDisableTime;
        Real mDisableTimeEnd;

        // Set up method, must override
        virtual void setUp(const String& name) = 0;
        /** Internal method for updating the state of the collision proxies. */
        virtual void updateCollisionProxies(void);

        /** Internal method for testing the plane bounded region WorldFragment type. */
        virtual bool testCollidePlaneBounds(SceneQuery::WorldFragment* wf);

        /// Internal method for updating the query mask
        virtual void setEntityQueryFlags(void);

    public:
        ApplicationObject(const String& name);
        virtual ~ApplicationObject();

        /** Sets the position of this object. */
        virtual void setPosition(const Vector3& vec);
        /** Sets the position of this object. */
        virtual void setPosition(Real x, Real y, Real z);
        /** Sets the orientation of this object. */
        virtual void setOrientation(const Quaternion& orientation);
        /** Gets the current position of this object. */
        virtual const Vector3& getPosition(void);
        /** Gets the current orientation of this object. */
        virtual const Quaternion& getOrientation(void);

        /// Updates the position of this game object from the simulation
        virtual void _updateFromDynamics(void);

        /// Returns whether or not this object is considered for collision.
        virtual bool isCollisionEnabled(void);
        /** Returns whether or not this object is physically simulated.
        @remarks
            Objects which are not physically simulated only move when their
            SceneNode is manually altered.
        */
        virtual bool isDynamicsEnabled(void);
        /** Sets whether or not this object is considered for collision.
        @remarks
            Objects which have collision enabled must set up an ODE 
            collision proxy as part of their setUp method.
        */

        /** Sets the linear and angular velocity thresholds, below which the 
        object will have it's dynamics automatically disabled for performance.
        @remarks
            These thresholds are used to speed up the simulation and to make it more
            stable, by turning off dynamics for objects that appear to be at rest.
            Otherwise, objects which are supposedly stationary can jitter when involved
            in large stacks, and can consume unnecessary CPU time. Note that if another
            object interacts with the disabled object, it will automatically reenable itself.
        @par
            If you never want to disable dynamics automatically for this object, just 
            set all the values to 0.
        @param linearSq The squared linear velocity magnitude threshold
        @param angularSq The squared angular velocity magnitude threshold
        @param overTime The number of seconds over which the values must continue to be under
            this threshold for the dynamics to be disabled. This is to catch cases
            where the object almost stops moving because of a boundary condition, but
            would speed up again later (e.g. box teetering on an edge).

        */
        virtual void setDynamicsDisableThreshold(Real linearSq, Real angularSq, Real overTime);

        virtual void setCollisionEnabled(bool enabled);
        /** Sets whether or not this object is physically simulated at this time.
        @remarks
            Objects which are not physically simulated only move when their
            SceneNode is manually altered. Objects which are physically 
            simulated must set up an ODE body as part of their setUp method.
        @par
            You can also use this to temporarily turn off simulation on an object,
            such that it is not simulated until some other object which IS simulated
            comes in contact with it, or is attached to it with a joint.
        @param enabled Specifies whether dynamics is enabled
        @param reEnableOnInteraction If set to true, this object will reenable if some
            other dynamically simulated object interacts with it
        */
        virtual void setDynamicsEnabled(bool enabled, bool reEnableOnInteraction = false);

        /** Sets the 'bounciness' of this object.
		 * @remarks
		 * Only applies if this object has both collision and dynamics enabled.
		 * When 2 movable objects collide, the greatest bounce parameters 
		 * from both objects apply, so even a non-bouncy object can
		 * bounce if it hits a bouncy surface. 
		 * @param restitutionValue Coeeficient of restitution 
		 * 		(0 for no bounce, 1 for perfect bounciness)
		 * @param velocityThreshold Velocity below which no bounce will occur; 
		 * 		this is a dampening value to ensure small velocities do not
		 * 		cause bounce.
		 */
		virtual void setBounceParameters(Real restitutionValue,	Real velocityThreshold);
		/** Gets the cefficient of restitution (bounciness) for this object. */
		virtual Real getBounceRestitutionValue(void);
		/** Gets the bounce velocity threshold for this object. */
		virtual Real getBounceVelocityThreshold(void);

		/** Sets the softness of this object, which determines how much it is allowed to 
		 * penetrate other objects.
		 * @remarks
		 * 	This parameter only has meaning if collision and dynamics are enabled for this object.
		 * 	@param softness Softness factor (0 is completely hard). Softness will be combined from
		 * 		both objects involved in a collision to determine how much they will penetrate.
		 */
		virtual void setSoftness(Real softness);
		/** Gets the softness factor of this object. */
		virtual Real getSoftness(void);

        /** Sets the Coulomb frictional coefficient for this object.
        @remarks
            This coefficient affects how much an object will slip when it comes
            into contact with another object. 
        @param friction The Coulomb friction coefficient, valid from 0 to Math::POS_INFINITY.
            0 means no friction, Math::POS_INFINITY means infinite friction ie no slippage.
            Note that friction between these 2 bounds is more CPU intensive so use with caution.
        */
        virtual void setFriction(Real friction);
        /** Gets the Coulomb frictional coefficient for this object. */
        virtual Real getFriction(void);
        /** Adds a linear force to this object, in object space, at the position indicated. 
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param direction The force direction in object coordinates.
        @param atPosition The position at which the force is to be applied, in object coordinates.
        */
        virtual void addForce(const Vector3& direction, const Vector3& atPosition = Vector3::ZERO);
        /** Adds a linear force to this object, in object space, at the position indicated. 
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param dir_x, dir_y, dir_z The force direction in object coordinates.
        @param pos_x, pos_y, pos_z The position at which the force is to be applied, in object coordinates.
        */
        virtual void addForce(Real dir_x, Real dir_y, Real dir_z, 
            Real pos_x = 0, Real pos_y = 0, Real pos_z = 0);
        /** Adds a linear force to this object, in world space, at the position indicated. 
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param direction The force direction in world coordinates.
        @param atPosition The position at which the force is to be applied, in world coordinates.
        */
        virtual void addForceWorldSpace(const Vector3& direction, const Vector3& atPosition = Vector3::ZERO);
        /** Adds a linear force to this object, in world space, at the position indicated. 
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param dir_x, dir_y, dir_z The force direction in world coordinates.
        @param pos_x, pos_y, pos_z The position at which the force is to be applied, in world coordinates.
        */
        virtual void addForceWorldSpace(Real dir_x, Real dir_y, Real dir_z, 
        Real pos_x, Real pos_y, Real pos_z);
        /** Adds rotational force to this object, in object space.
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param direction The direction of the torque to apply, in object space. */
        virtual void addTorque(const Vector3& direction);
        /** Adds rotational force to this object, in object space.
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param x, y, z The direction of the torque to apply, in object space. */
        virtual void addTorque(Real x, Real y, Real z);
        /** Adds rotational force to this object, in world space.
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param direction The direction of the torque to apply, in world space. */
        virtual void addTorqueWorldSpace(const Vector3& direction);
        /** Adds rotational force to this object, in world space.
        @remarks
            All forces are applied, then reset after World::applyDynamics is called. 
        @param x, y, z The direction of the torque to apply, in world space. */
        virtual void addTorqueWorldSpace(Real x, Real y, Real z);

        /** Tests to see if there is a detailed collision between this object and the object passed in.
        @remarks
            If there is a collision, both objects will be notified and if dynamics are enabled
            on these objects, physics will be applied automatically.
        @returns true if collision occurred

        */
        virtual bool testCollide(ApplicationObject* otherObj);

        /** Tests to see if there is a detailed collision between this object and the 
            world fragment passed in.
        @remarks
            If there is a collision, the object will be notified and if dynamics are enabled
            on this object, physics will be applied automatically.
        @returns true if collision occurred
        */
        virtual bool testCollide(SceneQuery::WorldFragment* wf);

        /** Contains information about a collision; used in the _notifyCollided call. */
        struct CollisionInfo
        {
            /// The position in world coordinates at which the collision occurred
            Vector3 position;
            /// The normal in world coordinates of the collision surface
            Vector3 normal;
            /// Penetration depth 
            Real penetrationDepth;
        };
        /** This method is called automatically if testCollide indicates a real collision. 
        */
        virtual void _notifyCollided(ApplicationObject* otherObj, const CollisionInfo& info);
        /** This method is called automatically if testCollide indicates a real collision. 
        */
        virtual void _notifyCollided(SceneQuery::WorldFragment* wf, const CollisionInfo& info);
        /** Gets the SceneNode which is being used to represent this object's position in 
            the OGRE world. */
        SceneNode* getSceneNode(void);
        /** Gets the Entity which is being used to represent this object in the OGRE world. */
        Entity* getEntity(void);
        /** Gets the ODE body used to represent this object's mass and current velocity. */
        dBody* getOdeBody(void);

        /** Set the mass parameters of this object to represent a sphere.
        @remarks
            This method sets the mass and inertia properties of this object such
            that it is like a sphere, ie center of gravity at the origin and
            an even distribution of mass in all directions.
        @param density Density of the sphere in Kg/m^3
        @param radius of the sphere mass
        */
        void setMassSphere(Real density, Real radius);

        /** Set the mass parameters of this object to represent a box. 
        @remarks
            This method sets the mass and inertia properties of this object such
            that it is like a box.
        @param density Density of the box in Kg/m^3
        @param dimensions Width, height and depth of the box.
        @param orientation Optional orientation of the box.
        */
        void setMassBox(Real density, const Vector3& dimensions, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Set the mass parameters of this object to represent a capped cylinder. 
        @remarks
            This method sets the mass and inertia properties of this object such
            that it is like a capped cylinder, by default lying along the Z-axis.
        @param density Density of the cylinder in Kg/m^3
        @param length Length of the cylinder
        @param width Width of the cylinder
        @param orientation Optional orientation if you wish the cylinder to lay 
            along a different axis from Z.
        */
        void setMassCappedCylinder(Real density, Real length, Real width, 
            const Quaternion& orientation = Quaternion::IDENTITY);

        /** Sets the mass parameters manually, use only if you know how!
        @param mass Mass in Kg
        @param center The center of gravity
        @param inertia The inertia matrix describing distribution of the mass around the body.
        */
        void setMassExpert(Real mass, const Vector3 center, const Matrix3 inertia);

        /** Gets the ODE mass parameters for this object. */
        const dMass* getOdeMass(void);

        /** Sets the current linear velocity of this object.
        @remarks
            Only applicable if dynamics are enabled for this object. This method is useful
            for starting an object off at a particular speed rather than applying forces to get 
            it there.
        */ 
        void setLinearVelocity(const Vector3& vel);
        /** Sets the current linear velocity of this object.
        @remarks
            Only applicable if dynamics are enabled for this object. This method is useful
            for starting an object off at a particular speed rather than applying forces to get 
            it there.
        */ 
        void setLinearVelocity(Real x, Real y, Real z);
        /** Gets the current linear velocity of this object.
        @remarks
            Only applicable if dynamics are enabled for this object.
        @returns Vector3 representing the velocity in units per second.
        */
        const Vector3& getLinearVelocity(void);

        /** Gets the current angular velocity of this object.
        @remarks
            Only applicable if dynamics are enabled for this object.
        @returns Vector3 representing the angular velocity in units per second around each axis.
        */
        const Vector3& getAngularVelocity(void);

        /** Sets the current angular velocity of this object.
        @remarks
            Only applicable if dynamics are enabled for this object. This method is useful
            for starting an object off rather than applying forces to get 
            it there.
        */ 
        void setAngularVelocity(const Vector3& vel);
        /** Sets the current angular velocity of this object.
        @remarks
            Only applicable if dynamics are enabled for this object. This method is useful
            for starting an object off rather than applying forces to get 
            it there.
        */ 
        void setAngularVelocity(Real x, Real y, Real z);

        /** Moves the object along it's local  axes.
            @par
                This method moves the object by the supplied vector along the
                local axes of the obect.
            @param 
                d Vector with x,y,z values representing the translation.
        */
        virtual void translate(const Vector3& d);
        /** Moves the object along it's local axes.
            @par
                This method moves the object by the supplied vector along the
                local axes of the obect.
            @param x, y z Real x, y and z values representing the translation.
        */
        virtual void translate(Real x, Real y, Real z);

        /** Moves the object along the world axes.
            @par
                This method moves the object by the supplied vector along the
                world axes.
            @param 
                d Vector with x,y,z values representing the translation.
        */
        virtual void translateWorldSpace(const Vector3& d);
        /** Moves the object along the world axes.
            @par
                This method moves the object by the supplied vector along the
                local axes of the obect.
            @param x, y z Real x, y and z values representing the translation.
        */
        virtual void translateWorldSpace(Real x, Real y, Real z);

        /** Rotate the object around the local Z-axis.
        */
        virtual void roll(const Radian& angle);

        /** Rotate the object around the local X-axis.
        */
        virtual void pitch(const Radian& angle);

        /** Rotate the object around the local Y-axis.
        */
        virtual void yaw(const Radian& angle);

        /** Rotate the object around an arbitrary axis.
        */
        virtual void rotate(const Vector3& axis, const Radian& angle);

        /** Rotate the object around an aritrary axis using a Quarternion.
        */
        virtual void rotate(const Quaternion& q);


    };


} // namespace

#endif


