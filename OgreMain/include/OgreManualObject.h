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

#ifndef __OgreManualObject_H__
#define __OgreManualObject_H__

#include "OgrePrerequisites.h"
#include "OgreMovableObject.h"
#include "OgreRenderable.h"
#include "OgreResourceGroupManager.h"


namespace Ogre
{
	/** Class providing a much simplified interface to generating manual
	 	objects with custom geometry.
	@remarks
		Building one-off geometry objects manually usually requires getting
		down and dirty with the vertex buffer and vertex declaration API, 
		which some people find a steep learning curve. This class gives you 
		a simpler interface specifically for the purpose of building a 
		3D object simply and quickly. Note that if you intend to instance your
		object you will still need to become familiar with the Mesh class. 
	@par
		This class draws heavily on the interface for OpenGL 
		immediate-mode (glBegin, glVertex, glNormal etc), since this
		is generally well-liked by people. There are a couple of differences
		in the results though - internally this class still builds hardware 
		buffers which can be re-used, so you can render the resulting object
		multiple times without re-issuing all the same commands again. 
		Secondly, the rendering is not immediate, it is still queued just like
		all OGRE objects. This makes this object more efficient than the 
		equivalent GL immediate-mode commands, so it's feasible to use it for
		large objects if you really want to.
	@par
		To construct some geometry with this object:
		  -# If you know roughly how many vertices (and indices, if you use them)
		     you're going to submit, call estimateVertexCount and estimateIndexCount.
			 This is not essential but will make the process more efficient by saving
			 memory reallocations.
		  -# Call begin() to begin entering data
		  -# For each vertex, call position(), normal(), textureCoord(), colour()
		     to define your vertex data. Note that each time you call position()
			 you start a new vertex. Note that the first vertex defines the 
			 components of the vertex - you can't add more after that. For example
			 if you didn't call normal() in the first vertex, you cannot call it
			 in any others. You ought to call the same combination of methods per
			 vertex.
		  -# If you want to define triangles (or lines/points) by indexing into the vertex list, 
			 you can call index() as many times as you need to define them.
			 If you don't do this, the class will assume you want triangles drawn
			 directly as defined by the vertex list, i.e. non-indexed geometry. Note
			 that stencil shadows are only supported on indexed geometry, and that
			 indexed geometry is a little faster; so you should try to use it.
		  -# Call end() to finish entering data.
		  -# Optionally repeat the begin-end cycle if you want more geometry 
		  	using different rendering operation types, or different materials
	    After calling end(), the class will organise the data for that section
		internally and make it ready to render with. Like any other 
		MovableObject you should attach the object to a SceneNode to make it 
		visible. Other aspects like the relative render order can be controlled
		using standard MovableObject methods like setRenderQueueGroup.
	@par
		You can also use beginUpdate() to alter the geometry later on if you wish.
		If you do this, you should call setDynamic(true) before your first call 
		to begin(), and also consider using estimateVertexCount / estimateIndexCount
		if your geometry is going to be growing, to avoid buffer recreation during
		growth.
	@par
		Note that like all OGRE geometry, triangles should be specified in 
		anti-clockwise winding order (whether you're doing it with just
		vertices, or using indexes too). That is to say that the front of the
		face is the one where the vertices are listed in anti-clockwise order.
	*/
	class _OgreExport ManualObject : public MovableObject
	{
	public:
		ManualObject(const String& name);
		virtual ~ManualObject();

		//pre-declare ManualObjectSection
		class ManualObjectSection;

		/** Completely clear the contents of the object.
		@remarks
			Clearing the contents of this object and rebuilding from scratch
			is not the optimal way to manage dynamic vertex data, since the 
			buffers are recreated. If you want to keep the same structure but
			update the content within that structure, use beginUpdate() instead 
			of clear() begin(). However if you do want to modify the structure 
			from time to time you can do so by clearing and re-specifying the data.
		*/
		virtual void clear(void);
		
		/** Estimate the number of vertices ahead of time.
		@remarks
			Calling this helps to avoid memory reallocation when you define
			vertices. Also very handy when using beginUpdate() to manage dynamic
			data - you can make the vertex buffers a little larger than their
			initial needs to allow for growth later with this method.
		*/
		virtual void estimateVertexCount(size_t vcount);

		/** Estimate the number of indices ahead of time.
		@remarks
			Calling this helps to avoid memory reallocation when you define
			indices. Also very handy when using beginUpdate() to manage dynamic
			data - you can make the index buffer a little larger than the
			initial need to allow for growth later with this method.
		*/
		virtual void estimateIndexCount(size_t icount);

		/** Start defining a part of the object.
		@remarks
			Each time you call this method, you start a new section of the
			object with its own material and potentially its own type of
			rendering operation (triangles, points or lines for example).
		@param materialName The name of the material to render this part of the
			object with.
		@param opType The type of operation to use to render. 
		*/
		virtual void begin(const String& materialName, 
			RenderOperation::OperationType opType = RenderOperation::OT_TRIANGLE_LIST);

		/** Use before defining geometry to indicate that you intend to update the
			geometry regularly and want the internal structure to reflect that.
		*/
		virtual void setDynamic(bool dyn) { mDynamic = dyn; }
		/** Gets whether this object is marked as dynamic */
		virtual bool getDynamic() const { return mDynamic; }

		/** Start the definition of an update to a part of the object.
		@remarks
			Using this method, you can update an existing section of the object
			efficiently. You do not have the option of changing the operation type
			obviously, since it must match the one that was used before. 
		@note If your sections are changing size, particularly growing, use
			estimateVertexCount and estimateIndexCount to pre-size the buffers a little
			larger than the initial needs to avoid buffer reconstruction.
		@param sectionIndex The index of the section you want to update. The first
			call to begin() would have created section 0, the second section 1, etc.
		*/
		virtual void beginUpdate(size_t sectionIndex);
		/** Add a vertex position, starting a new vertex at the same time. 
		@remarks A vertex position is slightly special among the other vertex data
			methods like normal() and textureCoord(), since calling it indicates
			the start of a new vertex. All other vertex data methods you call 
			after this are assumed to be adding more information (like normals or
			texture coordinates) to the last vertex started with position().
		*/
		virtual void position(const Vector3& pos);
		/// @copydoc ManualObject::position(const Vector3&)
		virtual void position(Real x, Real y, Real z);

		/** Add a vertex normal to the current vertex.
		@remarks
			Vertex normals are most often used for dynamic lighting, and 
			their components should be normalised.
		*/
		virtual void normal(const Vector3& norm);
		/// @copydoc ManualObject::normal(const Vector3&)
		virtual void normal(Real x, Real y, Real z);

		/** Add a texture coordinate to the current vertex.
		@remarks
			You can call this method multiple times between position() calls
			to add multiple texture coordinates to a vertex. Each one can have
			between 1 and 3 dimensions, depending on your needs, although 2 is
			most common. There are several versions of this method for the 
			variations in number of dimensions.
		*/
		virtual void textureCoord(Real u);
		/// @copydoc ManualObject::textureCoord(Real)
		virtual void textureCoord(Real u, Real v);
		/// @copydoc ManualObject::textureCoord(Real)
		virtual void textureCoord(Real u, Real v, Real w);
		/// @copydoc ManualObject::textureCoord(Real)
		virtual void textureCoord(Real x, Real y, Real z, Real w);
		/// @copydoc ManualObject::textureCoord(Real)
		virtual void textureCoord(const Vector2& uv);
		/// @copydoc ManualObject::textureCoord(Real)
		virtual void textureCoord(const Vector3& uvw);
		/// @copydoc ManualObject::textureCoord(Real)
		virtual void textureCoord(const Vector4& xyzw);

		/** Add a vertex colour to a vertex.
		*/
		virtual void colour(const ColourValue& col);
		/** Add a vertex colour to a vertex.
		@param r,g,b,a Colour components expressed as floating point numbers from 0-1
		*/
		virtual void colour(Real r, Real g, Real b, Real a = 1.0f);

		/** Add a vertex index to construct faces / lines / points via indexing
			rather than just by a simple list of vertices. 
		@remarks
			You will have to call this 3 times for each face for a triangle list, 
			or use the alternative 3-parameter version. Other operation types
			require different numbers of indexes, @see RenderOperation::OperationType.
		@note
			32-bit indexes are not supported on all cards and will only be used
            when required, if an index is > 65535.
		@param idx A vertex index from 0 to 4294967295. 
		*/
		virtual void index(uint32 idx);
		/** Add a set of 3 vertex indices to construct a triangle; this is a
			shortcut to calling index() 3 times. It is only valid for triangle 
			lists.
		@note
			32-bit indexes are not supported on all cards and will only be used
            when required, if an index is > 65535.
		@param i1, i2, i3 3 vertex indices from 0 to 4294967295 defining a face. 
		*/
		virtual void triangle(uint32 i1, uint32 i2, uint32 i3);
		/** Add a set of 4 vertex indices to construct a quad (out of 2 
			triangles); this is a shortcut to calling index() 6 times, 
			or triangle() twice. It's only valid for triangle list operations.
		@note
			32-bit indexes are not supported on all cards and will only be used
            when required, if an index is > 65535.
		@param i1, i2, i3 3 vertex indices from 0 to 4294967295 defining a face. 
		*/
		virtual void quad(uint32 i1, uint32 i2, uint32 i3, uint32 i4);

		/** Finish defining the object and compile the final renderable version. 
		@note
			Will return a pointer to the finished section or NULL if the section was discarded (i.e. has zero vertices/indices).
		*/
		virtual ManualObjectSection* end(void);

		/** Alter the material for a subsection of this object after it has been
			specified.
		@remarks
			You specify the material to use on a section of this object during the
			call to begin(), however if you want to change the material afterwards
			you can do so by calling this method.
		@param subIndex The index of the subsection to alter
		@param name The name of the new material to use
		*/
		virtual void setMaterialName(size_t subindex, const String& name);

		/** Convert this object to a Mesh. 
		@remarks
			After you've finished building this object, you may convert it to 
			a Mesh if you want in order to be able to create many instances of
			it in the world (via Entity). This is optional, since this instance
			can be directly attached to a SceneNode itself, but of course only
			one instance of it can exist that way. 
		@note Only objects which use indexed geometry may be converted to a mesh.
		@param meshName The name to give the mesh
		@param groupName The resource group to create the mesh in
		*/
		virtual MeshPtr convertToMesh(const String& meshName, 
			const String& groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		/** Sets whether or not to use an 'identity' projection.
		@remarks
			Usually ManualObjects will use a projection matrix as determined
			by the active camera. However, if they want they can cancel this out
			and use an identity projection, which effectively projects in 2D using
			a {-1, 1} view space. Useful for overlay rendering. Normally you don't
			need to change this. The default is false.
		@see ManualObject::getUseIdentityProjection
		*/
		void setUseIdentityProjection(bool useIdentityProjection);

		/** Returns whether or not to use an 'identity' projection.
		@remarks
			Usually ManualObjects will use a projection matrix as determined
			by the active camera. However, if they want they can cancel this out
			and use an identity projection, which effectively projects in 2D using
			a {-1, 1} view space. Useful for overlay rendering. Normally you don't
			need to change this.
		@see ManualObject::setUseIdentityProjection
		*/
		bool getUseIdentityProjection(void) const { return mUseIdentityProjection; }

		/** Sets whether or not to use an 'identity' view.
		@remarks
			Usually ManualObjects will use a view matrix as determined
			by the active camera. However, if they want they can cancel this out
			and use an identity matrix, which means all geometry is assumed
			to be relative to camera space already. Useful for overlay rendering. 
			Normally you don't need to change this. The default is false.
		@see ManualObject::getUseIdentityView
		*/
		void setUseIdentityView(bool useIdentityView);

		/** Returns whether or not to use an 'identity' view.
		@remarks
			Usually ManualObjects will use a view matrix as determined
			by the active camera. However, if they want they can cancel this out
			and use an identity matrix, which means all geometry is assumed
			to be relative to camera space already. Useful for overlay rendering. 
			Normally you don't need to change this.
		@see ManualObject::setUseIdentityView
		*/
		bool getUseIdentityView(void) const { return mUseIdentityView; }

		/** Sets the bounding box.
			@remarks Call this after having finished creating sections to modify the
				bounding box. E.g. if you're using ManualObject to create 2D overlays
				you can call things function to set an infinite bounding box so that
				the object always stays visible when attached.
			@see ManualObject::setUseIdentityProjection, ManualObject::setUseIdentityView,
				AxisAlignedBox::setInfinite */
		void setBoundingBox(const AxisAlignedBox& box) { mAABB = box; }

		/** Gets a pointer to a ManualObjectSection, i.e. a part of a ManualObject.
		*/
		ManualObjectSection* getSection(unsigned int index) const;

		/** Retrieves the number of ManualObjectSection objects making up this ManualObject.
		*/
		unsigned int getNumSections(void) const;
		/** Sets whether or not to keep the original declaration order when 
			queuing the renderables.
		@remarks
			This overrides the default behavior of the rendering queue, 
			specifically stating the desired order of rendering. Might result in a 
			performance loss, but lets the user to have more direct control when 
			creating geometry through this class.
		@param keepOrder Whether to keep the declaration order or not.
		*/
		void setKeepDeclarationOrder(bool keepOrder) { mKeepDeclarationOrder = keepOrder; }

		/** Gets whether or not the declaration order is to be kept or not.
		@return A flag indication if the declaration order will be kept when 
			queuing the renderables.
		*/
		bool getKeepDeclarationOrder() const { return mKeepDeclarationOrder; }
		// MovableObject overrides

		/** @copydoc MovableObject::getMovableType. */
		const String& getMovableType(void) const;
		/** @copydoc MovableObject::getBoundingBox. */
		const AxisAlignedBox& getBoundingBox(void) const;
		/** @copydoc MovableObject::getBoundingRadius. */
		Real getBoundingRadius(void) const;
		/** @copydoc MovableObject::_updateRenderQueue. */
		void _updateRenderQueue(RenderQueue* queue);
		/** Implement this method to enable stencil shadows. */
		EdgeData* getEdgeList(void);
		/** Overridden member from ShadowCaster. */
		bool hasEdgeList(void);
		/** Implement this method to enable stencil shadows. */
		ShadowRenderableListIterator getShadowVolumeRenderableIterator(
			ShadowTechnique shadowTechnique, const Light* light, 
			HardwareIndexBufferSharedPtr* indexBuffer, 
			bool extrudeVertices, Real extrusionDist, unsigned long flags = 0);


		/// Built, renderable section of geometry
		class _OgreExport ManualObjectSection : public Renderable, public MovableAlloc
		{
		protected:
			ManualObject* mParent;
			String mMaterialName;
			mutable MaterialPtr mMaterial;
			RenderOperation mRenderOperation;
			bool m32BitIndices;

			
		public:
			ManualObjectSection(ManualObject* parent, const String& materialName,
				RenderOperation::OperationType opType);
			virtual ~ManualObjectSection();
			
			/// Retrieve render operation for manipulation
			RenderOperation* getRenderOperation(void);
			/// Retrieve the material name in use
			const String& getMaterialName(void) const { return mMaterialName; }
			/// update the material name in use
			void setMaterialName(const String& name);
			/// Set whether we need 32-bit indices
			void set32BitIndices(bool n32) { m32BitIndices = n32; }
			/// Get whether we need 32-bit indices
			bool get32BitIndices() const { return m32BitIndices; }
			
			// Renderable overrides
			/** @copydoc Renderable::getMaterial. */
			const MaterialPtr& getMaterial(void) const;
			/** @copydoc Renderable::getRenderOperation. */
			void getRenderOperation(RenderOperation& op);
			/** @copydoc Renderable::getWorldTransforms. */
			void getWorldTransforms(Matrix4* xform) const;
			/** @copydoc Renderable::getSquaredViewDepth. */
			Real getSquaredViewDepth(const Ogre::Camera *) const;
			/** @copydoc Renderable::getLights. */
			const LightList &getLights(void) const;


					
		};
		/** Nested class to allow shadows. */
		class _OgreExport ManualObjectSectionShadowRenderable : public ShadowRenderable
		{
		protected:
			ManualObject* mParent;
			// Shared link to position buffer
			HardwareVertexBufferSharedPtr mPositionBuffer;
			// Shared link to w-coord buffer (optional)
			HardwareVertexBufferSharedPtr mWBuffer;

		public:
			ManualObjectSectionShadowRenderable(ManualObject* parent, 
				HardwareIndexBufferSharedPtr* indexBuffer, const VertexData* vertexData, 
				bool createSeparateLightCap, bool isLightCap = false);
			~ManualObjectSectionShadowRenderable();
			/// Overridden from ShadowRenderable
			void getWorldTransforms(Matrix4* xform) const;
			HardwareVertexBufferSharedPtr getPositionBuffer(void) { return mPositionBuffer; }
			HardwareVertexBufferSharedPtr getWBuffer(void) { return mWBuffer; }

		};

		typedef std::vector<ManualObjectSection*> SectionList;

		/// @copydoc MovableObject::visitRenderables
		void visitRenderables(Renderable::Visitor* visitor, 
			bool debugRenderables = false);
		
		
	protected:
		/// Dynamic?
		bool mDynamic;
		/// List of subsections
		SectionList mSectionList;
		/// Current section
		ManualObjectSection* mCurrentSection;
		/// Are we updating?
		bool mCurrentUpdating;
		/// Temporary vertex structure
		struct TempVertex
		{
			Vector3 position;
			Vector3 normal;
			Vector4 texCoord[OGRE_MAX_TEXTURE_COORD_SETS];
			ushort texCoordDims[OGRE_MAX_TEXTURE_COORD_SETS];
			ColourValue colour;
		};
		/// Temp storage
		TempVertex mTempVertex;
		/// First vertex indicator
		bool mFirstVertex;
		/// Temp vertex data to copy?
		bool mTempVertexPending;
		/// System-memory buffer whilst we establish the size required
		char* mTempVertexBuffer;
		/// System memory allocation size, in bytes
		size_t mTempVertexSize;
		/// System-memory buffer whilst we establish the size required
		uint32* mTempIndexBuffer;
		/// System memory allocation size, in bytes
		size_t mTempIndexSize;
		/// Current declaration vertex size
		size_t mDeclSize;
		/// Estimated vertex count
		size_t mEstVertexCount;
		/// Estimated index count
		size_t mEstIndexCount;
		/// Current texture coordinate
		ushort mTexCoordIndex;
		/// Bounding box
		AxisAlignedBox mAABB;
		/// Bounding sphere
		Real mRadius;
		/// Any indexed geometry on any sections?
		bool mAnyIndexed;
		/// Edge list, used if stencil shadow casting is enabled 
		EdgeData* mEdgeList;
		/// List of shadow renderables
		ShadowRenderableList mShadowRenderables;
		/// Whether to use identity projection for sections
		bool mUseIdentityProjection;
		/// Whether to use identity view for sections
		bool mUseIdentityView;
		/// Keep declaration order or let the queue optimize it
		bool mKeepDeclarationOrder;


		/// Delete temp buffers and reset init counts
		virtual void resetTempAreas(void);
		/// Resize the temp vertex buffer?
		virtual void resizeTempVertexBufferIfNeeded(size_t numVerts);
		/// Resize the temp index buffer?
		virtual void resizeTempIndexBufferIfNeeded(size_t numInds);

		/// Copy current temp vertex into buffer
		virtual void copyTempVertexToBuffer(void);

	};


	/** Factory object for creating ManualObject instances */
	class _OgreExport ManualObjectFactory : public MovableObjectFactory
	{
	protected:
		MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params);
	public:
		ManualObjectFactory() {}
		~ManualObjectFactory() {}

		static String FACTORY_TYPE_NAME;

		const String& getType(void) const;
		void destroyInstance( MovableObject* obj);  

	};
}

#endif

