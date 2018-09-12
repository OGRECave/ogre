
#ifndef _OgreDecal_H_
#define _OgreDecal_H_

#include "OgreMovableObject.h"
#include "OgreRenderable.h"

namespace Ogre
{
    /**
    @class Decal
        Decals can have diffuse, normal map, and emissive on top of regular objects.
        You can GLOBALLY disable each of these (diffuse/normal/emissive) by setting
        a null texture in SceneManager::setDecalsDiffuse and co.

        If a nullptr is set on e.g. Normal textures, and a Decal uses normal maps,
        they won't appear. You need to set a valid pointer to SceneManager::setDecalsNormals.

        Likewise, if no Decal actually uses normal textures, but you still set one
        via SceneManager::setDecalsNormals, then you will be wasting GPU shader performance.

        When diffuse is used, the fresnel and specular colours of the material are
        overwritten as well.

        Q: How to disable diffuse/normal/emissive per Decal?

        To disable diffuse just point to a diffuse texture slice (mDiffuseIdx) that
        is a black and call setIgnoreAlphaDiffuse( true )<br/>
        To disable normals, just point to a normal map that has XY components set
        both to 0<br/>
        To disable emissive, just point to a texture that is black.

        Q: Why does it use Metalness? What happens when a non-metallic workflow is used?

        Metalness is used for performance reasons, as it allows giving enough control
        over the specular by just using one parameter, rather than having 3 (one per
        specular channel)

        On metallic & fresnel workflows, this "just works" because the metalness
        parameter will produce a new coloured F0 parameter, which will overwrite
        the one from the original material.

        On specular workflows, if the fresnel is coloured, it will behave the same
        way as a metallic/fresnel workflow, because the original specular colour will
        be set to white, and F0 will be overwritten.

        If the fresnel is monochrome (specular workflow), then the specular colour
        will be overwritten instead, and the metalness value of the decal will
        act as a fresnel value, overwriting the original fresnel.
    */
    class _OgreExport Decal : public MovableObject
    {
    protected:
        TexturePtr  mDiffuseTexture;
        TexturePtr  mNormalTexture;
        TexturePtr  mEmissiveTexture;

    public:
        uint16 mDiffuseIdx;
        uint16 mNormalMapIdx;
        uint16 mEmissiveIdx;
        uint16 mPadding;
        float mMetalness;
        float mRoughness;

    public:
        Decal( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager* manager );
        virtual ~Decal();

        void setDiffuseTexture( const TexturePtr &diffuseTex, uint16 diffuseIdx );
        void setNormalTexture( const TexturePtr &normalTex, uint16 normalIdx );
        void setEmissiveTexture( const TexturePtr &emissiveTex, uint16 emissiveIdx );

        const TexturePtr& getDiffuseTexture(void) const;
        const TexturePtr& getNormalTexture(void) const;
        const TexturePtr& getEmissiveTexture(void) const;

        /// Value for Metalness. Must be in range [0; 1]
        void setMetalness( float value );
        float getMetalness(void) const              { return mMetalness; }

        /// Value for Roughness. Valid range depends on the BRDF used.
        void setRoughness( float roughness );
        float getRoughness(void) const              { return mRoughness; }

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
