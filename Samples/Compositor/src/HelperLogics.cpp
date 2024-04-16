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

#include <cmath>

#include "Ogre.h"
#include "OgreTimer.h"

//---------------------------------------------------------------------------
class HeatVisionListener: public Ogre::CompositorInstance::Listener
{
public:
    HeatVisionListener();
    virtual ~HeatVisionListener();
    void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) override;
    void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) override;
protected:
    Ogre::GpuProgramParametersSharedPtr fpParams;
    float start, end, curr;
    Ogre::Timer *timer;
};
//---------------------------------------------------------------------------
class HDRListener: public Ogre::CompositorInstance::Listener
{
protected:
    int mVpWidth, mVpHeight;
    int mBloomSize;
    // Array params - have to pack in groups of 4 since this is how Cg generates them
    // also prevents dependent texture read problems if ops don't require swizzle
    float mBloomTexWeights[15][4];
    float mBloomTexOffsetsHorz[15][4];
    float mBloomTexOffsetsVert[15][4];
public:
    HDRListener();
    virtual ~HDRListener();
    void notifyViewportSize(int width, int height);
    void notifyCompositor(Ogre::CompositorInstance* instance);
    void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) override;
    void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) override;
};
//---------------------------------------------------------------------------
class GaussianListener : public Ogre::CompositorInstance::Listener
{
protected:
    int mVpWidth, mVpHeight;
    // Array params - have to pack in groups of 4 since this is how Cg generates them
    // also prevents dependent texture read problems if ops don't require swizzle
    float mBloomTexWeights[15][4];
    float mBloomTexOffsetsHorz[15][4];
    float mBloomTexOffsetsVert[15][4];
public:
    GaussianListener();
    virtual ~GaussianListener();
    void notifyViewportSize(int width, int height);
    void notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) override;
    void notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat) override;
};
//---------------------------------------------------------------------------
Ogre::CompositorInstance::Listener* HDRLogic::createListener(Ogre::CompositorInstance* instance)
{
    HDRListener* listener = new HDRListener;
    Ogre::Viewport* vp = instance->getChain()->getViewport();
    listener->notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
    listener->notifyCompositor(instance);
    return listener;
}
//---------------------------------------------------------------------------
Ogre::CompositorInstance::Listener* HeatVisionLogic::createListener(Ogre::CompositorInstance* instance)
{
    return new HeatVisionListener;
}
//---------------------------------------------------------------------------
Ogre::CompositorInstance::Listener* GaussianBlurLogic::createListener(Ogre::CompositorInstance* instance)
{
    GaussianListener* listener = new GaussianListener;
    Ogre::Viewport* vp = instance->getChain()->getViewport();
    listener->notifyViewportSize(vp->getActualWidth(), vp->getActualHeight());
    return listener;
}
/*************************************************************************
                        HeatVisionListener Methods
*************************************************************************/
//---------------------------------------------------------------------------
HeatVisionListener::HeatVisionListener()
{
    timer = new Ogre::Timer();
    start = end = curr = 0.0f;
}
//---------------------------------------------------------------------------
HeatVisionListener::~HeatVisionListener()
{
   delete timer;
}
//---------------------------------------------------------------------------
void HeatVisionListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
    if(pass_id == 0xDEADBABE)
    {
        timer->reset();
        fpParams =
            mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
    }
}
//---------------------------------------------------------------------------
void HeatVisionListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
    if(pass_id == 0xDEADBABE)
    {
        // "random_fractions" parameter
        fpParams->setNamedConstant("random_fractions", Ogre::Vector4(Ogre::Math::RangeRandom(0.0, 1.0), Ogre::Math::RangeRandom(0, 1.0), 0, 0));

        // "depth_modulator" parameter
        float inc = ((float)timer->getMilliseconds())/1000.0f;
        if ( (std::fabs(curr-end) <= 0.001) ) {
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
}
//---------------------------------------------------------------------------

/*************************************************************************
HDRListener Methods
*************************************************************************/
//---------------------------------------------------------------------------
HDRListener::HDRListener()
{
}
//---------------------------------------------------------------------------
HDRListener::~HDRListener()
{
}
//---------------------------------------------------------------------------
void HDRListener::notifyViewportSize(int width, int height)
{
    mVpWidth = width;
    mVpHeight = height;
}
//---------------------------------------------------------------------------
void HDRListener::notifyCompositor(Ogre::CompositorInstance* instance)
{
    // Get some RTT dimensions for later calculations
    for (const auto *t : instance->getTechnique()->getTextureDefinitions())
    {
        if(t->name == "rt_bloom0")
        {
            mBloomSize = (int)t->width; // should be square
            // Calculate gaussian texture offsets & weights
            float deviation = 3.0f;
            float texelSize = 1.0f / (float)mBloomSize;

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
                    mBloomTexWeights[i][2] = 1.25f * Ogre::Math::gaussianDistribution((Ogre::Real)i, 0, deviation);
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
    }
}
//---------------------------------------------------------------------------
void HDRListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
    // Prepare the fragment params offsets
    switch(pass_id)
    {
    //case 994: // rt_lum4
    case 993: // rt_lum3
    case 992: // rt_lum2
    case 991: // rt_lum1
    case 990: // rt_lum0
        break;
    case 800: // rt_brightpass
        break;
    case 701: // rt_bloom1
        {
            // horizontal bloom
            mat->load();
            Ogre::GpuProgramParametersSharedPtr fparams =
                mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
            fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
            fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

            break;
        }
    case 700: // rt_bloom0
        {
            // vertical bloom
            mat->load();
            Ogre::GpuProgramParametersSharedPtr fparams =
                mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
            fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
            fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

            break;
        }
    }
}
//---------------------------------------------------------------------------
void HDRListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
}
//---------------------------------------------------------------------------


/*************************************************************************
GaussianListener Methods
*************************************************************************/
//---------------------------------------------------------------------------
GaussianListener::GaussianListener()
{
}
//---------------------------------------------------------------------------
GaussianListener::~GaussianListener()
{
}
//---------------------------------------------------------------------------
void GaussianListener::notifyViewportSize(int width, int height)
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
//---------------------------------------------------------------------------
void GaussianListener::notifyMaterialSetup(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
    // Prepare the fragment params offsets
    switch(pass_id)
    {
    case 701: // blur horz
        {
            // horizontal bloom
            mat->load();
            Ogre::GpuProgramParametersSharedPtr fparams =
                mat->getBestTechnique()->getPass(0)->getFragmentProgramParameters();
            fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsHorz[0], 15);
            fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

            break;
        }
    case 700: // blur vert
        {
            // vertical bloom
            mat->load();
            Ogre::GpuProgramParametersSharedPtr fparams =
                mat->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
            fparams->setNamedConstant("sampleOffsets", mBloomTexOffsetsVert[0], 15);
            fparams->setNamedConstant("sampleWeights", mBloomTexWeights[0], 15);

            break;
        }
    }
}
//---------------------------------------------------------------------------
void GaussianListener::notifyMaterialRender(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
}
//---------------------------------------------------------------------------
