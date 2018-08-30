
#ifndef _OgreDecal_H_
#define _OgreDecal_H_

#include "OgreMovableObject.h"
#include "OgreRenderable.h"

namespace Ogre
{
    class Decal : public MovableObject
    {
    public:
        uint32 mDiffuseIdx;
        uint32 mNormalMapIdx;
        uint32 mEmissiveIdx;

    public:
        Decal( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager* manager );
        virtual ~Decal();

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
