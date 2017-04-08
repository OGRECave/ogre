
#include "Utils/MiscUtils.h"

#include "OgreHlmsCompute.h"
#include "OgreHlmsComputeJob.h"
#include "OgreLwString.h"

#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"

namespace Demo
{
    void MiscUtils::setGaussianLogFilterParams( Ogre::HlmsComputeJob *job, Ogre::uint8 kernelRadius,
                                                float gaussianDeviationFactor, Ogre::uint16 K )
    {
        using namespace Ogre;

        assert( !(kernelRadius & 0x01) && "kernelRadius must be even!" );

        if( job->getProperty( "kernel_radius" ) != kernelRadius )
            job->setProperty( "kernel_radius", kernelRadius );
        if( job->getProperty( "K" ) != K )
            job->setProperty( "K", K );
        ShaderParams &shaderParams = job->getShaderParams( "default" );

        std::vector<float> weights( kernelRadius + 1u );

        const float fKernelRadius = kernelRadius;
        const float gaussianDeviation = fKernelRadius * gaussianDeviationFactor;

        //It's 2.0f if using the approximate filter (sampling between two pixels to
        //get the bilinear interpolated result and cut the number of samples in half)
        const float stepSize = 1.0f;

        //Calculate the weights
        float fWeightSum = 0;
        for( uint32 i=0; i<kernelRadius + 1u; ++i )
        {
            const float _X = i - fKernelRadius + ( 1.0f - 1.0f / stepSize );
            float fWeight = 1.0f / sqrt ( 2.0f * Math::PI * gaussianDeviation * gaussianDeviation );
            fWeight *= exp( - ( _X * _X ) / ( 2.0f * gaussianDeviation * gaussianDeviation ) );

            fWeightSum += fWeight;
            weights[i] = fWeight;
        }

        fWeightSum = fWeightSum * 2.0f - weights[kernelRadius];

        //Normalize the weights
        for( uint32 i=0; i<kernelRadius + 1u; ++i )
            weights[i] /= fWeightSum;

        //Remove shader constants from previous calls (needed in case we've reduced the radius size)
        ShaderParams::ParamVec::iterator itor = shaderParams.mParams.begin();
        ShaderParams::ParamVec::iterator end  = shaderParams.mParams.end();

        while( itor != end )
        {
            String::size_type pos = itor->name.find( "c_weights[" );

            if( pos != String::npos )
            {
                itor = shaderParams.mParams.erase( itor );
                end  = shaderParams.mParams.end();
            }
            else
            {
                ++itor;
            }
        }

        //Set the shader constants, 16 at a time (since that's the limit of what ManualParam can hold)
        char tmp[32];
        LwString weightsString( LwString::FromEmptyPointer( tmp, sizeof(tmp) ) );
        const uint32 floatsPerParam = sizeof( ShaderParams::ManualParam().dataBytes ) / sizeof(float);
        for( uint32 i=0; i<kernelRadius + 1u; i += floatsPerParam )
        {
            weightsString.clear();
            weightsString.a( "c_weights[", i, "]" );

            ShaderParams::Param p;
            p.isAutomatic   = false;
            p.isDirty       = true;
            p.name = weightsString.c_str();
            shaderParams.mParams.push_back( p );
            ShaderParams::Param *param = &shaderParams.mParams.back();

            param->setManualValue( &weights[i], std::min<uint32>( floatsPerParam, weights.size() - i ) );
        }

        shaderParams.setDirty();
    }
    //-----------------------------------------------------------------------------------
    int MiscUtils::retrievePreprocessorParameter( const Ogre::String &preprocessDefines,
                                                  const Ogre::String &paramName )
    {
        size_t startPos = preprocessDefines.find( paramName + '=' );
        if( startPos == Ogre::String::npos )
        {
            OGRE_EXCEPT( Ogre::Exception::ERR_INVALID_STATE,
                         "Corrupted material? Expected: " + paramName +
                         " but preprocessor defines are: " + preprocessDefines,
                         "MiscUtils::retrievePreprocessorParameter" );
        }

        startPos += paramName.size() + 1u;

        size_t endPos = preprocessDefines.find_first_of( ";,", startPos );

        Ogre::String valuePart = preprocessDefines.substr( startPos, endPos - startPos );
        const int retVal = Ogre::StringConverter::parseInt( valuePart );
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void MiscUtils::setGaussianLogFilterParams( const Ogre::String &materialName,
                                                Ogre::uint8 kernelRadius,
                                                float gaussianDeviationFactor,
                                                Ogre::uint16 K )
    {
        using namespace Ogre;

        assert( !(kernelRadius & 0x01) && "kernelRadius must be even!" );

        MaterialPtr material;
        GpuProgram *psShader = 0;
        GpuProgramParametersSharedPtr oldParams;
        Pass *pass = 0;

        material = MaterialManager::getSingleton().load(
                    materialName,
                    ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME ).
                staticCast<Material>();

        pass = material->getTechnique(0)->getPass(0);
        //Save old manual & auto params
        oldParams = pass->getFragmentProgramParameters();
        //Retrieve the HLSL/GLSL/Metal shader and rebuild it with new kernel radius
        psShader = pass->getFragmentProgram()->_getBindingDelegate();

        String preprocessDefines = psShader->getParameter( "preprocessor_defines" );
        int oldNumWeights = retrievePreprocessorParameter( preprocessDefines, "NUM_WEIGHTS" );
        int oldK = retrievePreprocessorParameter( preprocessDefines, "K" );
        if( oldNumWeights != (kernelRadius + 1) || oldK != K )
        {
            int horizontalStep  = retrievePreprocessorParameter( preprocessDefines, "HORIZONTAL_STEP" );
            int verticalStep    = retrievePreprocessorParameter( preprocessDefines, "VERTICAL_STEP" );

            char tmp[64];
            LwString preprocessString( LwString::FromEmptyPointer( tmp, sizeof(tmp) ) );

            preprocessString.a( "NUM_WEIGHTS=",     kernelRadius + 1u );
            preprocessString.a( ",K=",              K );
            preprocessString.a( ",HORIZONTAL_STEP=",horizontalStep );
            preprocessString.a( ",VERTICAL_STEP=",  verticalStep );

            psShader->setParameter( "preprocessor_defines", preprocessString.c_str() );
            pass->getFragmentProgram()->reload();
            //Restore manual & auto params to the newly compiled shader
            pass->getFragmentProgramParameters()->copyConstantsFrom( *oldParams );
        }

        std::vector<float> weights( kernelRadius + 1u );

        const float fKernelRadius = kernelRadius;
        const float gaussianDeviation = fKernelRadius * gaussianDeviationFactor;

        //It's 2.0f if using the approximate filter (sampling between two pixels to
        //get the bilinear interpolated result and cut the number of samples in half)
        const float stepSize = 1.0f;

        //Calculate the weights
        float fWeightSum = 0;
        for( uint32 i=0; i<kernelRadius + 1u; ++i )
        {
            const float _X = i - fKernelRadius + ( 1.0f - 1.0f / stepSize );
            float fWeight = 1.0f / sqrt ( 2.0f * Math::PI * gaussianDeviation * gaussianDeviation );
            fWeight *= exp( - ( _X * _X ) / ( 2.0f * gaussianDeviation * gaussianDeviation ) );

            fWeightSum += fWeight;
            weights[i] = fWeight;
        }

        fWeightSum = fWeightSum * 2.0f - weights[kernelRadius];

        //Normalize the weights
        for( uint32 i=0; i<kernelRadius + 1u; ++i )
            weights[i] /= fWeightSum;

        GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();
        psParams->setNamedConstant( "weights", &weights[0], kernelRadius + 1u, 1 );
    }
}
