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
#include "OgreScriptCompiler.h"
#include "OgreRenderSystem.h"
#include "OgreHeaderPrefix.h"

namespace Ogre{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Script
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
        static void processNode(ScriptCompiler *compiler, const AbstractNodePtr &node);

        /// Retrieves the node iterator at the given index
        static AbstractNodeList::const_iterator getNodeAt(const AbstractNodeList &nodes, size_t index);
        /// Converts the node to a boolean and returns true if successful
        static bool getBoolean(const AbstractNodePtr &node, bool *result);
        /// Converts the node to a string and returns true if successful
        static bool getString(const AbstractNodePtr &node, String *result);
        /// Converts the node to a Real and returns true if successful
        static bool getReal(const AbstractNodePtr& node, Real* result)
        {
#if OGRE_DOUBLE_PRECISION == 0
            return getFloat(node, result);
#else
            return getDouble(node, result);
#endif
        }
        /// Converts the node to a float and returns true if successful
        static bool getFloat(const AbstractNodePtr &node, float *result);
        /// Converts the node to a float and returns true if successful
        static bool getDouble(const AbstractNodePtr &node, double *result);
        /// Converts the node to an integer and returns true if successful
        static bool getInt(const AbstractNodePtr &node, int *result); 
        /// Converts the node to an unsigned integer and returns true if successful
        static bool getUInt(const AbstractNodePtr &node, uint32 *result); 
        /// Converts the range of nodes to a ColourValue and returns true if successful
        static bool getColour(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, ColourValue *result, int maxEntries = 4);
        /// Converts the node to a SceneBlendFactor enum and returns true if successful
        static bool getSceneBlendFactor(const AbstractNodePtr &node, SceneBlendFactor *sbf);
        /// Converts the node to a CompareFunction enum and returns true if successful
        static bool getCompareFunction(const AbstractNodePtr &node, CompareFunction *func);
        /// Converts the range of nodes to a Matrix4 and returns true if successful
        static bool getMatrix4(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, Matrix4 *m);
        /// @deprecated use getVector
        OGRE_DEPRECATED static bool getInts(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, int *vals, int count);
        /// @deprecated use getVector
        OGRE_DEPRECATED static bool getFloats(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, float *vals, int count);
        /// @deprecated
        OGRE_DEPRECATED static bool getDoubles(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, double *vals, int count);
        /// @deprecated
        OGRE_DEPRECATED static bool getUInts(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, uint *vals, int count);
        /// @deprecated
        OGRE_DEPRECATED static bool getBooleans(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, uint *vals, int count);
        /// read count values from the AbstractNodeList into vals. Fill with default value if AbstractNodeList is shorter then count
        static bool getVector(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, std::vector<int>& vals, size_t count);
        /// @overload
        static bool getVector(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, std::vector<float>& vals, size_t count);
        /// Converts the node to a StencilOperation enum and returns true if successful
        static bool getStencilOp(const AbstractNodePtr &node, StencilOperation *op); 
        /// Converts the node to a GpuConstantType enum and returns true if successful
        static bool getConstantType(AbstractNodeList::const_iterator i, GpuConstantType *op); 

        template<typename T>
        friend bool getValue(const AbstractNodePtr &node, T& result);
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

        /// Returns a manager for the given object abstract node, or null if it is not supported
        virtual ScriptTranslator *getTranslator(const AbstractNodePtr&) = 0;
    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif

