
#include "Utils/ScreenSpaceReflections.h"

#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"

#include "OgreCamera.h"

namespace Demo
{
    const Ogre::Matrix4 PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE(
        0.5,    0,    0,  0.5,
        0,   -0.5,    0,  0.5,
        0,      0,    1,    0,
        0,      0,    0,    1);

    ScreenSpaceReflections::ScreenSpaceReflections()
    {
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().load(
                    "SSR/ScreenSpaceReflectionsVectors",
                    Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                staticCast<Ogre::Material>();

        Ogre::Pass *pass = material->getTechnique(0)->getPass(0);

        mPsParams = pass->getFragmentProgramParameters();
    }
    //-----------------------------------------------------------------------------------
    void ScreenSpaceReflections::update( Ogre::Camera *camera )
    {
        Ogre::Real projectionA = camera->getFarClipDistance() /
                                    (camera->getFarClipDistance() - camera->getNearClipDistance());
        Ogre::Real projectionB = (-camera->getFarClipDistance() * camera->getNearClipDistance()) /
                                    (camera->getFarClipDistance() - camera->getNearClipDistance());
        //The division will keep "linearDepth" in the shader in the [0; 1] range.
        projectionB /= camera->getFarClipDistance();
        mPsParams->setNamedConstant( "p_projectionParams",
                                     Ogre::Vector4( projectionA, projectionB, 0, 0 ) );

        Ogre::Matrix4 viewToTextureSpaceMatrix = PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE *
                camera->getProjectionMatrixWithRSDepth();
        mPsParams->setNamedConstant( "p_viewToTextureSpaceMatrix", viewToTextureSpaceMatrix );
    }
}
