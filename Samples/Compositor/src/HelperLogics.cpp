/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#include "HelperLogics.h"

#include "Ogre.h"

#include "Compositor/Pass/OgreCompositorPass.h"
#include "Compositor/Pass/OgreCompositorPassDef.h"
#include "Compositor/Pass/PassQuad/OgreCompositorPassQuad.h"

#include "OgreTimer.h"


SamplePostprocessWorkspaceListener::SamplePostprocessWorkspaceListener() :
    start( 0 ),
    end( 0 ),
    curr( 0 ),
    timer( new Ogre::Timer() )
{
}
//---------------------------------------------------------------------------
SamplePostprocessWorkspaceListener::~SamplePostprocessWorkspaceListener()
{
    delete timer;
    timer = 0;
}
//---------------------------------------------------------------------------
void SamplePostprocessWorkspaceListener::passPreExecute( Ogre::CompositorPass *pass )
{
    const Ogre::CompositorPassDef *passDef = pass->getDefinition();
    switch( passDef->mIdentifier )
    {
    case 0xDEADBABE:
        onHeatVision( pass );
        break;
    case 700:
        onGaussianBlurV( pass );
        break;
    case 701:
        onGaussianBlurH( pass );
        break;
    }
}
//---------------------------------------------------------------------------
void SamplePostprocessWorkspaceListener::onHeatVision( Ogre::CompositorPass *_pass )
{
    assert( _pass->getType() == Ogre::PASS_QUAD );
    Ogre::CompositorPassQuad *pass = static_cast<Ogre::CompositorPassQuad*>( _pass );
    Ogre::Pass *matPass = pass->getPass();

    Ogre::GpuProgramParametersSharedPtr fpParams = matPass->getFragmentProgramParameters();

    // "random_fractions" parameter
    fpParams->setNamedConstant("random_fractions", Ogre::Vector4(Ogre::Math::RangeRandom(0.0, 1.0), Ogre::Math::RangeRandom(0, 1.0), 0, 0));

    // "depth_modulator" parameter
    float inc = ((float)timer->getMilliseconds())/1000.0f;
    if ( (fabs(curr-end) <= 0.001) ) {
        // take a new value to reach
        end = Ogre::Math::RangeRandom(0.95, 1.0);
        start = curr;
    } else {
        if (curr > end) curr -= inc;
        else curr += inc;
    }
    timer->reset();

    fpParams->setNamedConstant("depth_modulator", Ogre::Vector4(curr, 0, 0, 0));
}
//---------------------------------------------------------------------------
void SamplePostprocessWorkspaceListener::onGaussianBlurH( Ogre::CompositorPass *_pass )
{
    assert( _pass->getType() == Ogre::PASS_QUAD );
    Ogre::CompositorPassQuad *pass = static_cast<Ogre::CompositorPassQuad*>( _pass );
    Ogre::Pass *matPass = pass->getPass();

    Ogre::GpuProgramParametersSharedPtr fpParams = matPass->getFragmentProgramParameters();

    fpParams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
    fpParams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
}
//---------------------------------------------------------------------------
void SamplePostprocessWorkspaceListener::onGaussianBlurV( Ogre::CompositorPass *_pass )
{
    assert( _pass->getType() == Ogre::PASS_QUAD );
    Ogre::CompositorPassQuad *pass = static_cast<Ogre::CompositorPassQuad*>( _pass );
    Ogre::Pass *matPass = pass->getPass();

    Ogre::GpuProgramParametersSharedPtr fpParams = matPass->getFragmentProgramParameters();

    fpParams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
    fpParams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);
}
//---------------------------------------------------------------------------
void SamplePostprocessWorkspaceListener::windowResized( unsigned int width, unsigned int height )
{
    mVpWidth = width;
    mVpHeight = height;
    // Calculate gaussian texture offsets & weights
    float deviation = 3.0f;
    float texelSize = 1.0f / (float)std::min(mVpWidth, mVpHeight);

    // central sample, no offset
    mBloomTexOffsetsHorz[0][0] = 0.0f;
    mBloomTexOffsetsHorz[0][1] = 0.0f;
    mBloomTexOffsetsVert[0][0] = 0.0f;
    mBloomTexOffsetsVert[0][1] = 0.0f;
    mBloomTexWeights[0][0] = mBloomTexWeights[0][1] =
        mBloomTexWeights[0][2] = Ogre::Math::gaussianDistribution(0, 0, deviation);
    mBloomTexWeights[0][3] = 1.0f;

    // 'pre' samples
    for(int i = 1; i < 8; ++i)
    {
        mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
            mBloomTexWeights[i][2] = Ogre::Math::gaussianDistribution((Ogre::Real)i, 0, deviation);
        mBloomTexWeights[i][3] = 1.0f;
        mBloomTexOffsetsHorz[i][0] = i * texelSize;
        mBloomTexOffsetsHorz[i][1] = 0.0f;
        mBloomTexOffsetsVert[i][0] = 0.0f;
        mBloomTexOffsetsVert[i][1] = i * texelSize;
    }
    // 'post' samples
    for(int i = 8; i < 15; ++i)
    {
        mBloomTexWeights[i][0] = mBloomTexWeights[i][1] =
            mBloomTexWeights[i][2] = mBloomTexWeights[i - 7][0];
        mBloomTexWeights[i][3] = 1.0f;

        mBloomTexOffsetsHorz[i][0] = -mBloomTexOffsetsHorz[i - 7][0];
        mBloomTexOffsetsHorz[i][1] = 0.0f;
        mBloomTexOffsetsVert[i][0] = 0.0f;
        mBloomTexOffsetsVert[i][1] = -mBloomTexOffsetsVert[i - 7][1];
    }
}
