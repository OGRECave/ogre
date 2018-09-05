
#include "OgreDecal.h"
#include "OgreForwardPlusBase.h"

#include "OgreSceneManager.h"

#include "OgreRoot.h"
#include "OgreHlms.h"
#include "OgreHlmsManager.h"

namespace Ogre
{
    Decal::Decal( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *manager ) :
        MovableObject( id, objectMemoryManager, manager, 0 ),
        mDiffuseIdx( 0 ),
        mNormalMapIdx( 0 ),
        mEmissiveIdx( 0 )
    {
        Aabb aabb( Vector3::ZERO, Vector3::UNIT_SCALE );
        mObjectData.mLocalAabb->setFromAabb( aabb, mObjectData.mIndex );
        mObjectData.mWorldAabb->setFromAabb( aabb, mObjectData.mIndex );
        const float radius = aabb.getRadius();
        mObjectData.mLocalRadius[mObjectData.mIndex] = radius;
        mObjectData.mWorldRadius[mObjectData.mIndex] = radius;
    }
    //-----------------------------------------------------------------------------------
    Decal::~Decal()
    {
    }
    //-----------------------------------------------------------------------------------
    const String& Decal::getMovableType(void) const
    {
        return DecalFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------------------
    void Decal::setRenderQueueGroup(uint8 queueID)
    {
        assert( queueID >= ForwardPlusBase::MinDecalRq && queueID <= ForwardPlusBase::MaxDecalRq &&
                "RenderQueue IDs > 128 are reserved for other Forward+ objects" );
        MovableObject::setRenderQueueGroup( queueID );
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String DecalFactory::FACTORY_TYPE_NAME = "Decal";
    //-----------------------------------------------------------------------
    const String& DecalFactory::getType(void) const
    {
     return FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    MovableObject* DecalFactory::createInstanceImpl( IdType id,
                                                  ObjectMemoryManager *objectMemoryManager,
                                                  SceneManager *manager,
                                                  const NameValuePairList* params )
    {

     Decal* decal = OGRE_NEW Decal( id, objectMemoryManager, manager );
     return decal;
    }
    //-----------------------------------------------------------------------
    void DecalFactory::destroyInstance( MovableObject* obj)
    {
     OGRE_DELETE obj;
    }
}
