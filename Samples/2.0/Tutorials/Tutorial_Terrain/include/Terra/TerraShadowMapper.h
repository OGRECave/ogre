
#ifndef _OgreTerraShadowMapper_H_
#define _OgreTerraShadowMapper_H_

#include "OgrePrerequisites.h"
#include "OgreMovableObject.h"
#include "OgreShaderParams.h"

#include "Terra/TerrainCell.h"

namespace Ogre
{
    struct CompositorChannel;

    class ShadowMapper
    {
        Ogre::TexturePtr    m_heightMapTex;

        ConstBufferPacked   *m_shadowStarts;
        ConstBufferPacked   *m_shadowPerGroupData;
        CompositorWorkspace *m_shadowWorkspace;
        TexturePtr          m_shadowMapTex;
        HlmsComputeJob      *m_shadowJob;
        ShaderParams::Param *m_jobParamDelta;
        ShaderParams::Param *m_jobParamXYStep;
        ShaderParams::Param *m_jobParamIsStep;
        ShaderParams::Param *m_jobParamHeightDelta;

        //Ogre stuff
        SceneManager            *m_sceneManager;
        CompositorManager2      *m_compositorManager;

        static inline size_t getStartsPtrCount( int32 *starts, int32 *startsBase );

        /** Gets how many steps are needed in Bresenham's algorithm to reach certain height,
            given its dx / dy ratio where:
                dx = abs( x1 - x0 );
                dy = abs( y1 - y0 );
            and Bresenham is drawn in ranges [x0; x1) and [y0; y1)
        @param y
            Height to reach
        @param fStep
            (dx * 0.5f) / dy;
        @return
            Number of X iterations needed to reach the the pixel at height 'y'
            The returned value is at position (retVal; y)
            which means (retVal-1; y-1) is true unless y = 0;
        */
        static inline int32 getXStepsNeededToReachY( uint32 y, float fStep );

        /** Calculates the value of the error at position x = xIterationsToSkip from
            Bresenham's algorithm.
        @remarks
            We use this function so we can start Bresenham from '0' but resuming as
            if we wouldn't be starting from 0.
        @param xIterationsToSkip
            The X position in which we want the error.
        @param dx
            delta.x
        @param dy
            delta.y
        @return
            The error at position (xIterationsToSkip; y)
        */
        static inline float getErrorAfterXsteps( uint32 xIterationsToSkip, float dx, float dy );

        static void setGaussianFilterParams( HlmsComputeJob *job, uint8 kernelRadius,
                                             float gaussianDeviationFactor=0.5f );

    public:
        ShadowMapper( SceneManager *sceneManager, CompositorManager2 *compositorManager );
        ~ShadowMapper();

        /** Sets the parameter of the gaussian filter we apply to the shadow map.
        @param kernelRadius
            Kernel radius. Must be an even number.
        @param gaussianDeviationFactor
            Expressed in terms of gaussianDeviation = kernelRadius * gaussianDeviationFactor
        */
        void setGaussianFilterParams( uint8 kernelRadius, float gaussianDeviationFactor=0.5f );

        void createShadowMap( IdType id, TexturePtr &heightMapTex );
        void destroyShadowMap(void);
        void updateShadowMap( const Vector3 &lightDir, const Vector2 &xzDimensions, float heightScale );

        void fillUavDataForCompositorChannel( CompositorChannel &outChannel,
                                              ResourceLayoutMap &outInitialLayouts,
                                              ResourceAccessMap &outInitialUavAccess ) const;

        Ogre::TexturePtr getShadowMapTex(void) const            { return m_shadowMapTex; }
    };
}

#endif
