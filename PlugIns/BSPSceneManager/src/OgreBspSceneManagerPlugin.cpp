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

#include "OgreBspSceneManagerPlugin.h"
#include "OgreRoot.h"
#include "OgreCodec.h"
#include "OgreQuake3ShaderManager.h"

namespace Ogre 
{
namespace {
    struct BspSceneCodec : public Codec
    {
        String magicNumberToFileExt(const char* magicNumberPtr, size_t maxbytes) const { return ""; }
        String getType() const override { return "bsp"; }
        void decode(const DataStreamPtr& stream, const Any& output) const override
        {
            auto group = ResourceGroupManager::getSingleton().getWorldResourceGroupName();
            auto rootNode = any_cast<SceneNode*>(output);
            BspSceneManager* mgr = dynamic_cast<BspSceneManager*>(rootNode->getCreator());

            OgreAssert(mgr, "only loading into a BspSceneManager supported");

            mgr->setLevel(BspLevelPtr()); // clear

            auto bspLevel = std::make_shared<BspLevel>(nullptr, "bsplevel", 0, group, false, nullptr);
            bspLevel->load(stream);

            mgr->setLevel(bspLevel);
        }
    };
}
    const String sPluginName = "BSP Scene Manager";
    //---------------------------------------------------------------------
    BspSceneManagerPlugin::BspSceneManagerPlugin() : mBspFactory(0) {}
    BspSceneManagerPlugin::~BspSceneManagerPlugin() {}
    //---------------------------------------------------------------------
    const String& BspSceneManagerPlugin::getName() const
    {
        return sPluginName;
    }
    //---------------------------------------------------------------------
    void BspSceneManagerPlugin::install()
    {
        // Create new scene manager
        mBspFactory = OGRE_NEW BspSceneManagerFactory();

    }
    //---------------------------------------------------------------------
    void BspSceneManagerPlugin::initialise()
    {
        // Register (factory not dependent on rsys resources)
        Root::getSingleton().addSceneManagerFactory(mBspFactory);

        // Also create related shader manager (singleton managed)
        mShaderMgr = OGRE_NEW Quake3ShaderManager();

        mCodec.reset(new BspSceneCodec());
        Codec::registerCodec(mCodec.get());
    }
    //---------------------------------------------------------------------
    void BspSceneManagerPlugin::shutdown()
    {
        // Unregister SM factory
        Root::getSingleton().removeSceneManagerFactory(mBspFactory);

        Codec::unregisterCodec(mCodec.get());
        mCodec.reset();

        OGRE_DELETE mShaderMgr;
    }
    //---------------------------------------------------------------------
    void BspSceneManagerPlugin::uninstall()
    {

        OGRE_DELETE mBspFactory;
        mBspFactory = 0;

    }


}
