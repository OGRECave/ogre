
#ifndef _OgreTerrainCell_H_
#define _OgreTerrainCell_H_

#include "OgrePrerequisites.h"
#include "OgreRenderable.h"

namespace Ogre
{
    class Terra;
    struct GridPoint;

    class TerrainCell : public Renderable
    {
        int32  m_gridX;
        int32  m_gridZ;
        uint32 m_lodLevel;
        uint32  m_verticesPerLine;

        uint32  m_sizeX;
        uint32  m_sizeZ;

        VaoManager *m_vaoManager;

        Terra *m_parentTerra;

        bool m_useSkirts;

    public:
        TerrainCell( Terra *parentTerra );
        virtual ~TerrainCell();

        bool getUseSkirts(void) const                   { return m_useSkirts; }

        void initialize( VaoManager *vaoManager, bool useSkirts );

        void setOrigin( const GridPoint &gridPos, uint32 horizontalPixelDim,
                        uint32 verticalPixelDim, uint32 lodLevel );

        /** Merges another TerrainCell into 'this' for reducing batch counts.
            e.g.
                Two 32x32 cells will merge into one 64x32 or 32x64
                Two 64x32 cells will merge into one 64x64
                A 32x64 cell cannot merge with a 32x32 one.
                A 64x32 cell cannot merge with a 32x32 one.
        @remarks
            Merge will only happen if the cells are of the same LOD level and are contiguous.
        @param next
            The other TerrainCell to merge with.
        @return
            False if couldn't merge, true on success.
        */
        bool merge( TerrainCell *next );

        void uploadToGpu( uint32 * RESTRICT_ALIAS gpuPtr ) const;

        //Renderable overloads
        virtual const LightList& getLights(void) const;
        virtual void getRenderOperation( v1::RenderOperation& op, bool casterPass );
        virtual void getWorldTransforms( Matrix4* xform ) const;
        virtual bool getCastsShadows(void) const;
    };
}

#endif
