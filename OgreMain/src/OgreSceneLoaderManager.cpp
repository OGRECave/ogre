#include "OgreSceneLoaderManager.h"
#include "OgreException.h"
#include "OgreStringVector.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"

namespace Ogre {

template<> SceneLoaderManager* Singleton<SceneLoaderManager>::msSingleton = 0;

SceneLoaderManager *SceneLoaderManager::getSingletonPtr(void)
{
    return msSingleton;
}

SceneLoaderManager& SceneLoaderManager::getSingleton(void)
{
    assert(msSingleton);
    return *msSingleton;
}

SceneLoaderManager::SceneLoaderManager()
{

}

SceneLoaderManager::~SceneLoaderManager()
{

}

SceneLoaderManager::SceneLoaderInfo::SceneLoaderInfo(SceneLoader *l, const StringVector& ext)
{
    loader = l;
    supportedExt = ext;
}

void SceneLoaderManager::registerSceneLoader(const String& name, const StringVector& ext, SceneLoader *sl)
{
    LogManager::getSingleton().logMessage("Registering SceneLoader " + name);
    if(mSceneLoaders.find(name) != mSceneLoaders.end())
        OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
            String("a SceneLoader with the name \"") +
            name + String("\" already exists"),
            "SceneLoaderManager::registerSceneLoader");
    
    unsigned int shadow = 0;
    for(StringVector::const_iterator s = ext.begin(); s != ext.end(); ++s)
    {
        bool skipCurrent = false;
        for(SceneLoaderMap::iterator l = mSceneLoaders.begin();
            l != mSceneLoaders.end() && !skipCurrent; ++l)
        {
            StringVector ep = l->second.supportedExt;
            for(StringVector::const_iterator e = ep.begin(); e != ep.end(); ++e)
            {
                if((*s).compare(*e) == 0)
                {
                    LogManager::getSingleton().logWarning("support for file extension \"" +
                        (*s) + "\" has already been registered by the SceneLoader " +
                        l->first + ". This extension will be shadowed");
                    ++shadow;
                    skipCurrent = true;
                    break;
                }
            }
        }
    }
    
    if(shadow)
        LogManager::getSingleton().logWarning("registering with " + 
            StringConverter::toString(shadow) + " shadowed extension(s) out of " + 
            StringConverter::toString(ext.size()));
        
        
    
    mSceneLoaders.insert(std::pair<String, SceneLoaderInfo>(name, SceneLoaderInfo(sl, ext)));
}

void SceneLoaderManager::unregisterSceneLoader(const String& name)
{
    mSceneLoaders.erase(name);
}

void SceneLoaderManager::load(const String& filename, const String& groupName, SceneNode *rootNode)
{
    DataStreamPtr stream(Root::openFileStream(filename, groupName));
    load(stream, groupName, rootNode);
}

void SceneLoaderManager::load(DataStreamPtr& stream, const String& groupName, SceneNode *rootNode)
{
    String ext;
    String filename = stream -> getName();
    for(unsigned int i = filename.length() - 2; i > 0; --i)
    {
        if(filename[i] == '.')
        {
            ext = filename.substr(i, filename.length() - i);
            break;
        }
    }
    
    if (ext.empty())
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
            String("could not get file extension from filename \"") + 
            filename + String("\""), "SceneLoaderManager::load");
    
    StringUtil::toLowerCase(ext);

    for(SceneLoaderMap::iterator i = mSceneLoaders.begin();
        i != mSceneLoaders.end(); ++i)
    {
        StringVector ep = i->second.supportedExt;
        for(StringVector::const_iterator s = ep.begin(); s != ep.end(); ++s)
        {
            if(!ext.compare(*s))
            {
                i->second.loader->load (stream, groupName, rootNode);
                return;
            }
        }
    }
    
    OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
        String("no SceneLoader for extension \"") +
        ext + String("\" was found"),
        "SceneLoaderManager::load");
}

}
