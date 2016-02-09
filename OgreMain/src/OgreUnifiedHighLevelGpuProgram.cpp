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
#include "OgreUnifiedHighLevelGpuProgram.h"
#include "OgreException.h"
#include "OgreGpuProgramManager.h"

namespace Ogre
{
    //-----------------------------------------------------------------------
    UnifiedHighLevelGpuProgram::CmdDelegate UnifiedHighLevelGpuProgram::msCmdDelegate;
    static const String sLanguage = "unified";
    std::map<String,int> UnifiedHighLevelGpuProgram::mLanguagePriorities;

    int UnifiedHighLevelGpuProgram::getPriority(String shaderLanguage)
    {
        std::map<String,int>::iterator it = mLanguagePriorities.find(shaderLanguage);
        if (it == mLanguagePriorities.end())
            return -1;
        else 
            return (*it).second;
    }

    void UnifiedHighLevelGpuProgram::setPrioriry(String shaderLanguage,int priority)
    {
        mLanguagePriorities[shaderLanguage] = priority;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    UnifiedHighLevelGpuProgram::UnifiedHighLevelGpuProgram(
        ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        :HighLevelGpuProgram(creator, name, handle, group, isManual, loader)
    {
        if (createParamDictionary("UnifiedHighLevelGpuProgram"))
        {
            setupBaseParamDictionary();

            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("delegate", 
                "Additional delegate programs containing implementations.",
                PT_STRING),&msCmdDelegate);
        }

    }
    //-----------------------------------------------------------------------
    UnifiedHighLevelGpuProgram::~UnifiedHighLevelGpuProgram()
    {

    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::chooseDelegate() const
    {
        OGRE_LOCK_AUTO_MUTEX;

        mChosenDelegate.setNull();

        HighLevelGpuProgramPtr tmpDelegate;
        tmpDelegate.setNull();
        int tmpPriority = -1;

        for (StringVector::const_iterator i = mDelegateNames.begin(); i != mDelegateNames.end(); ++i)
        {
            HighLevelGpuProgramPtr deleg = 
                HighLevelGpuProgramManager::getSingleton().getByName(*i);

            // Silently ignore missing links
            if(!deleg.isNull() && deleg->isSupported())
            {
                int priority = getPriority(deleg->getLanguage());
                //Find the delegate with the highest prioriry
                if (priority >= tmpPriority)
                {
                    tmpDelegate = deleg;
                    tmpPriority = priority;
                }
            }

        }

        mChosenDelegate = tmpDelegate;
    }
    //-----------------------------------------------------------------------
    const HighLevelGpuProgramPtr& UnifiedHighLevelGpuProgram::_getDelegate() const
    {
        if (mChosenDelegate.isNull())
        {
            chooseDelegate();
        }
        return mChosenDelegate;
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::addDelegateProgram(const String& name)
    {
            OGRE_LOCK_AUTO_MUTEX;

        mDelegateNames.push_back(name);

        // reset chosen delegate
        mChosenDelegate.setNull();

    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::clearDelegatePrograms()
    {
            OGRE_LOCK_AUTO_MUTEX;

        mDelegateNames.clear();
        mChosenDelegate.setNull();

    }
    //-----------------------------------------------------------------------------
    size_t UnifiedHighLevelGpuProgram::calculateSize(void) const
    {
        size_t memSize = 0;

        memSize += HighLevelGpuProgram::calculateSize();

        // Delegate Names
        for (StringVector::const_iterator i = mDelegateNames.begin(); i != mDelegateNames.end(); ++i)
            memSize += (*i).size() * sizeof(char);

        return memSize;
    }
    //-----------------------------------------------------------------------
    const String& UnifiedHighLevelGpuProgram::getLanguage(void) const
    {
        return sLanguage;
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr UnifiedHighLevelGpuProgram::createParameters(void)
    {
        if (isSupported())
        {
            return _getDelegate()->createParameters();
        }
        else
        {
            // return a default set
            GpuProgramParametersSharedPtr params = GpuProgramManager::getSingleton().createParameters();
            // avoid any errors on parameter names that don't exist
            params->setIgnoreMissingParams(true);
            return params;
        }
    }
    //-----------------------------------------------------------------------
    GpuProgram* UnifiedHighLevelGpuProgram::_getBindingDelegate(void)
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->_getBindingDelegate();
        else
            return 0;
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::isSupported(void) const
    {
        // Supported if one of the delegates is
        return !(_getDelegate().isNull());
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::isSkeletalAnimationIncluded(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->isSkeletalAnimationIncluded();
        else
            return false;
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::isMorphAnimationIncluded(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->isMorphAnimationIncluded();
        else
            return false;
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::isPoseAnimationIncluded(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->isPoseAnimationIncluded();
        else
            return false;
    }
    //-----------------------------------------------------------------------
    ushort UnifiedHighLevelGpuProgram::getNumberOfPosesIncluded(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->getNumberOfPosesIncluded();
        else
            return 0;
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::isVertexTextureFetchRequired(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->isVertexTextureFetchRequired();
        else
            return false;
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr UnifiedHighLevelGpuProgram::getDefaultParameters(void)
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->getDefaultParameters();
        else
            return GpuProgramParametersSharedPtr();
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::hasDefaultParameters(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->hasDefaultParameters();
        else
            return false;
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::getPassSurfaceAndLightStates(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->getPassSurfaceAndLightStates();
        else
            return HighLevelGpuProgram::getPassSurfaceAndLightStates();
    }
    //---------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::getPassFogStates(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->getPassFogStates();
        else
            return HighLevelGpuProgram::getPassFogStates();
    }
    //---------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::getPassTransformStates(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->getPassTransformStates();
        else
            return HighLevelGpuProgram::getPassTransformStates();

    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::hasCompileError(void) const
    {
        if (_getDelegate().isNull())
        {
            return false;
        }
        else
        {
            return _getDelegate()->hasCompileError();
        }
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::resetCompileError(void)
    {
        if (!_getDelegate().isNull())
            _getDelegate()->resetCompileError();
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::load(bool backgroundThread)
    {
        if (!_getDelegate().isNull())
            _getDelegate()->load(backgroundThread);
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::reload(LoadingFlags flags)
    {
        if (!_getDelegate().isNull())
            _getDelegate()->reload(flags);
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::isReloadable(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->isReloadable();
        else
            return true;
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::unload(void)
    {
        if (!_getDelegate().isNull())
            _getDelegate()->unload();
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::isLoaded(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->isLoaded();
        else
            return false;
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::isLoading() const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->isLoading();
        else
            return false;
    }
    //-----------------------------------------------------------------------
    Resource::LoadingState UnifiedHighLevelGpuProgram::getLoadingState() const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->getLoadingState();
        else
            return Resource::LOADSTATE_UNLOADED;
    }
    //-----------------------------------------------------------------------
    size_t UnifiedHighLevelGpuProgram::getSize(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->getSize();
        else
            return 0;
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::touch(void)
    {
        if (!_getDelegate().isNull())
            _getDelegate()->touch();
    }
    //-----------------------------------------------------------------------
    bool UnifiedHighLevelGpuProgram::isBackgroundLoaded(void) const
    {
        if (!_getDelegate().isNull())
            return _getDelegate()->isBackgroundLoaded();
        else
            return false;
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::setBackgroundLoaded(bool bl)
    {
        if (!_getDelegate().isNull())
            _getDelegate()->setBackgroundLoaded(bl);
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::escalateLoading()
    {
        if (!_getDelegate().isNull())
            _getDelegate()->escalateLoading();
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::addListener(Resource::Listener* lis)
    {
        if (!_getDelegate().isNull())
            _getDelegate()->addListener(lis);
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::removeListener(Resource::Listener* lis)
    {
        if (!_getDelegate().isNull())
            _getDelegate()->removeListener(lis);
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::createLowLevelImpl(void)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "This method should never get called!",
            "UnifiedHighLevelGpuProgram::createLowLevelImpl");
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::unloadHighLevelImpl(void)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "This method should never get called!",
            "UnifiedHighLevelGpuProgram::unloadHighLevelImpl");
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::buildConstantDefinitions() const
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "This method should never get called!",
            "UnifiedHighLevelGpuProgram::buildConstantDefinitions");
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::loadFromSource(void)
    {
        OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED, 
            "This method should never get called!",
            "UnifiedHighLevelGpuProgram::loadFromSource");
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String UnifiedHighLevelGpuProgram::CmdDelegate::doGet(const void* target) const
    {
        // Can't do this (not one delegate), shouldn't matter
        return BLANKSTRING;
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgram::CmdDelegate::doSet(void* target, const String& val)
    {
        static_cast<UnifiedHighLevelGpuProgram*>(target)->addDelegateProgram(val);
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    UnifiedHighLevelGpuProgramFactory::UnifiedHighLevelGpuProgramFactory()
    {
    }
    //-----------------------------------------------------------------------
    UnifiedHighLevelGpuProgramFactory::~UnifiedHighLevelGpuProgramFactory()
    {
    }
    //-----------------------------------------------------------------------
    const String& UnifiedHighLevelGpuProgramFactory::getLanguage(void) const
    {
        return sLanguage;
    }
    //-----------------------------------------------------------------------
    HighLevelGpuProgram* UnifiedHighLevelGpuProgramFactory::create(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
    {
        return OGRE_NEW UnifiedHighLevelGpuProgram(creator, name, handle, group, isManual, loader);
    }
    //-----------------------------------------------------------------------
    void UnifiedHighLevelGpuProgramFactory::destroy(HighLevelGpuProgram* prog)
    {
        OGRE_DELETE prog;
    }
    //-----------------------------------------------------------------------

}

