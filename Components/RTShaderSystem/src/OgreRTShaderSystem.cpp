/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2011 Torus Knot Software Ltd
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
#include "OgreRTShaderSystem.h"

namespace Ogre {

//-----------------------------------------------------------------------
template<> 
RTShader::RTShaderSystem* Singleton<RTShader::RTShaderSystem>::ms_Singleton = 0;

namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/

Ogre::Technique* RTShaderSystem::ShaderGeneratorTechniqueResolverListener::handleSchemeNotFound( unsigned short schemeIndex, const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, const Ogre::Renderable* rend )
{
  Ogre::Technique* generatedTech = NULL;

  // Case this is the default shader generator scheme.
  if (schemeName == ShaderGenerator::DEFAULT_SCHEME_NAME)
  {
    bool techniqueCreated;

    // Create shader generated technique for this material.
    techniqueCreated = mShaderGenerator->createShaderBasedTechnique(
      originalMaterial->getName(), 
      Ogre::MaterialManager::DEFAULT_SCHEME_NAME, 
      schemeName);	

    // Case technique registration succeeded.
    if (techniqueCreated)
    {
      // Force creating the shaders for the generated technique.
      mShaderGenerator->validateMaterial(schemeName, originalMaterial->getName());

      // Grab the generated technique.
      Ogre::Material::TechniqueIterator itTech = originalMaterial->getTechniqueIterator();

      while (itTech.hasMoreElements())
      {
        Ogre::Technique* curTech = itTech.getNext();

        if (curTech->getSchemeName() == schemeName)
        {
          generatedTech = curTech;
          break;
        }
      }				
    }
  }

  return generatedTech;
}

//-----------------------------------------------------------------------
RTShaderSystem::ShaderGeneratorTechniqueResolverListener::ShaderGeneratorTechniqueResolverListener( ShaderGenerator* pShaderGenerator )
{
  mShaderGenerator = pShaderGenerator;
}

//-----------------------------------------------------------------------
RTShaderSystem::RTShaderSystem() 
  : mShaderGenerator(0), mMaterialMgrListener(0)
{

}

//-----------------------------------------------------------------------
RTShaderSystem::~RTShaderSystem()
{

}

//-----------------------------------------------------------------------
size_t RTShaderSystem::getVertexShaderCount() const
{
  return mShaderGenerator->getVertexShaderCount();
}

//-----------------------------------------------------------------------
size_t RTShaderSystem::getFragmentShaderCount() const
{
  return mShaderGenerator->getFragmentShaderCount();
}

//-----------------------------------------------------------------------
bool RTShaderSystem::initializeRTShaderSystem( Ogre::SceneManager* sceneMgr )
{
  if (ShaderGenerator::initialize())
  {
    mShaderGenerator = ShaderGenerator::getSingletonPtr();

    mShaderGenerator->addSceneManager(sceneMgr);

    // Setup core libraries and shader cache path.
    Ogre::StringVector groupVector = Ogre::ResourceGroupManager::getSingleton().getResourceGroups();
    Ogre::StringVector::iterator itGroup = groupVector.begin();
    Ogre::StringVector::iterator itGroupEnd = groupVector.end();
    Ogre::String shaderCoreLibsPath;
    Ogre::String shaderCachePath;

    for (; itGroup != itGroupEnd; ++itGroup)
    {
      Ogre::ResourceGroupManager::LocationList resLocationsList = Ogre::ResourceGroupManager::getSingleton().getResourceLocationList(*itGroup);
      Ogre::ResourceGroupManager::LocationList::iterator it = resLocationsList.begin();
      Ogre::ResourceGroupManager::LocationList::iterator itEnd = resLocationsList.end();
      bool coreLibsFound = false;

      // Try to find the location of the core shader lib functions and use it
      // as shader cache path as well - this will reduce the number of generated files
      // when running from different directories.
      for (; it != itEnd; ++it)
      {
        if ((*it)->archive->getName().find("RTShaderLib") != Ogre::String::npos)
        {
          shaderCoreLibsPath = (*it)->archive->getName() + "/";
          shaderCachePath = shaderCoreLibsPath;
          coreLibsFound = true;
          break;
        }
      }
      // Core libs path found in the current group.
      if (coreLibsFound) 
        break; 
    }

    // Core shader libs not found -> shader generating will fail.
    if (shaderCoreLibsPath.empty())			
      return false;			

#ifdef _RTSS_WRITE_SHADERS_TO_DISK
    // Set shader cache path.
#if OGRE_PLATFORM == OGRE_PLATFORM_IPHONE
    shaderCachePath = Ogre::macCachePath();
#endif
    mShaderGenerator->setShaderCachePath(shaderCachePath);		
#endif
    // Create and register the material manager listener.
    mMaterialMgrListener = new ShaderGeneratorTechniqueResolverListener(mShaderGenerator);				
    Ogre::MaterialManager::getSingleton().addListener(mMaterialMgrListener);
  }

  return true;
}

//-----------------------------------------------------------------------
void RTShaderSystem::finalizeRTShaderSystem()
{
  // Restore default scheme.
  Ogre::MaterialManager::getSingleton().setActiveScheme(Ogre::MaterialManager::DEFAULT_SCHEME_NAME);

  // Unregister the material manager listener.
  if (mMaterialMgrListener != NULL)
  {			
    Ogre::MaterialManager::getSingleton().removeListener(mMaterialMgrListener);
    delete mMaterialMgrListener;
    mMaterialMgrListener = NULL;
  }

  // Finalize RTShader system.
  if (mShaderGenerator != NULL)
  {				
    ShaderGenerator::finalize();
    mShaderGenerator = NULL;
  }
}

//-----------------------------------------------------------------------
RTShaderSystem& RTShaderSystem::getSingleton()
{
	assert( ms_Singleton );  
	return ( *ms_Singleton );
}

//-----------------------------------------------------------------------
RTShaderSystem* RTShaderSystem::getSingletonPtr()
{
	assert( ms_Singleton );  
	return ms_Singleton;
}

}
}

