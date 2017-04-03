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

#ifndef __SCRIPTTRANSLATOR_H_
#define __SCRIPTTRANSLATOR_H_

#include "OgrePrerequisites.h"
#include "Compositor/OgreCompositorCommon.h"
#include "OgreScriptCompiler.h"
#include "OgreBlendMode.h"
#include "OgreHeaderPrefix.h"

namespace Ogre{
	struct IdString;
    class TextureDefinitionBase;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */
    /** This class translates script AST (abstract syntax tree) into
     *  Ogre resources. It defines a common interface for subclasses
     *  which perform the actual translation.
     */

    class _OgreExport ScriptTranslator : public ScriptTranslatorAlloc
    {
    public:
        /**
         * This function translates the given node into Ogre resource(s).
         * @param compiler The compiler invoking this translator
         * @param node The current AST node to be translated
         */
        virtual void translate(ScriptCompiler *compiler, const AbstractNodePtr &node) = 0;
    protected:
        // needs virtual destructor
        virtual ~ScriptTranslator() {}
        /// Retrieves a new translator from the factories and uses it to process the give node
        void processNode(ScriptCompiler *compiler, const AbstractNodePtr &node);

        /// Retrieves the node iterator at the given index
        static AbstractNodeList::const_iterator getNodeAt(const AbstractNodeList &nodes, int index);
        /// Converts the node to a boolean and returns true if successful
        static bool getBoolean(const AbstractNodePtr &node, bool *result);
        /// Converts the node to a string and returns true if successful
        static bool getString(const AbstractNodePtr &node, String *result);
		/// Converts the node to an IdString and returns true if successful
        static bool getIdString(const AbstractNodePtr &node, IdString *result);
        /// Converts the node to a Real and returns true if successful
        static bool getReal(const AbstractNodePtr &node, Real *result);
        /// Converts the node to a float and returns true if successful
        static bool getFloat(const AbstractNodePtr &node, float *result);
        /// Converts the node to a float and returns true if successful
        static bool getDouble(const AbstractNodePtr &node, double *result);
        /// Converts the node to an integer and returns true if successful
        static bool getInt(const AbstractNodePtr &node, int *result); 
        /// Converts the node to an unsigned integer and returns true if successful
        static bool getUInt(const AbstractNodePtr &node, uint32 *result); 
		/// Converts the node to a an integer (from hexadecimal) and returns true if successful
        static bool getHex(const AbstractNodePtr &node, uint32 *result);
        /// Converts the range of nodes to a ColourValue and returns true if successful
        static bool getColour(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, ColourValue *result, int maxEntries = 4);
        /// Converts the node to a SceneBlendFactor enum and returns true if successful
        static bool getSceneBlendFactor(const AbstractNodePtr &node, SceneBlendFactor *sbf);
        /// Converts the node to a CompareFunction enum and returns true if successful
        static bool getCompareFunction(const AbstractNodePtr &node, CompareFunction *func);
        /// Converts the range of nodes to a Matrix4 and returns true if successful
        static bool getMatrix4(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, Matrix4 *m);
        /// Converts the range of nodes to an array of ints and returns true if successful
        static bool getInts(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, int *vals, int count);
        /// Converts the range of nodes to an array of floats and returns true if successful
        static bool getFloats(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, float *vals, int count);
        /// Converts the range of nodes to an array of floats and returns true if successful
        static bool getDoubles(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, double *vals, int count);
        /// Converts the range of nodes to an array of floats and returns true if successful
        static bool getUInts(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, uint *vals, int count);
        /// Converts the range of nodes to an array of uint-stored boolean values and returns true if successful
        static bool getBooleans(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, uint *vals, int count);
        /// Converts the node to a StencilOperation enum and returns true if successful
        static bool getStencilOp(const AbstractNodePtr &node, StencilOperation *op); 
        /// Converts the node to a GpuConstantType enum and returns true if successful
        static bool getConstantType(AbstractNodeList::const_iterator i, GpuConstantType *op); 

    };

    /** The ScriptTranslatorManager manages the lifetime and access to
     *  script translators. You register these managers with the
     *  ScriptCompilerManager tied to specific object types.
     *  Each manager may manage multiple types.
     */
    class ScriptTranslatorManager : public ScriptTranslatorAlloc
    {
    public:
        // required - virtual destructor
        virtual ~ScriptTranslatorManager() {}

        /// Returns the number of translators being managed
        virtual size_t getNumTranslators() const = 0;
        /// Returns a manager for the given object abstract node, or null if it is not supported
        virtual ScriptTranslator *getTranslator(const AbstractNodePtr&) = 0;
    };

    /**************************************************************************
     * HLMS compilation section
     *************************************************************************/
    class _OgreExport HlmsTranslator : public ScriptTranslator
    {
    public:
        //HlmsTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };

    /**************************************************************************
     * Material compilation section
     *************************************************************************/
    class _OgreExport MaterialTranslator : public ScriptTranslator
    {
    protected:
        Material *mMaterial;
        Ogre::AliasTextureNamePairList mTextureAliases;
    public:
        MaterialTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
        
    class _OgreExport TechniqueTranslator : public ScriptTranslator
    {
    protected:
        Technique *mTechnique;
    public:
        TechniqueTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    
    class _OgreExport PassTranslator : public ScriptTranslator
    {
    protected:
        Pass *mPass;
    public:
        PassTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    protected:
        void translateVertexProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node);
        void translateGeometryProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node);
        void translateFragmentProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node);
        void translateTessellationHullProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node);
        void translateTessellationDomainProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node);
        void translateComputeProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node);
        void translateShadowCasterVertexProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node);
        void translateShadowCasterFragmentProgramRef(ScriptCompiler *compiler, ObjectAbstractNode *node);
    };

    class _OgreExport TextureUnitTranslator : public ScriptTranslator
    {
    protected:
        TextureUnitState *mUnit;
    public:
        TextureUnitTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };

    class _OgreExport TextureSourceTranslator : public ScriptTranslator
    {
    public:
        TextureSourceTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };

    class _OgreExport GpuProgramTranslator : public ScriptTranslator
    {   
    public:
        GpuProgramTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    protected:
        void translateGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj);
        void translateHighLevelGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj);
        void translateUnifiedGpuProgram(ScriptCompiler *compiler, ObjectAbstractNode *obj);
    public:
        static void translateProgramParameters(ScriptCompiler *compiler, GpuProgramParametersSharedPtr params, ObjectAbstractNode *obj);
    };

    class _OgreExport SharedParamsTranslator : public ScriptTranslator
    {   
    public:
        SharedParamsTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
        template <class T, BaseConstantType baseType>
        void translateSharedParamNamed(ScriptCompiler *compiler, GpuSharedParameters *sharedParams, PropertyAbstractNode *prop, String pName, GpuConstantType constType);
        template <class T, BaseConstantType baseType>
        T parseParameter(const String& param);
    protected:
    };

    /**************************************************************************
     * Particle System section
     *************************************************************************/
    class _OgreExport ParticleSystemTranslator : public ScriptTranslator
    {
    protected:
        Ogre::ParticleSystem *mSystem;
    public:
        ParticleSystemTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class _OgreExport ParticleEmitterTranslator : public ScriptTranslator
    {
    protected:
        Ogre::ParticleEmitter *mEmitter;
    public:
        ParticleEmitterTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class _OgreExport ParticleAffectorTranslator : public ScriptTranslator
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
    class _OgreExport CompositorTextureBaseTranslator : public ScriptTranslator
    {
    protected:
        void translateTextureProperty( TextureDefinitionBase *defBase, PropertyAbstractNode *prop,
                                        ScriptCompiler *compiler ) const;
        void translateBufferProperty( TextureDefinitionBase *defBase, PropertyAbstractNode *prop,
                                      ScriptCompiler *compiler ) const;
    };
    class _OgreExport CompositorWorkspaceTranslator : public CompositorTextureBaseTranslator
    {
    protected:
        CompositorWorkspaceDef *mWorkspaceDef;
    public:
        CompositorWorkspaceTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class _OgreExport CompositorNodeTranslator : public CompositorTextureBaseTranslator
    {
    protected:
        CompositorNodeDef *mNodeDef;
    public:
        CompositorNodeTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class _OgreExport CompositorShadowNodeTranslator : public CompositorTextureBaseTranslator
    {
    protected:
        CompositorShadowNodeDef *mShadowNodeDef;
        void translateShadowMapProperty( PropertyAbstractNode *prop, ScriptCompiler *compiler,
                                         const ShadowTextureDefinition &defaultParams ) const;
    public:
        CompositorShadowNodeTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class _OgreExport CompositorTargetTranslator : public ScriptTranslator
    {
    protected:
        CompositorTargetDef *mTargetDef;
    public:
        CompositorTargetTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class _OgreExport CompositorShadowMapTargetTypeTranslator : public ScriptTranslator
    {
    public:
        CompositorShadowMapTargetTypeTranslator();
        static size_t calculateNumTargets( const AbstractNodePtr &node );
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class _OgreExport CompositorShadowMapRepeatTranslator : public ScriptTranslator
    {
    public:
        CompositorShadowMapRepeatTranslator();
        static size_t calculateNumTargets( const AbstractNodePtr &node );
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class _OgreExport CompositorShadowMapTargetTranslator : public ScriptTranslator
    {
    protected:
        CompositorTargetDef *mTargetDef;
    public:
        CompositorShadowMapTargetTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };
    class _OgreExport CompositorPassTranslator : public ScriptTranslator
    {
    protected:
        CompositorPassDef *mPassDef;

        void translateClear( ScriptCompiler *compiler, const AbstractNodePtr &node,
                             CompositorTargetDef *targetDef );
        void translateQuad( ScriptCompiler *compiler, const AbstractNodePtr &node,
                            CompositorTargetDef *targetDef );
        void translateDepthCopy( ScriptCompiler *compiler, const AbstractNodePtr &node,
                                 CompositorTargetDef *targetDef );
        void translateScene( ScriptCompiler *compiler, const AbstractNodePtr &node,
                             CompositorTargetDef *targetDef );
        void translateStencil( ScriptCompiler *compiler, const AbstractNodePtr &node,
                               CompositorTargetDef *targetDef );
        void translateStencilFace( ScriptCompiler *compiler, const AbstractNodePtr &node,
                                   StencilStateOp *stencilStateOp );
        void translateUav( ScriptCompiler *compiler, const AbstractNodePtr &node,
                           CompositorTargetDef *targetDef );
        void translateCompute( ScriptCompiler *compiler, const AbstractNodePtr &node,
                               CompositorTargetDef *targetDef );
        void translateMipmap( ScriptCompiler *compiler, const AbstractNodePtr &node,
                              CompositorTargetDef *targetDef );

    public:
        CompositorPassTranslator();
        void translate(ScriptCompiler *compiler, const AbstractNodePtr &node);
    };

    /**************************************************************************
     * BuiltinScriptTranslatorManager
     *************************************************************************/
    /// This class manages the builtin translators
    class _OgreExport BuiltinScriptTranslatorManager : public ScriptTranslatorManager
    {
    private:
        HlmsTranslator      mHlmsTranslator;
        MaterialTranslator mMaterialTranslator;
        TechniqueTranslator mTechniqueTranslator;
        PassTranslator mPassTranslator;
        TextureUnitTranslator mTextureUnitTranslator;
        TextureSourceTranslator mTextureSourceTranslator;
        GpuProgramTranslator mGpuProgramTranslator;
        SharedParamsTranslator mSharedParamsTranslator;
        ParticleSystemTranslator mParticleSystemTranslator;
        ParticleEmitterTranslator mParticleEmitterTranslator;
        ParticleAffectorTranslator mParticleAffectorTranslator;
        CompositorWorkspaceTranslator mCompositorWorkspaceTranslator;
        CompositorNodeTranslator mCompositorNodeTranslator;
        CompositorShadowNodeTranslator mCompositorShadowNodeTranslator;
        CompositorTargetTranslator mCompositorTargetTranslator;
        CompositorShadowMapTargetTypeTranslator mCompositorShadowMapTargetTypeTranslator;
        CompositorShadowMapRepeatTranslator mCompositorShadowMapRepeatTranslator;
        CompositorShadowMapTargetTranslator mCompositorShadowMapTargetTranslator;
        CompositorPassTranslator mCompositorPassTranslator;
    public:
        BuiltinScriptTranslatorManager();
        /// Returns the number of translators being managed
        virtual size_t getNumTranslators() const;
        /// Returns a manager for the given object abstract node, or null if it is not supported
        virtual ScriptTranslator *getTranslator(const AbstractNodePtr &node);
    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif

