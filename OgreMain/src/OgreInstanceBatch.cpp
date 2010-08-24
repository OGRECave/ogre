/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreStableHeaders.h"
#include "OgreInstanceBatch.h"
#include "OgreSubMesh.h"
#include "OgreRenderOperation.h"
#include "OgreInstancedEntity.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"
#include "OgreException.h"

namespace Ogre
{
	InstanceBatch::InstanceBatch( MeshPtr &meshReference, const MaterialPtr &material,
									size_t instancesPerBatch, const Mesh::IndexMap *indexToBoneMap,
									const String &batchName ) :
				m_instancesPerBatch( instancesPerBatch ),
				m_material( material ),
				m_meshReference( meshReference ),
				m_indexToBoneMap( indexToBoneMap ),
				m_boundingRadius( 0 ),
				m_boundsDirty( true ),
				m_currentCamera( 0 ),
				mCachedCamera( 0 )
	{
		assert( m_instancesPerBatch );
		assert( !(meshReference->hasSkeleton() && !indexToBoneMap) );

		m_fullBoundingBox.setExtents( -Vector3::ZERO, Vector3::ZERO );

		mName = batchName;
	}

	InstanceBatch::~InstanceBatch()
	{
		deleteAllInstancedEntities();
	}

	void InstanceBatch::_setInstancesPerBatch( size_t instancesPerBatch )
	{
		if( !m_instancedEntities.empty() )
		{
			OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "Instances per batch can only be changed before"
						" building the batch.", "InstanceBatch::_setInstancesPerBatch");
		}

		m_instancesPerBatch = instancesPerBatch;
	}
	//-----------------------------------------------------------------------
	bool InstanceBatch::checkSubMeshCompatibility( const SubMesh* baseSubMesh )
	{
		if( baseSubMesh->operationType != RenderOperation::OT_TRIANGLE_LIST )
		{
			OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, "Only meshes with OT_TRIANGLE_LIST are supported",
						"InstanceBatch::checkSubMeshCompatibility");
		}

		return true;
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::updateBounds()
	{
		const AxisAlignedBox oldAABB = m_fullBoundingBox;
		m_fullBoundingBox.setNull();

		m_fullBoundingBox.merge( Vector3( 0, 1000, 0 ) );

		InstancedEntityVec::const_iterator itor = m_instancedEntities.begin();
		InstancedEntityVec::const_iterator end  = m_instancedEntities.end();

		while( itor != end )
		{
			//Only increase the bounding box for those objects we know are in the scene
			if( (*itor)->isInScene() )
				m_fullBoundingBox.merge( (*itor)->getWorldBoundingBox(true) );

			++itor;
		}

		m_boundingRadius = Math::boundingRadiusFromAABB( m_fullBoundingBox );

		//Can't just call mParentNode->_updateBounds() since the SceneManager may be iterating
		//over us and such call would invalidate the iterator. We'll do this in the next frame
		//Worst case scenario, all instances flickered (disappeared) for a single frame.
		if( oldAABB != m_fullBoundingBox )
			mParentNode->needUpdate( true );

		m_boundsDirty = false;
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::updateVisibility()
	{
		mVisible = false;

		InstancedEntityVec::const_iterator itor = m_instancedEntities.begin();
		InstancedEntityVec::const_iterator end  = m_instancedEntities.end();

		while( itor != end )
		{
			//Trick to force Ogre not to render us if none of our instances is visible
			//Because we do Camera::isVisible(), it is better if the SceneNode from the
			//InstancedEntity is not part of the scene graph (i.e. ultimate parent is root node)
			//to avoid unnecesary wasteful calculations
			mVisible |= (*itor)->findVisible( m_currentCamera );
			++itor;
		}
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::createAllInstancedEntities()
	{
		m_instancedEntities.resize( m_instancesPerBatch );
		m_unusedEntities.resize( m_instancesPerBatch );

		for( size_t i=0; i<m_instancesPerBatch; ++i )
		{
			InstancedEntity *instance = OGRE_NEW InstancedEntity( this, i );
			m_instancedEntities[i]	= instance;
			m_unusedEntities[i]		= instance;
		}
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::deleteAllInstancedEntities()
	{
		InstancedEntityVec::const_iterator itor = m_instancedEntities.begin();
		InstancedEntityVec::const_iterator end  = m_instancedEntities.end();

		while( itor != end )
		{
			if( (*itor)->getParentSceneNode() )
				(*itor)->getParentSceneNode()->detachObject( (*itor) );

			OGRE_DELETE *itor++;
		}
	}
	//-----------------------------------------------------------------------
	RenderOperation InstanceBatch::build( const SubMesh* baseSubMesh )
	{
		if( checkSubMeshCompatibility( baseSubMesh ) )
		{
			//Only triangle list at the moment
			m_renderOperation.operationType	= RenderOperation::OT_TRIANGLE_LIST;
			m_renderOperation.srcRenderable	= this;
			m_renderOperation.useIndexes	= true;
			setupVertices( baseSubMesh );
			setupIndices( baseSubMesh );

			createAllInstancedEntities();
		}

		return m_renderOperation;
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::buildFrom( const SubMesh *baseSubMesh, const RenderOperation &renderOperation )
	{
		m_renderOperation = renderOperation;
		createAllInstancedEntities();
	}
	//-----------------------------------------------------------------------
	InstancedEntity* InstanceBatch::createInstancedEntity()
	{
		InstancedEntity *retVal = 0;

		if( !m_unusedEntities.empty() )
		{
			retVal = m_unusedEntities.back();
			m_unusedEntities.pop_back();
		}

		return retVal;
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::removeInstancedEntity( InstancedEntity *instancedEntity )
	{
		if( instancedEntity->m_batchOwner != this )
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Trying to remove an InstancedEntity from scene created"
						" with a different InstanceBatch",
						"InstanceBatch::removeInstancedEntity()");
		}

		if( instancedEntity->getParentSceneNode() )
			instancedEntity->getParentSceneNode()->detachObject( instancedEntity );

		//Put it back into the queue
		m_unusedEntities.push_back( instancedEntity );
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::_boundsDirty(void)
	{
		mParentNode->needUpdate();
		m_boundsDirty = true;
	}
	//-----------------------------------------------------------------------
	const String& InstanceBatch::getMovableType(void) const
	{
		static String sType = "InstanceBatch";
		return sType;
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::_notifyCurrentCamera( Camera* cam )
	{
		m_currentCamera = cam;

		//Clear the camera cache
		mCachedCamera = 0;

		MovableObject::_notifyCurrentCamera( cam );
	}
	//-----------------------------------------------------------------------
	const AxisAlignedBox& InstanceBatch::getBoundingBox(void) const
	{
		return m_fullBoundingBox;
	}
	//-----------------------------------------------------------------------
	Real InstanceBatch::getBoundingRadius(void) const
	{
		return m_boundingRadius;
	}
	//-----------------------------------------------------------------------
	Real InstanceBatch::getSquaredViewDepth( const Camera* cam ) const
	{
		if( mCachedCamera != cam )
		{
			mCachedCameraDist = std::numeric_limits<Real>::infinity();

			InstancedEntityVec::const_iterator itor = m_instancedEntities.begin();
			InstancedEntityVec::const_iterator end  = m_instancedEntities.end();

			while( itor != end )
			{
				mCachedCameraDist = std::min( mCachedCameraDist, (*itor)->getSquaredViewDepth( cam ) );
				++itor;
			}
		}

        return mCachedCameraDist;
	}
	//-----------------------------------------------------------------------
	const LightList& InstanceBatch::getLights( void ) const
	{
		return queryLights();
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::_updateRenderQueue( RenderQueue* queue )
	{
		if( m_boundsDirty )
			updateBounds();

		//Is at least one object in the scene?
		updateVisibility();

		if( mVisible )
		{
			if( m_meshReference->hasSkeleton() )
			{
				InstancedEntityVec::const_iterator itor = m_instancedEntities.begin();
				InstancedEntityVec::const_iterator end  = m_instancedEntities.end();

				while( itor != end )	
				{
					(*itor)->_updateAnimation();
					++itor;
				}
			}

			queue->addRenderable( this );
		}

		//Reset visibility once we skipped addRenderable (which saves GPU time), because OGRE for some
		//reason stops updating our render queue afterwards, preventing us to recalculate visibility
		mVisible = true;
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::visitRenderables( Renderable::Visitor* visitor, bool debugRenderables )
	{
		visitor->visit( this, 0, false );
	}
}
