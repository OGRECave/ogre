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

#ifndef __BUILTIN_SCRIPTTRANSLATORS_H_
#define __BUILTIN_SCRIPTTRANSLATORS_H_

#include "OgreScriptTranslator.h"

namespace Ogre{
    /**************************************************************************
     * Material compilation section
     *************************************************************************/
    class MaterialTranslator : public ScriptTranslator
    {
    protected:
        Material *mMaterial;
        Ogre::AliasTextureNamePairList mTextureAliases;
    public:
        MaterialTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
        
    class TechniqueTranslator : public ScriptTranslator
    {
    protected:
        Technique *mTechnique;
    public:
        TechniqueTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    
    class PassTranslator : public ScriptTranslator
    {
    protected:
        Pass *mPass;
    public:
        PassTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    protected:
        void translateProgramRef(GpuProgramType type, ScriptCompiler *compiler, ObjectAbstractNode *node);
        void translateShadowCasterProgramRef(GpuProgramType type, ScriptCompiler *compiler, ObjectAbstractNode *node);
        void translateShadowReceiverProgramRef(GpuProgramType type, ScriptCompiler *compiler, ObjectAbstractNode *node);
    };

    class TextureUnitTranslator : public ScriptTranslator
    {
    protected:
        TextureUnitState *mUnit;
    public:
        TextureUnitTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };

    struct SamplerTranslator : public ScriptTranslator
    {
        static void translateSamplerParam(ScriptCompiler *compiler, const SamplerPtr& sampler, PropertyAbstractNode* node);
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };

    class TextureSourceTranslator : public ScriptTranslator
    {
    public:
        TextureSourceTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };

    class GpuProgramTranslator : public ScriptTranslator
    {   
    public:
        GpuProgramTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    protected:
        void translateGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj, String language);
    public:
        static void translateProgramParameters(ScriptCompiler *compiler, GpuProgramParametersSharedPtr params, ObjectAbstractNode *obj);
    };

    class SharedParamsTranslator : public ScriptTranslator
    {   
    public:
        SharedParamsTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    protected:
    };

    /**************************************************************************
     * Particle System section
     *************************************************************************/
    class ParticleSystemTranslator : public ScriptTranslator
    {
    protected:
        Ogre::ParticleSystem *mSystem;
    public:
        ParticleSystemTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class ParticleEmitterTranslator : public ScriptTranslator
    {
    protected:
        Ogre::ParticleEmitter *mEmitter;
    public:
        ParticleEmitterTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class ParticleAffectorTranslator : public ScriptTranslator
    {
    protected:
        Ogre::ParticleAffector *mAffector;
    public:
        ParticleAffectorTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };

    /**************************************************************************
     * Compositor section
     *************************************************************************/
    class CompositorTranslator : public ScriptTranslator
    {
    protected:
        Compositor *mCompositor;
    public:
        CompositorTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class CompositionTechniqueTranslator : public ScriptTranslator
    {
    protected:
        CompositionTechnique *mTechnique;
    public:
        CompositionTechniqueTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class CompositionTargetPassTranslator : public ScriptTranslator
    {
    protected:
        CompositionTargetPass *mTarget;
    public:
        CompositionTargetPassTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class CompositionPassTranslator : public ScriptTranslator
    {
    protected:
        CompositionPass *mPass;
    public:
        CompositionPassTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    /**************************************************************************
     * BuiltinScriptTranslatorManager
     *************************************************************************/
    /// This class manages the builtin translators
    class BuiltinScriptTranslatorManager : public ScriptTranslatorManager
    {
    private:
        MaterialTranslator mMaterialTranslator;
        TechniqueTranslator mTechniqueTranslator;
        PassTranslator mPassTranslator;
        TextureUnitTranslator mTextureUnitTranslator;
        SamplerTranslator mSamplerTranslator;
        TextureSourceTranslator mTextureSourceTranslator;
        GpuProgramTranslator mGpuProgramTranslator;
        SharedParamsTranslator mSharedParamsTranslator;
        ParticleSystemTranslator mParticleSystemTranslator;
        ParticleEmitterTranslator mParticleEmitterTranslator;
        ParticleAffectorTranslator mParticleAffectorTranslator;
        CompositorTranslator mCompositorTranslator;
        CompositionTechniqueTranslator mCompositionTechniqueTranslator;
        CompositionTargetPassTranslator mCompositionTargetPassTranslator;
        CompositionPassTranslator mCompositionPassTranslator;
    public:
        BuiltinScriptTranslatorManager();
        /// Returns a manager for the given object abstract node, or null if it is not supported
        virtual ScriptTranslator *getTranslator(const AbstractNodePtr &node);
    };
}

#endif

