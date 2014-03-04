//|||||||||||||||||||||||||||||||||||||||||||||||

#ifndef OGRE_DEMO_H
#define OGRE_DEMO_H

//|||||||||||||||||||||||||||||||||||||||||||||||

#include "OgreFramework.h"

//|||||||||||||||||||||||||||||||||||||||||||||||

#ifdef INCLUDE_RTSHADER_SYSTEM
#include "OgreRTShaderSystem.h"

/** This class demonstrates basic usage of the RTShader system.
 It sub class the material manager listener class and when a target scheme callback
 is invoked with the shader generator scheme it tries to create an equivalent shader
 based technique based on the default technique of the given material.
 */
class ShaderGeneratorTechniqueResolverListener : public Ogre::MaterialManager::Listener
{
public:
    
    ShaderGeneratorTechniqueResolverListener(Ogre::RTShader::ShaderGenerator* pShaderGenerator)
    {
        mShaderGenerator = pShaderGenerator;            
    }
    
    /** This is the hook point where shader based technique will be created.
     It will be called whenever the material manager won't find appropriate technique
     that satisfy the target scheme name. If the scheme name is out target RT Shader System
     scheme name we will try to create shader generated technique for it. 
     */
    virtual Ogre::Technique* handleSchemeNotFound(unsigned short schemeIndex, 
                                                  const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
                                                  const Ogre::Renderable* rend)
    {   
        Ogre::Technique* generatedTech = NULL;
        
        // Case this is the default shader generator scheme.
        if (schemeName == Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
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
    
protected:  
    Ogre::RTShader::ShaderGenerator*    mShaderGenerator;           // The shader generator instance.       
};
#endif

class DemoApp : public OIS::KeyListener
{
public:
    DemoApp();
    ~DemoApp();
    
    void startDemo();
    
    bool keyPressed(const OIS::KeyEvent &keyEventRef);
    bool keyReleased(const OIS::KeyEvent &keyEventRef);
    
private:
    void setupDemoScene();
    void runDemo();
    bool initialiseRTShaderSystem(Ogre::SceneManager* sceneMgr);
    void destroyRTShaderSystem();

    Ogre::SceneNode*            m_pCubeNode;
    Ogre::Entity*               m_pCubeEntity;
    
    bool                    m_bShutdown;
#ifdef INCLUDE_RTSHADER_SYSTEM
    Ogre::RTShader::ShaderGenerator*            mShaderGenerator;           // The Shader generator instance.
    ShaderGeneratorTechniqueResolverListener*   mMaterialMgrListener;       // Shader generator material manager listener.  
#endif // INCLUDE_RTSHADER_SYSTEM

};

//|||||||||||||||||||||||||||||||||||||||||||||||

#endif 

//|||||||||||||||||||||||||||||||||||||||||||||||
