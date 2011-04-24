/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

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
#include "OgreInstanceManager.h"
#include "OgreInstanceBatch.h"
#include "OgreSubMesh.h"
#include "OgreRenderOperation.h"
#include "OgreInstancedEntity.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"
#include "OgreLodStrategy.h"
#include "OgreException.h"

namespace Ogre
{
	InstanceBatch::InstanceBatch( InstanceManager *creator, MeshPtr &meshReference,
									const MaterialPtr &material, size_t instancesPerBatch,
									const Mesh::IndexMap *indexToBoneMap, const String &batchName ) :
				Renderable(),
                MovableObject(),
				m_instancesPerBatch( instancesPerBatch ),
				m_creator( creator ),
				m_material( material ),
				m_meshReference( meshReference ),
				m_indexToBoneMap( indexToBoneMap ),
				m_boundingRadius( 0 ),
				m_boundsDirty( false ),
				m_boundsUpdated( false ),
				m_currentCamera( 0 ),
				m_materialLodIndex( 0 ),
				m_technSupportsSkeletal( true ),
				mCachedCamera( 0 )
	{
		assert( m_instancesPerBatch );

		//Force batch visibility to be always visible. The instanced entities
		//have individual visibility flags. If none matches the scene's current,
		//then this batch won't rendered.
		mVisibilityFlags = std::numeric_limits<Ogre::uint32>::max();

		if( indexToBoneMap )
			assert( !(meshReference->hasSkeleton() && indexToBoneMap->empty()) );

		m_fullBoundingBox.setExtents( -Vector3::ZERO, Vector3::ZERO );

		mName = batchName;
	}

	InstanceBatch::~InstanceBatch()
	{
		deleteAllInstancedEntities();

		//Remove the parent scene node automatically
		SceneNode *sceneNode = getParentSceneNode();
		if( sceneNode )
		{
			sceneNode->detachAllObjects();
			sceneNode->getParentSceneNode()->removeAndDestroyChild( sceneNode->getName() );
		}
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
	void InstanceBatch::_updateBounds(void)
	{
		m_fullBoundingBox.setNull();

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

		//Tell the SceneManager our bounds have changed
		getParentSceneNode()->_updateBounds();

		m_boundsDirty	= false;
		m_boundsUpdated	= true;
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::updateVisibility(void)
	{
		mVisible = false;

		InstancedEntityVec::const_iterator itor = m_instancedEntities.begin();
		InstancedEntityVec::const_iterator end  = m_instancedEntities.end();

		while( itor != end )
		{
			//Trick to force Ogre not to render us if none of our instances is visible
			//Because we do Camera::isVisible(), it is better if the SceneNode from the
			//InstancedEntity is not part of the scene graph (i.e. ultimate parent is root node)
			//to avoid unnecessary wasteful calculations
			mVisible |= (*itor)->findVisible( m_currentCamera );
			++itor;
		}
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::createAllInstancedEntities()
	{
		m_instancedEntities.reserve( m_instancesPerBatch );
		m_unusedEntities.reserve( m_instancesPerBatch );

		for( size_t i=0; i<m_instancesPerBatch; ++i )
		{
			InstancedEntity *instance = OGRE_NEW InstancedEntity( this, i );
			m_instancedEntities.push_back( instance );
			m_unusedEntities.push_back( instance );
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
	void InstanceBatch::deleteUnusedInstancedEntities()
	{
		InstancedEntityVec::const_iterator itor = m_unusedEntities.begin();
		InstancedEntityVec::const_iterator end  = m_unusedEntities.end();

		while( itor != end )
			OGRE_DELETE *itor++;

		m_unusedEntities.clear();
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::makeMatrixCameraRelative3x4( float *mat3x4, size_t numFloats )
	{
		const Vector3 &cameraRelativePosition = m_currentCamera->getDerivedPosition();

		for( size_t i=0; i<numFloats >> 2; i += 3 )
		{
			const Vector3 worldTrans( mat3x4[(i+0) * 4 + 3], mat3x4[(i+1) * 4 + 3],
										mat3x4[(i+2) * 4 + 3] );
			const Vector3 newPos( worldTrans - cameraRelativePosition );

			mat3x4[(i+0) * 4 + 3] = newPos.x;
			mat3x4[(i+1) * 4 + 3] = newPos.y;
			mat3x4[(i+2) * 4 + 3] = newPos.z;
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

			retVal->m_inUse = true;
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

		instancedEntity->m_inUse = false;
		instancedEntity->stopSharingTransform();

		//Put it back into the queue
		m_unusedEntities.push_back( instancedEntity );
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::getInstancedEntitiesInUse( InstancedEntityVec &outEntities )
	{
		InstancedEntityVec::const_iterator itor = m_instancedEntities.begin();
		InstancedEntityVec::const_iterator end  = m_instancedEntities.end();

		while( itor != end )
		{
			if( (*itor)->m_inUse )
				outEntities.push_back( *itor );
			++itor;
		}
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::defragmentBatchNoCull( InstancedEntityVec &usedEntities )
	{
		const size_t maxInstancesToCopy = std::min( m_instancesPerBatch, usedEntities.size() );
		InstancedEntityVec::iterator first = usedEntities.end() - maxInstancesToCopy;

		//Copy from the back to front, into m_instancedEntities
		m_instancedEntities.insert( m_instancedEntities.begin(), first, usedEntities.end() );
		//Remove them from the array
		usedEntities.resize( usedEntities.size() - maxInstancesToCopy );	
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::defragmentBatchDoCull( InstancedEntityVec &usedEntities )
	{
		//Get the the entity closest to the minimum bbox edge and put into "first"
		InstancedEntityVec::const_iterator itor	= usedEntities.begin();
		InstancedEntityVec::const_iterator end	= usedEntities.end();

		Vector3 vMinPos = Vector3::ZERO, firstPos = Vector3::ZERO;
		InstancedEntity *first = 0;

		if( !usedEntities.empty() )
		{
			first		= *usedEntities.begin();
			firstPos	= first->getParentNode()->_getDerivedPosition();
			vMinPos		= first->getParentNode()->_getDerivedPosition();
		}

		while( itor != end )
		{
			const Vector3 &vPos		= (*itor)->getParentNode()->_getDerivedPosition();

			vMinPos.x = std::min( vMinPos.x, vPos.x );
			vMinPos.y = std::min( vMinPos.y, vPos.y );
			vMinPos.z = std::min( vMinPos.z, vPos.z );

			if( vMinPos.squaredDistance( vPos ) < vMinPos.squaredDistance( firstPos ) )
			{
				first		= *itor;
				firstPos	= vPos;
			}

			++itor;
		}

		//Now collect entities closest to 'first'
		while( !usedEntities.empty() && m_instancedEntities.size() < m_instancesPerBatch )
		{
			InstancedEntityVec::iterator closest	= usedEntities.begin();
			InstancedEntityVec::iterator it         = usedEntities.begin();
			InstancedEntityVec::iterator e          = usedEntities.end();

			Vector3 closestPos;
			closestPos = (*closest)->getParentNode()->_getDerivedPosition();

			while( it != e )
			{
				const Vector3 &vPos	= (*it)->getParentNode()->_getDerivedPosition();

				if( firstPos.squaredDistance( vPos ) < firstPos.squaredDistance( closestPos ) )
				{
					closest		= it;
					closestPos	= vPos;
				}

				++it;
			}

			m_instancedEntities.push_back( *closest );

			//Remove 'closest' from usedEntities using swap and pop_back trick
			*closest = *(usedEntities.end() - 1);
			usedEntities.pop_back();
		}
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::_defragmentBatch( bool optimizeCulling, InstancedEntityVec &usedEntities )
	{
		//Remove and clear what we don't need
		m_instancedEntities.clear();
		deleteUnusedInstancedEntities();

		if( !optimizeCulling )
			defragmentBatchNoCull( usedEntities );
		else
			defragmentBatchDoCull( usedEntities );

		//Reassign instance IDs and tell we're the new parent
		uint32 instanceId = 0;
		InstancedEntityVec::const_iterator itor = m_instancedEntities.begin();
		InstancedEntityVec::const_iterator end  = m_instancedEntities.end();

		while( itor != end )
		{
			(*itor)->m_instanceID = instanceId++;
			(*itor)->m_batchOwner = this;
			++itor;
		}

		//Recreate unused entities, if there's left space in our container
		assert( (signed)(m_instancesPerBatch) - (signed)(m_instancedEntities.size()) >= 0 );
		m_instancedEntities.reserve( m_instancesPerBatch );
		m_unusedEntities.reserve( m_instancesPerBatch );
		for( size_t i=m_instancedEntities.size(); i<m_instancesPerBatch; ++i )
		{
			InstancedEntity *instance = OGRE_NEW InstancedEntity( this, i );
			m_instancedEntities.push_back( instance );
			m_unusedEntities.push_back( instance );
		}

		//We've potentially changed our bounds
		if( !isBatchUnused() )
			_boundsDirty();
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::_defragmentBatchDiscard(void)
	{
		//Remove and clear what we don't need
		m_instancedEntities.clear();
		deleteUnusedInstancedEntities();
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::_boundsDirty(void)
	{
		if( !m_boundsDirty )
			m_creator->_addDirtyBatch( this );
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

		//See DistanceLodStrategy::getValueImpl()
		//We use our own because our SceneNode is just filled with zeroes, and updating it
		//with real values is expensive, plus we would need to make sure it doesn't get to
		//the shader
		Real squaredDepth = getSquaredViewDepth(cam) -
							Math::Sqr( m_meshReference->getBoundingSphereRadius() );
        squaredDepth = std::max( squaredDepth, Real(0) );
        Real lodValue = squaredDepth * cam->_getLodBiasInverse();

		//Now calculate Material LOD
        /*const LodStrategy *materialStrategy = m_material->getLodStrategy();
        
        //Calculate lod value for given strategy
        Real lodValue = materialStrategy->getValue( this, cam );*/

        //Get the index at this depth
        unsigned short idx = m_material->getLodIndex( lodValue );

		//TODO: Replace subEntity for MovableObject
        // Construct event object
        /*EntityMaterialLodChangedEvent subEntEvt;
        subEntEvt.subEntity = this;
        subEntEvt.camera = cam;
        subEntEvt.lodValue = lodValue;
        subEntEvt.previousLodIndex = m_materialLodIndex;
        subEntEvt.newLodIndex = idx;

        //Notify lod event listeners
        cam->getSceneManager()->_notifyEntityMaterialLodChanged(subEntEvt);*/

        //Change lod index
        m_materialLodIndex = idx;

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

			mCachedCamera = cam;
		}

        return mCachedCameraDist;
	}
	//-----------------------------------------------------------------------
	const LightList& InstanceBatch::getLights( void ) const
	{
		return queryLights();
	}
	//-----------------------------------------------------------------------
	Technique* InstanceBatch::getTechnique( void ) const
	{
		return m_material->getBestTechnique( m_materialLodIndex, this );
	}
	//-----------------------------------------------------------------------
	void InstanceBatch::_updateRenderQueue( RenderQueue* queue )
	{
		/*if( m_boundsDirty )
			_updateBounds();*/

		m_dirtyAnimation = false;

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
					m_dirtyAnimation |= (*itor)->_updateAnimation();
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
