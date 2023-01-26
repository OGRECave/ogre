/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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

#ifndef __Ogre_TerrainMaterialGenerator_H__
#define __Ogre_TerrainMaterialGenerator_H__

#include "OgreTerrainPrerequisites.h"
#include "OgrePixelFormat.h"
#include "OgreSharedPtr.h"

namespace Ogre
{
    class Rectangle2D;
    class Terrain;

    /** \addtogroup Optional
    *  @{
    */
    /** \addtogroup Terrain
    *  Some details on the terrain component
    *  @{
    */

    /** Description of a sampler that will be used with each layer. 
    */
    struct _OgreTerrainExport TerrainLayerSampler
    {
        /// A descriptive name that is merely used to assist in recognition
        String alias;
        /// The format required of this texture
        PixelFormat format;

        TerrainLayerSampler()
            : alias(""), format(PF_UNKNOWN)
        {
        }

        TerrainLayerSampler(const String& aliasName, PixelFormat fmt)
            : alias(aliasName), format(fmt)
        {
        }
    };
    /** The definition of the information each layer will contain in this terrain.
    All layers must contain the same structure of information, although the
    input textures can be different per layer instance. 
    */
    typedef std::vector<TerrainLayerSampler> TerrainLayerDeclaration;

    /** Class that provides functionality to generate materials for use with a terrain.

        Terrains are composed of one or more layers of texture information, and
        require that a material is generated to render them. There are various approaches
        to rendering the terrain, which may vary due to:
        <ul><li>Hardware support (static)</li>
        <li>Texture instances assigned to a particular terrain (dynamic in an editor)</li>
        <li>User selection (e.g. changing to a cheaper option in order to increase performance, 
        or in order to test how the material might look on other hardware (dynamic)</li>
        </ul>
        Subclasses of this class are responsible for responding to these factors and
        to generate a terrain material.
    */
    class _OgreTerrainExport TerrainMaterialGenerator : public TerrainAlloc
    {
    public:
        struct Profile
        {
            virtual ~Profile() {}
        };

        TerrainMaterialGenerator();
        virtual ~TerrainMaterialGenerator();
    
        /// Get the active profile
        virtual Profile* getActiveProfile() const { return NULL; }

        /// Internal method - indicates that a change has been made that would require material regeneration
        void _markChanged() { ++mChangeCounter; }

        /** Returns the number of times the generator has undergone a change which 
            would require materials to be regenerated.
        */
        unsigned long long int getChangeCount() const { return mChangeCounter; }

        /** Get the layer declaration that this material generator operates with.
        */
        const TerrainLayerDeclaration& getLayerDeclaration() const { return mLayerDecl; }

        /** Return whether this material generator supports using a compressed
            vertex format. This is only possible when using shaders.
        */
        virtual bool isVertexCompressionSupported() const { return false; }

        /** Triggers the generator to request the options that it needs.
        */
        virtual void requestOptions(Terrain* terrain) {}
        /** Generate a material for the given terrain using the active profile.
        */
        virtual MaterialPtr generate(const Terrain* terrain) = 0;
        /** Generate a material for the given composite map of the terrain using the active profile.
        */
        virtual MaterialPtr generateForCompositeMap(const Terrain* terrain) = 0;
        /** Whether to support a light map over the terrain in the shader,
        if it's present (default true). 
        */
        virtual void setLightmapEnabled(bool enabled) {}
        /** Get the maximum number of layers supported with the given terrain. 
        @note When you change the options on the terrain, this value can change. 
        */
        virtual uint8 getMaxLayers(const Terrain* terrain) const { return 0; }

        /** Update the composite map for a terrain.
        The composite map for a terrain must match what the terrain should look like
        at distance. This method will only be called in the render thread so the
        generator is free to render into a texture to support this, so long as 
        the results are blitted into the Terrain's own composite map afterwards.
        */
        void updateCompositeMap(const Terrain* terrain, const Rect& rect);

        /** Update parameters for the given terrain using the active profile.
        */
        virtual void updateParams(const MaterialPtr& mat, const Terrain* terrain) {}
        /** Update parameters for the given terrain composite map using the active profile.
        */
        virtual void updateParamsForCompositeMap(const MaterialPtr& mat, const Terrain* terrain) {}

        /** Set the debug level of the material. 

            Sets the level of debug display for this material.
            What this debug level means is entirely depdendent on the generator, 
            the only constant is that 0 means 'no debug' and non-zero means 
            'some level of debugging', with any graduations in non-zero values
            being generator-specific.
        */
        void setDebugLevel(unsigned int dbg)
        {
            if (mDebugLevel != dbg)
            {
                mDebugLevel = dbg;
                _markChanged();
            }
        }
        /// Get the debug level of the material. 
        unsigned int getDebugLevel() const { return mDebugLevel; }

        /** Helper method to render a composite map.
        @param size The requested composite map size
        @param rect The region of the composite map to update, in image space
        @param mat The material to use to render the map
        @param destCompositeMap A TexturePtr for the composite map to be written into
        */
        void _renderCompositeMap(size_t size, const Rect& rect, const MaterialPtr& mat,
                                 const TexturePtr& destCompositeMap);

        Texture* _getCompositeMapRTT() { return mCompositeMapRTT; }
    protected:
        unsigned long long int mChangeCounter;
        TerrainLayerDeclaration mLayerDecl;
        unsigned int mDebugLevel;
        SceneManager* mCompositeMapSM;
        Camera* mCompositeMapCam;
        Texture* mCompositeMapRTT; // deliberately holding this by raw pointer to avoid shutdown issues
        Rectangle2D* mCompositeMapPlane;
        Light* mCompositeMapLight;
        SceneNode* mLightNode;


    };

    typedef SharedPtr<TerrainMaterialGenerator> TerrainMaterialGeneratorPtr;

    /** @} */
    /** @} */

}
#endif

