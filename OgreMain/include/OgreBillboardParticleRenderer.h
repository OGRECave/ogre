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
#ifndef __BillboardParticleRenderer_H__
#define __BillboardParticleRenderer_H__

#include "OgrePrerequisites.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreBillboardSet.h"

namespace Ogre {

    /** Specialisation of ParticleSystemRenderer to render particles using 
        a BillboardSet. 
    @remarks
        This renderer has a few more options than the standard particle system,
        which will be passed to it automatically when the particle system itself
        does not understand them.
    */
    class _OgreExport BillboardParticleRenderer : public ParticleSystemRenderer
    {
    protected:
        /// The billboard set that's doing the rendering
        BillboardSet* mBillboardSet;
    public:
        BillboardParticleRenderer();
        ~BillboardParticleRenderer();

        /** Command object for billboard type (see ParamCommand).*/
        class _OgrePrivate CmdBillboardType : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for billboard origin (see ParamCommand).*/
        class _OgrePrivate CmdBillboardOrigin : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for billboard rotation type (see ParamCommand).*/
        class _OgrePrivate CmdBillboardRotationType : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for common direction (see ParamCommand).*/
        class _OgrePrivate CmdCommonDirection : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for common up-vector (see ParamCommand).*/
        class _OgrePrivate CmdCommonUpVector : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for point rendering (see ParamCommand).*/
        class _OgrePrivate CmdPointRendering : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
		/** Command object for accurate facing(see ParamCommand).*/
		class _OgrePrivate CmdAccurateFacing : public ParamCommand
		{
		public:
			String doGet(const void* target) const;
			void doSet(void* target, const String& val);
		};

        /** Sets the type of billboard to render.
        @remarks
            The default sort of billboard (BBT_POINT), always has both x and y axes parallel to 
            the camera's local axes. This is fine for 'point' style billboards (e.g. flares,
            smoke, anything which is symmetrical about a central point) but does not look good for
            billboards which have an orientation (e.g. an elongated raindrop). In this case, the
            oriented billboards are more suitable (BBT_ORIENTED_COMMON or BBT_ORIENTED_SELF) since they retain an independant Y axis
            and only the X axis is generated, perpendicular to both the local Y and the camera Z.
        @param bbt The type of billboard to render
        */
        void setBillboardType(BillboardType bbt);

        /** Returns the billboard type in use. */
        BillboardType getBillboardType(void) const;

		/// @copydoc BillboardSet::setUseAccurateFacing
		void setUseAccurateFacing(bool acc);
		/// @copydoc BillboardSet::getUseAccurateFacing
		bool getUseAccurateFacing(void) const;

        /** Sets the point which acts as the origin point for all billboards in this set.
        @remarks
        This setting controls the fine tuning of where a billboard appears in relation to it's
        position. It could be that a billboard's position represents it's center (e.g. for fireballs),
        it could mean the center of the bottom edge (e.g. a tree which is positioned on the ground),
        the top-left corner (e.g. a cursor).
        @par
        The default setting is BBO_CENTER.
        @param
        origin A member of the BillboardOrigin enum specifying the origin for all the billboards in this set.
        */
        void setBillboardOrigin(BillboardOrigin origin) { mBillboardSet->setBillboardOrigin(origin); }

        /** Gets the point which acts as the origin point for all billboards in this set.
        @returns
        A member of the BillboardOrigin enum specifying the origin for all the billboards in this set.
        */
        BillboardOrigin getBillboardOrigin(void) const { return mBillboardSet->getBillboardOrigin(); }

        /** Sets billboard rotation type.
            @remarks
                This setting controls the billboard rotation type, you can deciding rotate the billboard's vertices
                around their facing direction or rotate the billboard's texture coordinates.
            @par
                The default settings is BBR_TEXCOORD.
            @param
                rotationType A member of the BillboardRotationType enum specifying the rotation type for all the billboards in this set.
        */
        void setBillboardRotationType(BillboardRotationType rotationType);

        /** Sets billboard rotation type.
            @returns
                A member of the BillboardRotationType enum specifying the rotation type for all the billboards in this set.
        */
        BillboardRotationType getBillboardRotationType(void) const;

        /** Use this to specify the common direction given to billboards of type BBT_ORIENTED_COMMON.
        @remarks
            Use BBT_ORIENTED_COMMON when you want oriented billboards but you know they are always going to 
            be oriented the same way (e.g. rain in calm weather). It is faster for the system to calculate
            the billboard vertices if they have a common direction.
        @param vec The direction for all billboards.
        */
        void setCommonDirection(const Vector3& vec);

        /** Gets the common direction for all billboards (BBT_ORIENTED_COMMON) */
        const Vector3& getCommonDirection(void) const;

        /** Use this to specify the common up-vector given to billboards of type BBT_PERPENDICULAR_SELF.
        @remarks
            Use BBT_PERPENDICULAR_SELF when you want oriented billboards perpendicular to their own
            direction vector and doesn't face to camera. In this case, we need an additional vector
            to determine the billboard X, Y axis. The generated X axis perpendicular to both the own
            direction and up-vector, the Y axis will coplanar with both own direction and up-vector,
            and perpendicular to own direction.
        @param vec The up-vector for all billboards.
        */
        void setCommonUpVector(const Vector3& vec);

        /** Gets the common up-vector for all billboards (BBT_PERPENDICULAR_SELF) */
        const Vector3& getCommonUpVector(void) const;

		/// @copydoc BillboardSet::setPointRenderingEnabled
		void setPointRenderingEnabled(bool enabled);

		/// @copydoc BillboardSet::isPointRenderingEnabled
		bool isPointRenderingEnabled(void) const;



        /// @copydoc ParticleSystemRenderer::getType
        const String& getType(void) const;
        /// @copydoc ParticleSystemRenderer::_updateRenderQueue
        void _updateRenderQueue(RenderQueue* queue, 
            std::list<Particle*>& currentParticles, bool cullIndividually);
		/// @copydoc ParticleSystemRenderer::visitRenderables
		void visitRenderables(Renderable::Visitor* visitor, 
			bool debugRenderables = false);
        /// @copydoc ParticleSystemRenderer::_setMaterial
        void _setMaterial(MaterialPtr& mat);
        /// @copydoc ParticleSystemRenderer::_notifyCurrentCamera
        void _notifyCurrentCamera(Camera* cam);
        /// @copydoc ParticleSystemRenderer::_notifyParticleRotated
        void _notifyParticleRotated(void);
        /// @copydoc ParticleSystemRenderer::_notifyParticleResized
        void _notifyParticleResized(void);
        /// @copydoc ParticleSystemRenderer::_notifyParticleQuota
        void _notifyParticleQuota(size_t quota);
        /// @copydoc ParticleSystemRenderer::_notifyAttached
        void _notifyAttached(Node* parent, bool isTagPoint = false);
        /// @copydoc ParticleSystemRenderer::_notifyDefaultDimensions
        void _notifyDefaultDimensions(Real width, Real height);
		/// @copydoc ParticleSystemRenderer::setRenderQueueGroup
		void setRenderQueueGroup(uint8 queueID);
		/// @copydoc ParticleSystemRenderer::setKeepParticlesInLocalSpace
		void setKeepParticlesInLocalSpace(bool keepLocal);
        /// @copydoc ParticleSystemRenderer::_getSortMode
        SortMode _getSortMode(void) const;

		/// Access BillboardSet in use
		BillboardSet* getBillboardSet(void) const { return mBillboardSet; }

    protected:
        static CmdBillboardType msBillboardTypeCmd;
        static CmdBillboardOrigin msBillboardOriginCmd;
        static CmdBillboardRotationType msBillboardRotationTypeCmd;
        static CmdCommonDirection msCommonDirectionCmd;
        static CmdCommonUpVector msCommonUpVectorCmd;
        static CmdPointRendering msPointRenderingCmd;
		static CmdAccurateFacing msAccurateFacingCmd;


    };

    /** Factory class for BillboardParticleRenderer */
    class _OgreExport BillboardParticleRendererFactory : public ParticleSystemRendererFactory
    {
    public:
        /// @copydoc FactoryObj::getType
        const String& getType() const;
        /// @copydoc FactoryObj::createInstance
        ParticleSystemRenderer* createInstance( const String& name );    
        /// @copydoc FactoryObj::destroyInstance
        void destroyInstance( ParticleSystemRenderer* inst);    
    };

}

#endif

