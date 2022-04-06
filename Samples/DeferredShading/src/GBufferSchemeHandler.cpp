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

#include "GBufferSchemeHandler.h"

#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgreRTShaderSystem.h"

using namespace Ogre;

//! [schemenotfound]
Technique* GBufferSchemeHandler::handleSchemeNotFound(unsigned short schemeIndex, 
        const String& schemeName, Material* originalMaterial, unsigned short lodIndex, 
        const Renderable* rend)
{
    Ogre::MaterialManager& matMgr = Ogre::MaterialManager::getSingleton();
    String curSchemeName = matMgr.getActiveScheme();
    matMgr.setActiveScheme(MSN_DEFAULT);
    Technique* originalTechnique = originalMaterial->getBestTechnique(lodIndex, rend);
    matMgr.setActiveScheme(curSchemeName);

    RTShader::ShaderGenerator& rtShaderGen = RTShader::ShaderGenerator::getSingleton();
    rtShaderGen.createShaderBasedTechnique(originalTechnique, "NoGBuffer");
    rtShaderGen.createShaderBasedTechnique(originalTechnique, "GBuffer");

    for (unsigned short i=0; i<originalTechnique->getNumPasses(); i++)
    {
        Pass* originalPass = originalTechnique->getPass(i);

        //Check transparency
        if (originalPass->getDestBlendFactor() != Ogre::SBF_ZERO)
        {
            rtShaderGen.validateMaterial("NoGBuffer", *originalMaterial);
            continue;
        }

        rtShaderGen.validateMaterial("GBuffer", *originalMaterial);
        // Grab the generated technique.
        for(Technique* curTech : originalMaterial->getTechniques())
        {
            if (curTech->getSchemeName() == schemeName)
            {
                return curTech;
            }
        }
    }
    
    return NULL;
}
//! [schemenotfound]