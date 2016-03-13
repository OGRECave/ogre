
#ifndef _OgreWireAabb_H_
#define _OgreWireAabb_H_

#include "OgreMovableObject.h"
#include "OgreRenderable.h"

#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** Helper class to display the Aabb of a MovableObject as lines.
        Just call track( someMovableObject ); to start rendering the AABB of 'someMovableObject'
    @remarks
        This class creates, attaches itself and manages its own SceneNode.
        The SceneNode is created and destroyed inside track()
    @par
        To get coloured lines, assign an Unlit material and change the colour.
    */
    class _OgreExport WireAabb : public MovableObject, public Renderable
    {
        MovableObject const *mTrackedObject;

        void createBuffers(void);

    public:
        WireAabb( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager* manager );
        virtual ~WireAabb();

        /** Starts tracking the given MovableObject and render its Aabb.
            If the tracked object is destroyed, we automatically stop
            tracking that object.
        @param movableObject
            MovableObject to track and render its Aabb. Null pointer to
            stop tracking.
        */
        void track( const MovableObject *movableObject );

        /** Sets it fixed to display the given aabb.
        @remarks
            Disables tracking of a movable object.
        */
        void setToAabb( const Aabb &aabb );

        const MovableObject* getTrackedObject(void) const       { return mTrackedObject; }

        /// Called by the SceneManager every frame to
        /// update our data based on tracked target
        void _updateTracking(void);

        //Overrides from MovableObject
        virtual const String& getMovableType(void) const;

        //Overrides from Renderable
        virtual const LightList& getLights(void) const;
        virtual void getRenderOperation( v1::RenderOperation& op, bool casterPass );
        virtual void getWorldTransforms( Matrix4* xform ) const;
        virtual bool getCastsShadows(void) const;
    };

    /** Factory object for creating WireAabb instances */
    class _OgreExport WireAabbFactory : public MovableObjectFactory
    {
    protected:
        virtual MovableObject* createInstanceImpl( IdType id, ObjectMemoryManager *objectMemoryManager,
                                                   SceneManager *manager,
                                                   const NameValuePairList* params = 0 );
    public:
        WireAabbFactory() {}
        virtual ~WireAabbFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const;
        void destroyInstance( MovableObject* obj);
    };
}

#include "OgreHeaderSuffix.h"

#endif
