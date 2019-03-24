#include "OgreSGTechniqueResolverListener.h"

#include "OgreTechnique.h"

namespace OgreBites {

SGTechniqueResolverListener::SGTechniqueResolverListener(Ogre::RTShader::ShaderGenerator *pShaderGenerator)
{
    mShaderGenerator = pShaderGenerator;
}

Ogre::Technique *SGTechniqueResolverListener::handleSchemeNotFound(unsigned short schemeIndex, const Ogre::String &schemeName, Ogre::Material *originalMaterial, unsigned short lodIndex, const Ogre::Renderable *rend) {
    if (!mShaderGenerator->hasRenderState(schemeName))
    {
        return NULL;
    }
    // Case this is the default shader generator scheme.

    // Create shader generated technique for this material.
    bool techniqueCreated = mShaderGenerator->createShaderBasedTechnique(
                *originalMaterial,
                Ogre::MaterialManager::DEFAULT_SCHEME_NAME,
                schemeName);

    if (!techniqueCreated)
    {
        return NULL;
    }
    // Case technique registration succeeded.

    // Force creating the shaders for the generated technique.
    mShaderGenerator->validateMaterial(schemeName, originalMaterial->getName(), originalMaterial->getGroup());

    // Grab the generated technique.
    Ogre::Material::Techniques::const_iterator it;
    for(it = originalMaterial->getTechniques().begin(); it != originalMaterial->getTechniques().end(); ++it)
    {
        Ogre::Technique* curTech = *it;

        if (curTech->getSchemeName() == schemeName)
        {
            return curTech;
        }
    }

    return NULL;
}

bool SGTechniqueResolverListener::afterIlluminationPassesCreated(Ogre::Technique *tech)
{
    if(mShaderGenerator->hasRenderState(tech->getSchemeName()))
    {
        Ogre::Material* mat = tech->getParent();
        mShaderGenerator->validateMaterialIlluminationPasses(tech->getSchemeName(),
                                                             mat->getName(), mat->getGroup());
        return true;
    }
    return false;
}

bool SGTechniqueResolverListener::beforeIlluminationPassesCleared(Ogre::Technique *tech)
{
    if(mShaderGenerator->hasRenderState(tech->getSchemeName()))
    {
        Ogre::Material* mat = tech->getParent();
        mShaderGenerator->invalidateMaterialIlluminationPasses(tech->getSchemeName(),
                                                               mat->getName(), mat->getGroup());
        return true;
    }
    return false;
}

}
