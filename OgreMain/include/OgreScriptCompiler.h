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

#ifndef __SCRIPTCOMPILER_H_
#define __SCRIPTCOMPILER_H_

#include "OgreSharedPtr.h"
#include "OgreSingleton.h"
#include "OgreScriptLoader.h"
#include "OgreGpuProgram.h"
#include "OgreAny.h"
#include "Threading/OgreThreadHeaders.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Script
    *  @{
    */

    struct ConcreteNode;
    typedef SharedPtr<ConcreteNode> ConcreteNodePtr;
    typedef std::list<ConcreteNodePtr> ConcreteNodeList;
    typedef SharedPtr<ConcreteNodeList> ConcreteNodeListPtr;

    /** This enum holds the types of the possible abstract nodes */
    enum AbstractNodeType
    {
        ANT_UNKNOWN,
        ANT_ATOM,
        ANT_OBJECT,
        ANT_PROPERTY,
        ANT_IMPORT,
        ANT_VARIABLE_SET,
        ANT_VARIABLE_ACCESS
    };
    class AbstractNode;
    typedef SharedPtr<AbstractNode> AbstractNodePtr;
    typedef std::list<AbstractNodePtr> AbstractNodeList;
    typedef SharedPtr<AbstractNodeList> AbstractNodeListPtr;

    class _OgreExport AbstractNode : public AbstractNodeAlloc
    {
    public:
        String file;
        unsigned int line;
        AbstractNodeType type;
        AbstractNode *parent;
        Any context; // A holder for translation context data
    public:
        AbstractNode(AbstractNode *ptr);
        virtual ~AbstractNode(){}
        /// Returns a new AbstractNode which is a replica of this one.
        virtual AbstractNode *clone() const = 0;
        /// Returns a string value depending on the type of the AbstractNode.
        virtual const String& getValue() const = 0;
        /// Returns the string content of the node for ANT_ATOM. Empty string otherwise.
        const String& getString() const;
    };

    /** This is an abstract node which cannot be broken down further */
    class _OgreExport AtomAbstractNode : public AbstractNode
    {
    public:
        String value;
        uint32 id;
    public:
        AtomAbstractNode(AbstractNode *ptr);
        AbstractNode *clone() const override;
        const String& getValue() const override { return value; }
    };

    inline const String& AbstractNode::getString() const
    {
        return type == ANT_ATOM ? static_cast<const AtomAbstractNode*>(this)->value : BLANKSTRING;
    }

    /** This specific abstract node represents a script object */
    class _OgreExport ObjectAbstractNode : public AbstractNode
    {
    private:
        std::map<String,String> mEnv;
    public:
        String name, cls;
        std::vector<String> bases;
        uint32 id;
        bool abstract;
        AbstractNodeList children;
        AbstractNodeList values;
        AbstractNodeList overrides; // For use when processing object inheritance and overriding
    public:
        ObjectAbstractNode(AbstractNode *ptr);
        AbstractNode *clone() const override;
        const String& getValue() const override { return cls; }

        void addVariable(const String &name);
        void setVariable(const String &name, const String &value);
        std::pair<bool,String> getVariable(const String &name) const;
        const std::map<String,String> &getVariables() const;
    };

    /** This abstract node represents a script property */
    class _OgreExport PropertyAbstractNode : public AbstractNode
    {
    public:
        String name;
        uint32 id;
        AbstractNodeList values;
    public:
        PropertyAbstractNode(AbstractNode *ptr);
        AbstractNode *clone() const override;
        const String& getValue() const override { return name; }
    };

    class ScriptCompilerEvent;
    class ScriptCompilerListener;

    /** This is the main class for the compiler. It calls the parser
        and processes the CST into an AST and then uses translators
        to translate the AST into the final resources.
    */
    class _OgreExport ScriptCompiler : public ScriptCompilerAlloc
    {
        friend class ScriptCompilerManager;
    public: // Externally accessible types
        //typedef std::map<String,uint32> IdMap;
        typedef std::unordered_map<String,uint32> IdMap;

        // These are the built-in error codes
        enum{
            CE_STRINGEXPECTED,
            CE_NUMBEREXPECTED,
            CE_FEWERPARAMETERSEXPECTED,
            CE_VARIABLEEXPECTED,
            CE_UNDEFINEDVARIABLE,
            CE_OBJECTNAMEEXPECTED,
            CE_OBJECTALLOCATIONERROR,
            CE_INVALIDPARAMETERS,
            CE_DUPLICATEOVERRIDE,
            CE_UNEXPECTEDTOKEN,
            CE_OBJECTBASENOTFOUND,
            CE_REFERENCETOANONEXISTINGOBJECT,
            CE_DEPRECATEDSYMBOL
        };
        static String formatErrorCode(uint32 code);
    public:
        ScriptCompiler();
        virtual ~ScriptCompiler() {}

        /// Adds the given error to the compiler's list of errors
        void addError(uint32 code, const String &file, int line, const String &msg = "");
        /// Sets the listener used by the compiler
        void setListener(ScriptCompilerListener *listener);
        /// Returns the currently set listener
        ScriptCompilerListener *getListener();
        /// Returns the resource group currently set for this compiler
        const String &getResourceGroup() const;
        /// Internal method for firing the handleEvent method
        bool _fireEvent(ScriptCompilerEvent *evt, void *retval);

    private: // Tree processing
        /// Takes in a string of script code and compiles it into resources
        /**
         * @param str The script code
         * @param source The source of the script code (e.g. a script file)
         * @param group The resource group to place the compiled resources into
         */
        bool compile(const String &str, const String &source, const String &group);
        /// Compiles resources from the given concrete node list
        bool compile(const ConcreteNodeListPtr &nodes, const String &group);
        /// Adds a custom word id which can be used for custom script translators
		uint32 registerCustomWordId(const String &word);
        AbstractNodeListPtr convertToAST(const ConcreteNodeList &nodes);
        /// This built-in function processes import nodes
        void processImports(AbstractNodeList &nodes);
        /// Loads the requested script and converts it to an AST
        AbstractNodeListPtr loadImportPath(const String &name);
        /// Returns the abstract nodes from the given tree which represent the target
        AbstractNodeList locateTarget(const AbstractNodeList& nodes, const String &target);
        /// Handles object inheritance and variable expansion
        void processObjects(AbstractNodeList& nodes, const AbstractNodeList &top);
        /// Handles processing the variables
        void processVariables(AbstractNodeList& nodes);
        /// This function overlays the given object on the destination object following inheritance rules
        void overlayObject(const ObjectAbstractNode &source, ObjectAbstractNode& dest);
        /// Returns true if the given class is name excluded
        bool isNameExcluded(const ObjectAbstractNode& node, AbstractNode *parent);
        /// This function sets up the initial values in word id map
        void initWordMap();
    private:
        friend String getPropertyName(const ScriptCompiler *compiler, uint32 id);
        // Resource group
        String mGroup;
        // The word -> id conversion table
        IdMap mIds;

		// The largest registered id
		uint32 mLargestRegisteredWordId;

        // This is an environment map
        typedef std::map<String,String> Environment;
        Environment mEnv;

        typedef std::map<String,AbstractNodeListPtr> ImportCacheMap;
        ImportCacheMap mImports; // The set of imported scripts to avoid circular dependencies
        typedef std::multimap<String,String> ImportRequestMap;
        ImportRequestMap mImportRequests; // This holds the target objects for each script to be imported

        // This stores the imports of the scripts, so they are separated and can be treated specially
        AbstractNodeList mImportTable;

        // Error list
        // The container for errors
        struct Error
        {
            String file, message;
            int line;
            uint32 code;
        };
        typedef std::list<Error> ErrorList;
        ErrorList mErrors;

        // The listener
        ScriptCompilerListener *mListener;
    private: // Internal helper classes and processors
        class AbstractTreeBuilder
        {
        private:
            AbstractNodeListPtr mNodes;
            AbstractNode *mCurrent;
            ScriptCompiler *mCompiler;
        public:
            AbstractTreeBuilder(ScriptCompiler *compiler);
            const AbstractNodeListPtr &getResult() const;
            void visit(ConcreteNode *node);
            static void visit(AbstractTreeBuilder *visitor, const ConcreteNodeList &nodes);
        };
        friend class AbstractTreeBuilder;
    };

    /**
     * This struct is a base class for events which can be thrown by the compilers and caught by
     * subscribers. There are a set number of standard events which are used by Ogre's core.
     * New event types may be derived for more custom compiler processing.
     */
    class ScriptCompilerEvent
    {
    public:
        String mType;

        ScriptCompilerEvent(const String &type):mType(type){}
        virtual ~ScriptCompilerEvent(){}
    private: // Non-copyable
        ScriptCompilerEvent(const ScriptCompilerEvent&);
        ScriptCompilerEvent &operator = (const ScriptCompilerEvent&);
    };

    /** This is a listener for the compiler. The compiler can be customized with
        this listener. It lets you listen in on events occurring during compilation,
        hook them, and change the behavior.
    */
    class _OgreExport ScriptCompilerListener
    {
    public:
        ScriptCompilerListener();
        virtual ~ScriptCompilerListener() {}

        /// Returns the concrete node list from the given file
        virtual ConcreteNodeListPtr importFile(ScriptCompiler *compiler, const String &name);
        /// Allows for responding to and overriding behavior before a CST is translated into an AST
        virtual void preConversion(ScriptCompiler *compiler, ConcreteNodeListPtr nodes);
        /// Allows vetoing of continued compilation after the entire AST conversion process finishes
        /**
         @remarks   Once the script is turned completely into an AST, including import
                    and override handling, this function allows a listener to exit
                    the compilation process.
         @return True continues compilation, false aborts
         */
        virtual bool postConversion(ScriptCompiler *compiler, const AbstractNodeListPtr&);
        /// Called when an error occurred
        virtual void handleError(ScriptCompiler *compiler, uint32 code, const String &file, int line, const String &msg);
        /// Called when an event occurs during translation, return true if handled
        /**
         @remarks   This function is called from the translators when an event occurs that
                    that can be responded to. Often this is overriding names, or it can be a request for
                    custom resource creation.
         @arg compiler A reference to the compiler
         @arg evt The event object holding information about the event to be processed
         @arg retval A possible return value from handlers
         @return True if the handler processed the event
        */
        virtual bool handleEvent(ScriptCompiler *compiler, ScriptCompilerEvent *evt, void *retval);
    };

    class ScriptTranslator;
    class ScriptTranslatorManager;

    /** Manages threaded compilation of scripts. This script loader forwards
        scripts compilations to a specific compiler instance.
    */
    class _OgreExport ScriptCompilerManager : public Singleton<ScriptCompilerManager>, public ScriptLoader, public ScriptCompilerAlloc
    {
    private:
            OGRE_AUTO_MUTEX;

        // A list of patterns loaded by this compiler manager
        StringVector mScriptPatterns;

        // Stores a map from object types to the translators that handle them
        std::vector<ScriptTranslatorManager*> mManagers;

        // A pointer to the built-in ScriptTranslatorManager
        ScriptTranslatorManager *mBuiltinTranslatorManager;

        // the specific compiler instance used
        ScriptCompiler mScriptCompiler;
    public:
        ScriptCompilerManager();
        virtual ~ScriptCompilerManager();

        /// Sets the listener used for compiler instances
        void setListener(ScriptCompilerListener *listener);
        /// Returns the currently set listener used for compiler instances
        ScriptCompilerListener *getListener();

        /// Adds the given translator manager to the list of managers
        void addTranslatorManager(ScriptTranslatorManager *man);
        /// Removes the given translator manager from the list of managers
        void removeTranslatorManager(ScriptTranslatorManager *man);
        /// Clears all translator managers
        void clearTranslatorManagers();
        /// Retrieves a ScriptTranslator from the supported managers
        ScriptTranslator *getTranslator(const AbstractNodePtr &node);

		/// Adds a custom word id which can be used for custom script translators
		/** 
		@param
		word The word to be registered.

		@return
		The word id for the registered word.
		
		@note
		If the word is already registered, the already registered id is returned.
		*/
		uint32 registerCustomWordId(const String &word);

        /// Adds a script extension that can be handled (e.g. *.material, *.pu, etc.)
        void addScriptPattern(const String &pattern);
        /// @copydoc ScriptLoader::getScriptPatterns
        const StringVector& getScriptPatterns(void) const override;
        /// @copydoc ScriptLoader::parseScript
        void parseScript(DataStreamPtr& stream, const String& groupName) override;
        /// @copydoc ScriptLoader::getLoadingOrder
        Real getLoadingOrder(void) const override;

        /// @copydoc Singleton::getSingleton()
        static ScriptCompilerManager& getSingleton(void);
        /// @copydoc Singleton::getSingleton()
        static ScriptCompilerManager* getSingletonPtr(void);
    };

    class _OgreExport ProcessResourceNameScriptCompilerEvent : public ScriptCompilerEvent
    {
    public:
        enum ResourceType
        {
            TEXTURE,
            MATERIAL,
            GPU_PROGRAM,
            COMPOSITOR
        };
        ResourceType mResourceType;
        String mName;
        static String eventType;

        ProcessResourceNameScriptCompilerEvent(ResourceType resourceType, const String &name)
            :ScriptCompilerEvent(eventType), mResourceType(resourceType), mName(name){}     
    };

    class _OgreExport ProcessNameExclusionScriptCompilerEvent : public ScriptCompilerEvent
    {
    public:
        String mClass;
        AbstractNode *mParent;
        static String eventType;

        ProcessNameExclusionScriptCompilerEvent(const String &cls, AbstractNode *parent)
            :ScriptCompilerEvent(eventType), mClass(cls), mParent(parent){}     
    };

    class _OgreExport CreateMaterialScriptCompilerEvent : public ScriptCompilerEvent
    {
    public:
        String mFile, mName, mResourceGroup;
        static String eventType;

        CreateMaterialScriptCompilerEvent(const String &file, const String &name, const String &resourceGroup)
            :ScriptCompilerEvent(eventType), mFile(file), mName(name), mResourceGroup(resourceGroup){}  
    };

    class _OgreExport CreateGpuProgramScriptCompilerEvent : public ScriptCompilerEvent
    {
    public:
        String mFile, mName, mResourceGroup, mSource, mSyntax;
        GpuProgramType mProgramType;
        static String eventType;

        CreateGpuProgramScriptCompilerEvent(const String &file, const String &name, const String &resourceGroup, const String &source, 
            const String &syntax, GpuProgramType programType)
            :ScriptCompilerEvent(eventType), mFile(file), mName(name), mResourceGroup(resourceGroup), mSource(source), 
             mSyntax(syntax), mProgramType(programType)
        {}  
    };

    /// @deprecated use CreateGpuProgramScriptCompilerEvent
    typedef OGRE_DEPRECATED CreateGpuProgramScriptCompilerEvent CreateHighLevelGpuProgramScriptCompilerEvent;

    class _OgreExport CreateGpuSharedParametersScriptCompilerEvent : public ScriptCompilerEvent
    {
    public:
        String mFile, mName, mResourceGroup;
        static String eventType;

        CreateGpuSharedParametersScriptCompilerEvent(const String &file, const String &name, const String &resourceGroup)
            :ScriptCompilerEvent(eventType), mFile(file), mName(name), mResourceGroup(resourceGroup){}  
    };

    class _OgreExport CreateParticleSystemScriptCompilerEvent : public ScriptCompilerEvent
    {
    public:
        String mFile, mName, mResourceGroup;
        static String eventType;

        CreateParticleSystemScriptCompilerEvent(const String &file, const String &name, const String &resourceGroup)
            :ScriptCompilerEvent(eventType), mFile(file), mName(name), mResourceGroup(resourceGroup){}  
    };

    class _OgreExport CreateCompositorScriptCompilerEvent : public ScriptCompilerEvent
    {
    public:
        String mFile, mName, mResourceGroup;
        static String eventType;

        CreateCompositorScriptCompilerEvent(const String &file, const String &name, const String &resourceGroup)
            :ScriptCompilerEvent(eventType), mFile(file), mName(name), mResourceGroup(resourceGroup){}  
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
