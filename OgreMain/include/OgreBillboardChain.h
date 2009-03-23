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

// Thanks to Vincent Cantin (karmaGfa) for the original implementation of this
// class, although it has now been mostly rewritten

#ifndef _BillboardChain_H__
#define _BillboardChain_H__

#include "OgrePrerequisites.h"

#include "OgreMovableObject.h"
#include "OgreRenderable.h"
#include "OgreResourceGroupManager.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/

	/** Allows the rendering of a chain of connected billboards.
	@remarks
		A billboard chain operates much like a traditional billboard, i.e. its
		segments always face the camera; the difference being that instead of
		a set of disconnected quads, the elements in this class are connected
		together in a chain which must always stay in a continuous strip. This
		kind of effect is useful for creating effects such as trails, beams,
		lightning effects, etc.
	@par
		A single instance of this class can actually render multiple separate
		chain segments in a single render operation, provided they all use the
		same material. To clarify the terminology: a 'segment' is a separate 
		sub-part of the chain with its own start and end (called the 'head'
		and the 'tail'. An 'element' is a single position / colour / texcoord
		entry in a segment. You can add items to the head of a chain, and 
		remove them from the tail, very efficiently. Each segment has a max
		size, and if adding an element to the segment would exceed this size, 
		the tail element is automatically removed and re-used as the new item
		on the head.
	@par
		This class has no auto-updating features to do things like alter the
		colour of the elements or to automatically add / remove elements over
		time - you have to do all this yourself as a user of the class. 
		Subclasses can however be used to provide this kind of behaviour 
		automatically. @see RibbonTrail
	*/
	class _OgreExport BillboardChain : public MovableObject, public Renderable
	{

	public:

		/** Contains the data of an element of the BillboardChain.
		*/
		class _OgreExport Element
		{

		public:

			Element();

			Element(Vector3 position,
				Real width,
				Real texCoord,
				ColourValue colour);

			Vector3 position;
			Real width;
			/// U or V texture coord depending on options
			Real texCoord;
			ColourValue colour;

		};
		typedef vector<Element>::type ElementList;

		/** Constructor (don't use directly, use factory) 
		@param name The name to give this object
		@param maxElements The maximum number of elements per chain
		@param numberOfChains The number of separate chain segments contained in this object
		@param useTextureCoords If true, use texture coordinates from the chain elements
		@param useVertexColours If true, use vertex colours from the chain elements
		@param dynamic If true, buffers are created with the intention of being updated
		*/
		BillboardChain(const String& name, size_t maxElements = 20, size_t numberOfChains = 1, 
			bool useTextureCoords = true, bool useColours = true, bool dynamic = true);
		/// destructor
		virtual ~BillboardChain();

		/** Set the maximum number of chain elements per chain 
		*/
		virtual void setMaxChainElements(size_t maxElements);
		/** Get the maximum number of chain elements per chain 
		*/
		virtual size_t getMaxChainElements(void) const { return mMaxElementsPerChain; }
		/** Set the number of chain segments (this class can render multiple chains
			at once using the same material). 
		*/
		virtual void setNumberOfChains(size_t numChains);
		/** Get the number of chain segments (this class can render multiple chains
		at once using the same material). 
		*/
		virtual size_t getNumberOfChains(void) const { return mChainCount; }

		/** Sets whether texture coordinate information should be included in the
			final buffers generated.
		@note You must use either texture coordinates or vertex colour since the
			vertices have no normals and without one of these there is no source of
			colour for the vertices.
		*/
		virtual void setUseTextureCoords(bool use);
		/** Gets whether texture coordinate information should be included in the
			final buffers generated.
		*/
		virtual bool getUseTextureCoords(void) const { return mUseTexCoords; }

		/** The direction in which texture coordinates from elements of the
			chain are used.
		*/
		enum TexCoordDirection
		{
			/// Tex coord in elements is treated as the 'u' texture coordinate
			TCD_U,
			/// Tex coord in elements is treated as the 'v' texture coordinate
			TCD_V
		};
		/** Sets the direction in which texture coords specified on each element
			are deemed to run along the length of the chain.
		@param dir The direction, default is TCD_U.
		*/
		virtual void setTextureCoordDirection(TexCoordDirection dir);
		/** Gets the direction in which texture coords specified on each element
			are deemed to run.
		*/
		virtual TexCoordDirection getTextureCoordDirection(void) { return mTexCoordDir; }

		/** Set the range of the texture coordinates generated across the width of
			the chain elements.
		@param start Start coordinate, default 0.0
		@param end End coordinate, default 1.0
		*/
		virtual void setOtherTextureCoordRange(Real start, Real end);
		/** Get the range of the texture coordinates generated across the width of
			the chain elements.
		*/
		virtual const Real* getOtherTextureCoordRange(void) const { return mOtherTexCoordRange; }

		/** Sets whether vertex colour information should be included in the
			final buffers generated.
		@note You must use either texture coordinates or vertex colour since the
			vertices have no normals and without one of these there is no source of
			colour for the vertices.
		*/
		virtual void setUseVertexColours(bool use);
		/** Gets whether vertex colour information should be included in the
			final buffers generated.
		*/
		virtual bool getUseVertexColours(void) const { return mUseVertexColour; }

		/** Sets whether or not the buffers created for this object are suitable
			for dynamic alteration.
		*/
		virtual void setDynamic(bool dyn);

		/** Gets whether or not the buffers created for this object are suitable
			for dynamic alteration.
		*/
		virtual bool getDynamic(void) const { return mDynamic; }
		
		/** Add an element to the 'head' of a chain.
		@remarks
			If this causes the number of elements to exceed the maximum elements
			per chain, the last element in the chain (the 'tail') will be removed
			to allow the additional element to be added.
		@param chainIndex The index of the chain
		@param billboardChainElement The details to add
		*/
		virtual void addChainElement(size_t chainIndex, 
			const Element& billboardChainElement);
		/** Remove an element from the 'tail' of a chain.
		@param chainIndex The index of the chain
		*/
		virtual void removeChainElement(size_t chainIndex);
		/** Update the details of an existing chain element.
		@param chainIndex The index of the chain
		@param elementIndex The element index within the chain, measured from 
			the 'head' of the chain
		@param billboardChainElement The details to set
		*/
		virtual void updateChainElement(size_t chainIndex, size_t elementIndex, 
			const Element& billboardChainElement);
		/** Get the detail of a chain element.
		@param chainIndex The index of the chain
		@param elementIndex The element index within the chain, measured from
			the 'head' of the chain
		*/
		virtual const Element& getChainElement(size_t chainIndex, size_t elementIndex) const;

		/** Returns the number of chain elements. */
		virtual size_t getNumChainElements(size_t chainIndex) const;

		/** Remove all elements of a given chain (but leave the chain intact). */
		virtual void clearChain(size_t chainIndex);
		/** Remove all elements from all chains (but leave the chains themselves intact). */
		virtual void clearAllChains(void);

		/// Get the material name in use
		virtual const String& getMaterialName(void) const { return mMaterialName; }
		/// Set the material name to use for rendering
		virtual void setMaterialName( const String& name, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );


		// Overridden members follow
		void _notifyCurrentCamera(Camera* cam);
		Real getSquaredViewDepth(const Camera* cam) const;
		Real getBoundingRadius(void) const;
		const AxisAlignedBox& getBoundingBox(void) const;
		const MaterialPtr& getMaterial(void) const;
		const String& getMovableType(void) const;
		void _updateRenderQueue(RenderQueue *);
		void getRenderOperation(RenderOperation &);
		void getWorldTransforms(Matrix4 *) const;
		const LightList& getLights(void) const;
		/// @copydoc MovableObject::visitRenderables
		void visitRenderables(Renderable::Visitor* visitor, 
			bool debugRenderables = false);



	protected:

		/// Maximum length of each chain
		size_t mMaxElementsPerChain;
		/// Number of chains
		size_t mChainCount;
		/// Use texture coords?
		bool mUseTexCoords;
		/// Use vertex colour?
		bool mUseVertexColour;
		/// Dynamic use?
		bool mDynamic;
		/// Vertex data
		VertexData* mVertexData;
		/// Index data (to allow multiple unconnected chains)
		IndexData* mIndexData;
		/// Is the vertex declaration dirty?
		bool mVertexDeclDirty;
		/// Do the buffers need recreating?
		bool mBuffersNeedRecreating;
		/// Do the bounds need redefining?
		mutable bool mBoundsDirty;
		/// Is the index buffer dirty?
		bool mIndexContentDirty;
		/// AABB
		mutable AxisAlignedBox mAABB;
		/// Bounding radius
		mutable Real mRadius;
		/// Material 
		String mMaterialName;
		MaterialPtr mMaterial;
		/// Texture coord direction
		TexCoordDirection mTexCoordDir;
		/// Other texture coord range
		Real mOtherTexCoordRange[2];


		/// The list holding the chain elements
		ElementList mChainElementList;

		/** Simple struct defining a chain segment by referencing a subset of
			the preallocated buffer (which will be mMaxElementsPerChain * mChainCount
			long), by it's chain index, and a head and tail value which describe
			the current chain. The buffer subset wraps at mMaxElementsPerChain
			so that head and tail can move freely. head and tail are inclusive,
			when the chain is empty head and tail are filled with high-values.
		*/
		struct ChainSegment
		{
			/// The start of this chains subset of the buffer
			size_t start;
			/// The 'head' of the chain, relative to start
			size_t head;
			/// The 'tail' of the chain, relative to start
			size_t tail;
		};
		typedef vector<ChainSegment>::type ChainSegmentList;
		ChainSegmentList mChainSegmentList;

		/// Setup the STL collections
		virtual void setupChainContainers(void);
		/// Setup vertex declaration
		virtual void setupVertexDeclaration(void);
		// Setup buffers
		virtual void setupBuffers(void);
		/// Update the contents of the vertex buffer
		virtual void updateVertexBuffer(Camera* cam);
		/// Update the contents of the index buffer
		virtual void updateIndexBuffer(void);
		virtual void updateBoundingBox(void) const;

		/// Chain segment has no elements
		static const size_t SEGMENT_EMPTY;
	};


	/** Factory object for creating BillboardChain instances */
	class _OgreExport BillboardChainFactory : public MovableObjectFactory
	{
	protected:
		MovableObject* createInstanceImpl( const String& name, const NameValuePairList* params);
	public:
		BillboardChainFactory() {}
		~BillboardChainFactory() {}

		static String FACTORY_TYPE_NAME;

		const String& getType(void) const;
		void destroyInstance( MovableObject* obj);  

	};

	/** @} */
	/** @} */

} // namespace

#endif


