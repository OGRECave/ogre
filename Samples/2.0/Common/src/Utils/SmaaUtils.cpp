
#include "Utils/SmaaUtils.h"

#include "OgreRoot.h"

#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"

namespace Demo
{
    void SmaaUtils::initialize( Ogre::RenderSystem *renderSystem, PresetQuality quality )
    {
        const Ogre::RenderSystemCapabilities *caps = renderSystem->getCapabilities();

        Ogre::String materialNames[3] =
        {
            "SMAA/EdgeDetection",
            "SMAA/BlendingWeightCalculation",
            "SMAA/NeighborhoodBlending"
        };

        Ogre::String preprocessorDefines = "-DSMAA_INITIALIZED=1,";

        switch( quality )
        {
        case SMAA_PRESET_LOW:
            preprocessorDefines += "-DSMAA_PRESET_LOW=1,";
            break;
        case SMAA_PRESET_MEDIUM:
            preprocessorDefines += "-DSMAA_PRESET_MEDIUM=1,";
            break;
        case SMAA_PRESET_HIGH:
            preprocessorDefines += "-DSMAA_PRESET_HIGH=1,";
            break;
        case SMAA_PRESET_ULTRA:
            preprocessorDefines += "-DSMAA_PRESET_ULTRA=1,";
            break;
        }

        if( caps->isShaderProfileSupported( "ps_4_1" ) )
            preprocessorDefines += "-DSMAA_HLSL_4_1=1,";
        else if( caps->isShaderProfileSupported( "ps_4_0" ) )
            preprocessorDefines += "-DSMAA_HLSL_4=1,";
        else if( caps->isShaderProfileSupported( "glsl410" ) )
            preprocessorDefines += "-DSMAA_GLSL_4=1,";
        else if( caps->isShaderProfileSupported( "glsl330" ) )
            preprocessorDefines += "-DSMAA_GLSL_3=1,";

        for( size_t i=0; i<sizeof(materialNames) / sizeof(materialNames[0]); ++i )
        {
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().load(
                        materialNames[i],
                        Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                    staticCast<Ogre::Material>();

            Ogre::Pass *pass = material->getTechnique(0)->getPass(0);

            Ogre::GpuProgram *shader = 0;
            Ogre::GpuProgramParametersSharedPtr oldParams;

            //Save old manual & auto params
            oldParams = pass->getVertexProgramParameters();
            //Retrieve the HLSL/GLSL/Metal shader and rebuild it with the right settings.
            shader = pass->getVertexProgram()->_getBindingDelegate();
            shader->setParameter( "preprocessor_defines", preprocessorDefines );
            pass->getVertexProgram()->reload();
            //Restore manual & auto params to the newly compiled shader
            pass->getVertexProgramParameters()->copyConstantsFrom( *oldParams );

            //Save old manual & auto params
            oldParams = pass->getFragmentProgramParameters();
            //Retrieve the HLSL/GLSL/Metal shader and rebuild it with the right settings.
            shader = pass->getFragmentProgram()->_getBindingDelegate();
            shader->setParameter( "preprocessor_defines", preprocessorDefines );
            pass->getFragmentProgram()->reload();
            //Restore manual & auto params to the newly compiled shader
            pass->getFragmentProgramParameters()->copyConstantsFrom( *oldParams );
        }
    }
}
