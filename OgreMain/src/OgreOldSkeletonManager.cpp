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
#include "OgreStableHeaders.h"
#include "OgreOldSkeletonManager.h"

#include "OgreSkeleton.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    template<> v1::OldSkeletonManager* Singleton<v1::OldSkeletonManager>::msSingleton = 0;
namespace v1
{
    OldSkeletonManager* OldSkeletonManager::getSingletonPtr(void)
    {
        return msSingleton;
    }
    OldSkeletonManager& OldSkeletonManager::getSingleton(void)
    {  
        assert( msSingleton );  return ( *msSingleton );  
    }
    //-----------------------------------------------------------------------
    OldSkeletonManager::OldSkeletonManager()
    {
        mLoadOrder = 300.0f;
        mResourceType = "OldSkeleton";

        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }
    //-----------------------------------------------------------------------
    SkeletonPtr OldSkeletonManager::getByName(const String& name, const String& groupName)
    {
        return getResourceByName(name, groupName).staticCast<Skeleton>();
    }
    //-----------------------------------------------------------------------
    SkeletonPtr OldSkeletonManager::create (const String& name, const String& group,
                                    bool isManual, ManualResourceLoader* loader,
                                    const NameValuePairList* createParams)
    {
        return createResource(name,group,isManual,loader,createParams).staticCast<Skeleton>();
    }
    //-----------------------------------------------------------------------
    OldSkeletonManager::~OldSkeletonManager()
    {
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }
    //-----------------------------------------------------------------------
    Resource* OldSkeletonManager::createImpl(const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* createParams)
    {
        return OGRE_NEW Skeleton(this, name, handle, group, isManual, loader);
    }


}
}
