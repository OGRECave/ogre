
#ifndef _OgreDecal_H_
#define _OgreDecal_H_

#include "OgreMovableObject.h"
#include "OgreRenderable.h"

namespace Ogre
{
    class Decal : public MovableObject
    {
    protected:
        TexturePtr  mDiffuseTexture;
        TexturePtr  mNormalTexture;
        TexturePtr  mEmissiveTexture;

    public:
        uint32 mDiffuseIdx;
        uint32 mNormalMapIdx;
        uint32 mEmissiveIdx;

    public:
        Decal( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager* manager );
        virtual ~Decal();

        void setDiffuseTexture( const TexturePtr &diffuseTex, uint32 diffuseIdx );
        void setNormalTexture( const TexturePtr &normalTex, uint32 normalIdx );
        void setEmissiveTexture( const TexturePtr &emissiveTex, uint32 emissiveIdx );

        const TexturePtr& getDiffuseTexture(void) const;
        const TexturePtr& getNormalTexture(void) const;
        const TexturePtr& getEmissiveTexture(void) const;

        //Overrides from MovableObject
        virtual const String& getMovableType(void) const;

        /// Decals only allow ForwardPlusBase::MinDecalRq <= queueID < ForwardPlusBase::MaxDecalRq
        virtual void setRenderQueueGroup(uint8 queueID);
    };

    class _OgreExport DecalFactory : public MovableObjectFactory
    {
    protected:
        virtual MovableObject* createInstanceImpl( IdType id, ObjectMemoryManager *objectMemoryManager,
                                                   SceneManager *manager,
                                                   const NameValuePairList* params = 0 );
    public:
        DecalFactory() {}
        virtual ~DecalFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const;
        void destroyInstance(MovableObject* obj);
    };
}

#endif
