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
#include "OgreBspResourceManager.h"
#include "OgreBspLevel.h"
#include "OgreQuake3ShaderManager.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    template<> BspResourceManager* Singleton<BspResourceManager>::msSingleton = 0;
    BspResourceManager* BspResourceManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    BspResourceManager& BspResourceManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    BspResourceManager::BspResourceManager()
    {
        mResourceType = "BspLevel";
        // Also create related shader manager (singleton managed)
        mShaderMgr = OGRE_NEW Quake3ShaderManager();

        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }
    //-----------------------------------------------------------------------
    BspResourceManager::~BspResourceManager()
    {
        OGRE_DELETE mShaderMgr;
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }
    //-----------------------------------------------------------------------
    ResourcePtr BspResourceManager::load(const String& name, 
        const String& group, bool isManual, 
        ManualResourceLoader* loader, const NameValuePairList* loadParams)
    {
        // Only 1 BSP level allowed loaded at once
        removeAll();

        return ResourceManager::load(name, group, isManual, loader, loadParams);

    }
    //-----------------------------------------------------------------------
    ResourcePtr BspResourceManager::load(DataStreamPtr& stream, 
		const String& group)
    {
        // Only 1 BSP level allowed loaded at once
        removeAll();

		ResourcePtr ret = createResource("bsplevel", group, true, 0);
		BspLevelPtr bspLevel = ret.staticCast<BspLevel>();
		bspLevel->load(stream);
		
        return ret;

    }
    //-----------------------------------------------------------------------
    Resource* BspResourceManager::createImpl(const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* createParams)
    {
        return OGRE_NEW BspLevel(this, name, handle, group, isManual, loader);
    }


}
