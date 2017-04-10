
#include "Utils/ScreenSpaceReflections.h"

#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgreRenderSystem.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorNodeDef.h"

#include "OgreCamera.h"

namespace Demo
{
    const Ogre::Matrix4 PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE(
        0.5,    0,    0,  0.5,
        0,   -0.5,    0,  0.5,
        0,      0,    1,    0,
        0,      0,    0,    1);

    ScreenSpaceReflections::ScreenSpaceReflections( const Ogre::TexturePtr &globalCubemap,
                                                    Ogre::RenderSystem *renderSystem ) :
        mLastUvSpaceViewProjMatrix( PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE ),
        mRsDepthRange( 1.0f )
    {
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().load(
                    "SSR/ScreenSpaceReflectionsVectors",
                    Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                staticCast<Ogre::Material>();

        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        mPsParams[0] = pass->getFragmentProgramParameters();

        material = Ogre::MaterialManager::getSingleton().load(
                            "SSR/ScreenSpaceReflectionsCombine",
                            Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                        staticCast<Ogre::Material>();
        pass = material->getTechnique(0)->getPass(0);
        mPsParams[1] = pass->getFragmentProgramParameters();

        //pass->getTextureUnitState( "globalCubemap" )->setTexture( globalCubemap );
        mRsDepthRange = renderSystem->getRSDepthRange();
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflections::update( Ogre::Camera *camera )
    {
        Ogre::Real projectionA = camera->getFarClipDistance() /
                                    (camera->getFarClipDistance() - camera->getNearClipDistance());
        Ogre::Real projectionB = (-camera->getFarClipDistance() * camera->getNearClipDistance()) /
                                    (camera->getFarClipDistance() - camera->getNearClipDistance());
        for( int i=0; i<2; ++i )
        {
            //The division will keep "linearDepth" in the shader in the [0; 1] range.
            //projectionB /= camera->getFarClipDistance();
            mPsParams[0]->setNamedConstant( "projectionParams",
                                            Ogre::Vector4( projectionA, projectionB, 0, 0 ) );
        }

        Ogre::Matrix4 viewToTextureSpaceMatrix = camera->getProjectionMatrix();
        // Convert depth range from [-1,+1] to [0,1]
        viewToTextureSpaceMatrix[2][0] = (viewToTextureSpaceMatrix[2][0] + viewToTextureSpaceMatrix[3][0]) / 2;
        viewToTextureSpaceMatrix[2][1] = (viewToTextureSpaceMatrix[2][1] + viewToTextureSpaceMatrix[3][1]) / 2;
        viewToTextureSpaceMatrix[2][2] = (viewToTextureSpaceMatrix[2][2] + viewToTextureSpaceMatrix[3][2]) / 2;
        viewToTextureSpaceMatrix[2][3] = (viewToTextureSpaceMatrix[2][3] + viewToTextureSpaceMatrix[3][3]) / 2;

        // Convert right-handed to left-handed
        viewToTextureSpaceMatrix[0][2] = -viewToTextureSpaceMatrix[0][2];
        viewToTextureSpaceMatrix[1][2] = -viewToTextureSpaceMatrix[1][2];
        viewToTextureSpaceMatrix[2][2] = -viewToTextureSpaceMatrix[2][2];
        viewToTextureSpaceMatrix[3][2] = -viewToTextureSpaceMatrix[3][2];

        viewToTextureSpaceMatrix = PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE * viewToTextureSpaceMatrix;

        mPsParams[0]->setNamedConstant( "viewToTextureSpaceMatrix", viewToTextureSpaceMatrix );
        mPsParams[1]->setNamedConstant( "textureSpaceToViewSpace", viewToTextureSpaceMatrix.inverse() );

        Ogre::Matrix4 viewMatrix = camera->getViewMatrix(true);
        Ogre::Matrix3 viewMatrix3, invViewMatrixCubemap;
        viewMatrix.extract3x3Matrix( viewMatrix3 );
        //Cubemaps are left-handed.
        invViewMatrixCubemap = viewMatrix3;
        invViewMatrixCubemap[0][2] = -invViewMatrixCubemap[0][2];
        invViewMatrixCubemap[1][2] = -invViewMatrixCubemap[1][2];
        invViewMatrixCubemap[2][2] = -invViewMatrixCubemap[2][2];
        invViewMatrixCubemap = invViewMatrixCubemap.Inverse();

        mPsParams[1]->setNamedConstant( "invViewMatCubemap", &invViewMatrixCubemap[0][0], 3*3, 1 );

        //Why do we need to 2x the camera position in GL (so that difference is 2x)? I have no clue,
        //but could be realted with OpenGL's depth range being in range [-1;1] and projection magic.
        viewMatrix[0][3] *= mRsDepthRange;
        viewMatrix[1][3] *= mRsDepthRange;
        viewMatrix[2][3] *= mRsDepthRange;
        Ogre::Matrix4 projMatrix = camera->getProjectionMatrixWithRSDepth();
        Ogre::Matrix4 uvSpaceViewProjMatrix =
                (PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE * projMatrix) * viewMatrix;

        Ogre::Matrix4 reprojectionMatrix = mLastUvSpaceViewProjMatrix * uvSpaceViewProjMatrix.inverse();
        mPsParams[0]->setNamedConstant( "reprojectionMatrix", reprojectionMatrix );

        mLastUvSpaceViewProjMatrix = uvSpaceViewProjMatrix;
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflections::setupSSR( bool useMsaa, bool useHq,
                                           Ogre::CompositorManager2 *compositorManager )
    {
        Ogre::String preprocessDefines;

        if( useMsaa )
            preprocessDefines += "USE_MSAA=1;";
        if( useHq )
            preprocessDefines += "HQ=1;";

        Ogre::MaterialPtr material;
        Ogre::GpuProgram *psShader = 0;
        Ogre::GpuProgramParametersSharedPtr oldParams;
        Ogre::Pass *pass = 0;

        material = Ogre::MaterialManager::getSingleton().load(
                    "SSR/ScreenSpaceReflectionsVectors",
                    Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                staticCast<Ogre::Material>();

        pass = material->getTechnique(0)->getPass(0);
        //Save old manual & auto params
        oldParams = pass->getFragmentProgramParameters();
        //Retrieve the HLSL/GLSL/Metal shader and rebuild it with/without MSAA & HQ
        psShader = pass->getFragmentProgram()->_getBindingDelegate();
        psShader->setParameter( "preprocessor_defines", preprocessDefines );
        pass->getFragmentProgram()->reload();
        //Restore manual & auto params to the newly compiled shader
        pass->getFragmentProgramParameters()->copyConstantsFrom( *oldParams );

        material = Ogre::MaterialManager::getSingleton().load(
                    "SSR/ScreenSpaceReflectionsCombine",
                    Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                staticCast<Ogre::Material>();
        pass = material->getTechnique(0)->getPass(0);
        //Save old manual & auto params
        oldParams = pass->getFragmentProgramParameters();
        //Retrieve the HLSL/GLSL/Metal shader and rebuild it with/without MSAA & HQ
        psShader = pass->getFragmentProgram()->_getBindingDelegate();
        psShader->setParameter( "preprocessor_defines", preprocessDefines );
        pass->getFragmentProgram()->reload();
        //Restore manual & auto params to the newly compiled shader
        pass->getFragmentProgramParameters()->copyConstantsFrom( *oldParams );

        Ogre::CompositorNodeDef *nodeDef = 0;
        nodeDef = compositorManager->getNodeDefinitionNonConst("ScreenSpaceReflectionsPostprocessNode");
        Ogre::TextureDefinitionBase::TextureDefinitionVec &textureDefs =
                nodeDef->getLocalTextureDefinitionsNonConst();

        Ogre::TextureDefinitionBase::TextureDefinitionVec::iterator itor = textureDefs.begin();
        Ogre::TextureDefinitionBase::TextureDefinitionVec::iterator end  = textureDefs.end();

        while( itor != end )
        {
            if( itor->getName() == "rayTracingBuffer" )
            {
                if( useHq )
                {
                    itor->widthFactor   = 1.0f;
                    itor->heightFactor  = 1.0f;
                }
                else
                {
                    itor->widthFactor   = 0.5f;
                    itor->heightFactor  = 0.5f;
                }
                break;
            }
            ++itor;
        }
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflections::setupSSRValues( double equivalentMetersInCurrentUnit,
                                                 double zThickness,
                                                 double zThicknessBiasAmount,
                                                 double zThicknessBiasStart,
                                                 double zThicknessBiasEnd,
                                                 float maxDistance,
                                                 float reprojectionMaxDistanceError,
                                                 Ogre::uint16 pixelStride,
                                                 Ogre::uint16 maxSteps )
    {
        assert( zThicknessBiasStart < zThicknessBiasEnd );
        assert( maxDistance > 0 );
        assert( reprojectionMaxDistanceError >= 0 );
        assert( pixelStride > 0 );
        assert( maxSteps > 0 );

        zThickness                  *= equivalentMetersInCurrentUnit;
        zThicknessBiasStart         *= equivalentMetersInCurrentUnit;
        zThicknessBiasEnd           *= equivalentMetersInCurrentUnit;
        zThicknessBiasAmount        *= equivalentMetersInCurrentUnit;
        maxDistance                 *= equivalentMetersInCurrentUnit;
        reprojectionMaxDistanceError*= equivalentMetersInCurrentUnit;

        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().load(
                    "SSR/ScreenSpaceReflectionsVectors",
                    Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                staticCast<Ogre::Material>();

        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);
        Ogre::GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();

        const double biasRangeTimesAmount = zThicknessBiasAmount /
                                            (zThicknessBiasEnd - zThicknessBiasStart);
        const double biasStartTimesRangeTimesAmount = zThicknessBiasStart * biasRangeTimesAmount;

        psParams->setNamedConstant( "zThickness",
                                    Ogre::Vector4( Ogre::Real( zThickness ),
                                                   Ogre::Real( biasRangeTimesAmount ),
                                                   Ogre::Real( -biasStartTimesRangeTimesAmount ),
                                                   Ogre::Real( zThicknessBiasAmount ) ) );
        psParams->setNamedConstant( "maxDistance", maxDistance );
        psParams->setNamedConstant( "reprojectionMaxDistanceError", reprojectionMaxDistanceError );
        psParams->setNamedConstant( "stride", (float)pixelStride );
        psParams->setNamedConstant( "maxSteps", (float)maxSteps );
    }
}
