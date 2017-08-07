
#ifndef _OgreTerra_H_
#define _OgreTerra_H_

#include "OgrePrerequisites.h"
#include "OgreMovableObject.h"
#include "OgreShaderParams.h"

#include "Terra/TerrainCell.h"

namespace Ogre
{
    struct GridPoint
    {
        int32 x;
        int32 z;
    };

    struct GridDirection
    {
        int x;
        int z;
    };

    class ShadowMapper;

    class Terra : public MovableObject
    {
        friend class TerrainCell;

        std::vector<float>          m_heightMap;
        uint32                      m_width;
        uint32                      m_depth; //PNG's Height
        float                       m_depthWidthRatio;
        float                       m_skirtSize;
        float                       m_invWidth;
        float                       m_invDepth;

        Vector2     m_xzDimensions;
        Vector2     m_xzInvDimensions;
        Vector2     m_xzRelativeSize; // m_xzDimensions / [m_width, m_height]
        float       m_height;
        Vector3     m_terrainOrigin;
        uint32      m_basePixelDimension;

        std::vector<TerrainCell>   m_terrainCells;
        std::vector<TerrainCell*>  m_collectedCells[2];
        size_t                     m_currentCell;

        Ogre::TexturePtr    m_heightMapTex;
        Ogre::TexturePtr    m_normalMapTex;

        Vector3             m_prevLightDir;
        ShadowMapper        *m_shadowMapper;

        //Ogre stuff
        CompositorManager2      *m_compositorManager;
        Camera                  *m_camera;

        void destroyHeightmapTexture(void);

        /// Creates the Ogre texture based on the image data.
        /// Called by @see createHeightmap
        void createHeightmapTexture( const Ogre::Image &image, const String &imageName );

        /// Calls createHeightmapTexture, loads image data to our CPU-side buffers
        void createHeightmap( Image &image, const String &imageName );

        void createNormalTexture(void);
        void destroyNormalTexture(void);

        ///	Automatically calculates the optimum skirt size (no gaps with
        /// lowest overdraw possible).
        ///	This is done by taking the heighest delta between two adjacent
        /// pixels in a 4x4 block.
        ///	This calculation may not be perfect, as the block search should
        /// get bigger for higher LODs.
        void calculateOptimumSkirtSize(void);

        inline GridPoint worldToGrid( const Vector3 &vPos ) const;
        inline Vector2 gridToWorld( const GridPoint &gPos ) const;

        bool isVisible( const GridPoint &gPos, const GridPoint &gSize ) const;

        void addRenderable( const GridPoint &gridPos, const GridPoint &cellSize, uint32 lodLevel );

        void optimizeCellsAndAdd(void);

    public:
        Terra( IdType id, ObjectMemoryManager *objectMemoryManager, SceneManager *sceneManager,
               uint8 renderQueueId, CompositorManager2 *compositorManager, Camera *camera );
        ~Terra();

        /** Must be called every frame so we can check the camera's position
            (passed in the constructor) and update our visible batches (and LODs)
            We also update the shadow map if the light direction changed.
        @param lightDir
            Light direction for computing the shadow map.
        @param lightEpsilon
            Epsilon to consider how different light must be from previous
            call to recompute the shadow map.
            Interesting values are in the range [0; 2], but any value is accepted.
        @par
            Large epsilons will reduce the frequency in which the light is updated,
            improving performance (e.g. only compute the shadow map when needed)
        @par
            Use an epsilon of <= 0 to force recalculation every frame. This is
            useful to prevent heterogeneity between frames (reduce stutter) if
            you intend to update the light slightly every frame.
        */
        void update( const Vector3 &lightDir, float lightEpsilon=1e-6f );

        void load( const String &texName, const Vector3 center, const Vector3 &dimensions );
        void load( Image &image, const Vector3 center, const Vector3 &dimensions, const String &imageName = BLANKSTRING );

        /** Gets the interpolated height at the given location.
            If outside the bounds, it leaves the height untouched.
        @param vPos
            [in] XZ position, Y for default height.
            [out] Y height, or default Y (from input) if outside terrain bounds.
        @return
            True if Y component was changed
        */
        bool getHeightAt( Vector3 &vPos ) const;

        /// load must already have been called.
        void setDatablock( HlmsDatablock *datablock );

        //MovableObject overloads
        const String& getMovableType(void) const;

        Camera* getCamera() const                       { return m_camera; }
        void setCamera( Camera *camera )                { m_camera = camera; }

        const ShadowMapper* getShadowMapper(void) const { return m_shadowMapper; }

        Ogre::TexturePtr getHeightMapTex(void) const    { return m_heightMapTex; }
        Ogre::TexturePtr getNormalMapTex(void) const    { return m_normalMapTex; }
        Ogre::TexturePtr _getShadowMapTex(void) const;

        const Vector2& getXZDimensions(void) const      { return m_xzDimensions; }
        const Vector2& getXZInvDimensions(void) const   { return m_xzInvDimensions; }
        float getHeight(void) const                     { return m_height; }
        const Vector3& getTerrainOrigin(void) const     { return m_terrainOrigin; }
    };
}

#endif
