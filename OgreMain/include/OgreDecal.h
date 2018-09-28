/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2018 Torus Knot Software Ltd

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
        uint16 mIgnoreDiffuseAlpha;
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

        /** When diffuse textures are used (globally), the alpha component of the diffuse texture
            will be used to mask all the other textures (e.g. normal & emissive maps).

            When bIgnore = true, the alpha component of the diffuse texture won't be used on
            normal & emissive maps.
        @param bIgnore
            True to ignore the alpha on the other maps. False otherwise. Default: False
        */
        void setIgnoreAlphaDiffuse( bool bIgnore );
        bool getIgnoreAlphaDiffuse(void) const;

        /// Value for Metalness. Must be in range [0; 1]
        void setMetalness( float value );
        float getMetalness(void) const              { return mMetalness; }

        /// Value for Roughness. Valid range depends on the BRDF used.
        void setRoughness( float roughness );
        float getRoughness(void) const              { return mRoughness; }

        /** Helper function to set width, height and depth of the decal.
            This is a helper function because these parameters actually live in the parent scene node

            All the function does is literally:
            @code
                if( mParentNode )
                    mParentNode->setScale( planeDimensions.x, depth, planeDimensions.y );
            @endcode
        @param planeDimensions
            planeDimensions.x = width
            planeDimensions.y = height
        @param depth
            Decals are 2D by nature. The depth of the decal indicates its influence on the objects.
            Decals are like oriented boxes, and everything inside the box will be affected by
            the decal.
        */
        void setRectSize( Vector2 planeDimensions, Real depth );

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
