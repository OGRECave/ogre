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

#ifndef SAMPLES_BROWSER_INCLUDE_SHADERGENERATORTECHNIQUERESOLVERLISTENER_H_
#define SAMPLES_BROWSER_INCLUDE_SHADERGENERATORTECHNIQUERESOLVERLISTENER_H_

#include "OgreRTShaderSystem.h"

namespace OgreBites
{
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
                                                      const Ogre::String& schemeName,
                                                      Ogre::Material* originalMaterial,
                                                      unsigned short lodIndex,
                                                      const Ogre::Renderable* rend) {
            if (schemeName != Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
            {
                return NULL;
            }
            // Case this is the default shader generator scheme.

            // Create shader generated technique for this material.
            bool techniqueCreated = mShaderGenerator->createShaderBasedTechnique(
                originalMaterial->getName(),
                Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
                schemeName);

            if (!techniqueCreated)
            {
                return NULL;
            }
            // Case technique registration succeeded.

            // Force creating the shaders for the generated technique.
            mShaderGenerator->validateMaterial(schemeName, originalMaterial->getName());

            // Grab the generated technique.
            Ogre::Material::TechniqueIterator itTech = originalMaterial->getTechniqueIterator();

            while (itTech.hasMoreElements())
            {
                Ogre::Technique* curTech = itTech.getNext();

                if (curTech->getSchemeName() == schemeName)
                {
                    return curTech;
                }
            }

            return NULL;
        }

    virtual bool afterIlluminationPassesCreated(Ogre::Technique* tech)
    {
        if(tech->getSchemeName() == Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
        {
            Ogre::Material* mat = tech->getParent();
            mShaderGenerator->validateMaterialIlluminationPasses(tech->getSchemeName(),
                                                                 mat->getName(), mat->getGroup());
            return true;
        }
        return false;
    }

    virtual bool beforeIlluminationPassesCleared(Ogre::Technique* tech)
    {
        if(tech->getSchemeName() == Ogre::RTShader::ShaderGenerator::DEFAULT_SCHEME_NAME)
        {
            Ogre::Material* mat = tech->getParent();
            mShaderGenerator->invalidateMaterialIlluminationPasses(tech->getSchemeName(),
                                                                   mat->getName(), mat->getGroup());
            return true;
        }
        return false;
    }

    protected:
        Ogre::RTShader::ShaderGenerator*        mShaderGenerator;                       // The shader generator instance.
    };
}

#endif /* SAMPLES_BROWSER_INCLUDE_SHADERGENERATORTECHNIQUERESOLVERLISTENER_H_ */
