
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
        //NOTE: For performance reasons, ForwardClustered::collectLightForSlice ignores
        //mLocalAabb & mWorldAabb and assumes its local AABB is this aabb we set as
        //default. To change its shape, use node scaling
        Aabb aabb( Vector3::ZERO, Vector3::UNIT_SCALE * 0.5f );
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
    void Decal::setDiffuseTexture( const TexturePtr &diffuseTex, uint16 diffuseIdx )
    {
        mDiffuseTexture = diffuseTex;
        mDiffuseIdx = diffuseIdx;
    }
    //-----------------------------------------------------------------------------------
    const TexturePtr& Decal::getDiffuseTexture(void) const
    {
        return mDiffuseTexture;
    }
    //-----------------------------------------------------------------------------------
    void Decal::setNormalTexture( const TexturePtr &normalTex, uint16 normalIdx )
    {
        mNormalTexture = normalTex;
        mNormalMapIdx = normalIdx;
    }
    //-----------------------------------------------------------------------------------
    const TexturePtr& Decal::getNormalTexture(void) const
    {
        return mNormalTexture;
    }
    //-----------------------------------------------------------------------------------
    void Decal::setEmissiveTexture( const TexturePtr &emissiveTex, uint16 emissiveIdx )
    {
        mEmissiveTexture = emissiveTex;
        mEmissiveIdx = emissiveIdx;
    }
    //-----------------------------------------------------------------------------------
    const TexturePtr& Decal::getEmissiveTexture(void) const
    {
        return mEmissiveTexture;
    }
    //-----------------------------------------------------------------------------------
    void Decal::setMetalness( float value )
    {
        mMetalness = value;
    }
    //-----------------------------------------------------------------------------------
    float Decal::getMetalness(void) const
    {
        return mMetalness;
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
