/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __SCRIPTCOMPILER_H_
#define __SCRIPTCOMPILER_H_

#include "OgreSharedPtr.h"
#include "OgreMaterial.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreCompositor.h"
#include "OgreCompositionPass.h"
#include "OgreAny.h"

namespace Ogre
{
	/** These enums hold the types of the concrete parsed nodes */
	enum ConcreteNodeType
	{
		CNT_VARIABLE,
		CNT_VARIABLE_ASSIGN,
		CNT_WORD,
		CNT_IMPORT,
		CNT_QUOTE,
		CNT_LBRACE,
		CNT_RBRACE,
		CNT_COLON
	};

	/** The ConcreteNode is the struct that holds an un-conditioned sub-tree of parsed input */
	struct ConcreteNode;
	typedef SharedPtr<ConcreteNode> ConcreteNodePtr;
	typedef list<ConcreteNodePtr>::type ConcreteNodeList;
	typedef SharedPtr<ConcreteNodeList> ConcreteNodeListPtr;
	struct ConcreteNode : public ScriptCompilerAlloc
	{
		String token, file;
		unsigned int line;
		ConcreteNodeType type;
		ConcreteNodeList children;
		ConcreteNode *parent;
	};

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
	typedef list<AbstractNodePtr>::type AbstractNodeList;
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
		virtual String getValue() const = 0;
	};

	/** This is an abstract node which cannot be broken down further */
	class _OgreExport AtomAbstractNode : public AbstractNode
	{
	public:
		String value;
		uint32 id;
	public:
		AtomAbstractNode(AbstractNode *ptr);
		AbstractNode *clone() const;
		String getValue() const;
	private:
		void parseNumber() const;
	};

	/** This specific abstract node represents a script object */
	class _OgreExport ObjectAbstractNode : public AbstractNode
	{
	private:
		map<String,String>::type mEnv;
	public:
		String name, cls, base;
		uint32 id;
		bool abstract;
		AbstractNodeList children;
		AbstractNodeList values;
		AbstractNodeList overrides; // For use when processing object inheritance and overriding
	public:
		ObjectAbstractNode(AbstractNode *ptr);
		AbstractNode *clone() const;
		String getValue() const;

		void addVariable(const String &name);
		void setVariable(const String &name, const String &value);
		std::pair<bool,String> getVariable(const String &name) const;
		const map<String,String>::type &getVariables() const;
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
		AbstractNode *clone() const;
		String getValue() const;
	};

	/** This abstract node represents an import statement */
	class _OgreExport ImportAbstractNode : public AbstractNode
	{
	public:
		String target, source;
	public:
		ImportAbstractNode();
		AbstractNode *clone() const;
		String getValue() const;
	};

	/** This abstract node represents a variable assignment */
	class _OgreExport VariableAccessAbstractNode : public AbstractNode
	{
	public:
		String name;
	public:
		VariableAccessAbstractNode(AbstractNode *ptr);
		AbstractNode *clone() const;
		String getValue() const;
	};

	class ScriptCompilerListener;

	/** This is the main class for the compiler. It calls the parser
		and processes the CST into an AST and then uses translators
		to translate the AST into the final resources.
	*/
	class _OgreExport ScriptCompiler : public ScriptCompilerAlloc
	{
	public: // Externally accessible types
		typedef map<String,uint32>::type IdMap;

		// The container for errors
		struct Error : public ScriptCompilerAlloc
		{
			String file, message;
			int line;
			uint32 code;
		};
		typedef SharedPtr<Error> ErrorPtr;
		typedef list<ErrorPtr>::type ErrorList;

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
			CE_UNSUPPORTEDBYRENDERSYSTEM,
			CE_REFERENCETOANONEXISTINGOBJECT
		};
		static String formatErrorCode(uint32 code);
	public:
		ScriptCompiler();
		virtual ~ScriptCompiler() {}

		/// Takes in a string of script code and compiles it into resources
		/**
		 * @param str The script code
		 * @param source The source of the script code (e.g. a script file)
		 * @param group The resource group to place the compiled resources into
		 */
		bool compile(const String &str, const String &source, const String &group);
		/// Compiles resources from the given concrete node list
		bool compile(const ConcreteNodeListPtr &nodes, const String &group);
		/// Generates the AST from the given string script
		AbstractNodeListPtr _generateAST(const String &str, const String &source, bool doImports = false, bool doObjects = false, bool doVariables = false);
		/// Compiles the given abstract syntax tree
		bool _compile(AbstractNodeListPtr nodes, const String &group, bool doImports = true, bool doObjects = true, bool doVariables = true);
		/// Adds the given error to the compiler's list of errors
		void addError(uint32 code, const String &file, int line, const String &msg = "");
		/// Sets the listener used by the compiler
		void setListener(ScriptCompilerListener *listener);
		/// Returns the currently set listener
		ScriptCompilerListener *getListener();
		/// Returns the resource group currently set for this compiler
		const String &getResourceGroup() const;
		/// Adds a name exclusion to the map
		/**
		 * Name exclusions identify object types which cannot accept
		 * names. This means that excluded types will always have empty names.
		 * All values in the object header are stored as object values.
		 */
		void addNameExclusion(const String &type);
		/// Removes a name exclusion
		void removeNameExclusion(const String &type);
		/// Internal method for firing the handleEvent method
		bool _fireEvent(const String &name, const vector<Any>::type &args, Any *retval);
		/// Internal method for firing the createObject event
		Any _fireCreateObject(const String &type, const vector<Any>::type &args);
	private: // Tree processing
		AbstractNodeListPtr convertToAST(const ConcreteNodeListPtr &nodes);
		/// This built-in function processes import nodes
		void processImports(AbstractNodeListPtr &nodes);
		/// Loads the requested script and converts it to an AST
		AbstractNodeListPtr loadImportPath(const String &name);
		/// Returns the abstract nodes from the given tree which represent the target
		AbstractNodeListPtr locateTarget(AbstractNodeList *nodes, const String &target);
		/// Handles object inheritance and variable expansion
		void processObjects(AbstractNodeList *nodes, const AbstractNodeListPtr &top);
		/// Handles processing the variables
		void processVariables(AbstractNodeList *nodes);
		/// This function overlays the given object on the destination object following inheritance rules
		void overlayObject(const AbstractNodePtr &source, ObjectAbstractNode *dest);
		/// Returns true if the given class is name excluded
		bool isNameExcluded(const String &cls, AbstractNode *parent);
		/// This function sets up the initial values in word id map
		void initWordMap();
	private:
		// Resource group
		String mGroup;
		// The word -> id conversion table
		IdMap mIds;
		// This is an environment map
		typedef map<String,String>::type Environment;
		Environment mEnv;

		typedef map<String,AbstractNodeListPtr>::type ImportCacheMap;
		ImportCacheMap mImports; // The set of imported scripts to avoid circular dependencies
		typedef multimap<String,String>::type ImportRequestMap;
		ImportRequestMap mImportRequests; // This holds the target objects for each script to be imported

		// This stores the imports of the scripts, so they are separated and can be treated specially
		AbstractNodeList mImportTable;

		// Error list
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
	public: // Public translator definitions
		// This enum are built-in word id values
		enum
		{
			ID_ON = 1,
			ID_OFF = 2,
			ID_TRUE = 1,
			ID_FALSE = 2,
			ID_YES = 1,
			ID_NO = 2
		};	
	};

	/** This is a listener for the compiler. The compiler can be customized with
		this listener. It lets you listen in on events occuring during compilation,
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
		 @remarks	Once the script is turned completely into an AST, including import
					and override handling, this function allows a listener to exit
					the compilation process.
		 @return True continues compilation, false aborts
		 */
		virtual bool postConversion(ScriptCompiler *compiler, const AbstractNodeListPtr&);
		/// Called when an error occurred
		virtual void handleError(ScriptCompiler *compiler, uint32 code, const String &file, int line, const String &msg);
		/// Called when an event occurs during translation, return true if handled
		/**
		 @remarks	This function is called from the translators when an event occurs that
					that can be responded to. Often this is overriding names. The support events are:
					preApplyTextureAliases - Allows overriding texture aliases before they are set
						arg1 is a Material*
						arg2 is an AliasTextureNamePairList* holding the aliases
					processTextureNames - Allows overriding referenced textures
						arg1 is a String*
						arg2 is the size of the array
						No return value
					processMaterialName - Allows overriding referenced materials
						arg1 is a single String*
						No return value
					processGpuProgramName - Allows overriding referenced gpu programs
						arg1 is a single String*
						No return value
					processNameExclusion - Allows forcing a name exclusion
						arg1 is a String of the node object's class ("material", "particle_system", etc.)
						arg2 is an AbstractNode* which is the object's parent
						No return value
		 @arg compiler A reference to the compiler
		 @arg name The name of the event
		 @arg args The vector of argument for the event
		 @arg retval A possible return value from handlers
		 @return True if the handler processed the event
		*/
		virtual bool handleEvent(ScriptCompiler *compiler, const String &name, const vector<Ogre::Any>::type &args, Ogre::Any *retval);
		/// Called when a translator requests a concrete object to be created
		/**
		 @remarks	This function is called when a translator needs to create an Ogre object
					during comilation. If a valid object is returned from this function it is
					used instead of a default object created from Ogre's resource managers.
					The object types are:
					Material
						arg1 is a string holding the file where the object is compiled from
						arg2 is the name for the material specified in the script file
						arg3 is the resource group of the compiling script file
					GpuProgram
						arg1 is a string holding the file where the object is compiled from
						arg2 is the name for the material specified in the script file
						arg3 is the resource group of the compiling script file
						arg4 is the specified source file
						arg5 is the GpuProgramType
						arg6 is the program's syntax code
					UnifiedGpuProgram
						arg1 is a string holding the file where the object is compiled from
						arg2 is the name for the material specified in the script file
						arg3 is the resource group of the compiling script file
						arg4 is the GpuProgramType
					HighLevelGpuProgram
						arg1 is a string holding the file where the object is compiled from
						arg2 is the name for the material specified in the script file
						arg3 is the resource group of the compiling script file
						arg4 is the specified language
						arg5 is the GpuProgramType
						arg6 is the specified source file
					ParticleSystem
						arg1 is a string holding the file where the object is compiled from
						arg2 is the name for the material specified in the script file
						arg3 is the resource group of the compiling script file
					Compositor
						arg1 is a string holding the file where the object is compiled from
						arg2 is the name for the material specified in the script file
						arg3 is the resource group of the compiling script file
		 @arg compiler A reference to the compiler
		 @arg type The type of the requested object
		 @arg args Creation arguments for the object
		 @return A reference (pointer) to the created object wrapped in an Any
	    */
		virtual Ogre::Any createObject(ScriptCompiler *compiler, const String &type, const vector<Ogre::Any>::type &args);
	};

	class ScriptTranslator;
	class ScriptTranslatorManager;

	/** Manages threaded compilation of scripts. This script loader forwards
		scripts compilations to a specific compiler instance.
	*/
	class _OgreExport ScriptCompilerManager : public Singleton<ScriptCompilerManager>, public ScriptLoader, public ScriptCompilerAlloc
	{
	private:
		OGRE_AUTO_MUTEX

		// A list of patterns loaded by this compiler manager
		StringVector mScriptPatterns;

		// A pointer to the listener used for compiling scripts
		ScriptCompilerListener *mListener;

		// Stores a map from object types to the translators that handle them
		vector<ScriptTranslatorManager*>::type mManagers;

		// A pointer to the built-in ScriptTranslatorManager
		ScriptTranslatorManager *mBuiltinTranslatorManager;

		// A pointer to the specific compiler instance used
		OGRE_THREAD_POINTER(ScriptCompiler, mScriptCompiler);
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

		/// @copydoc ScriptLoader::getScriptPatterns
        const StringVector& getScriptPatterns(void) const;
        /// @copydoc ScriptLoader::parseScript
        void parseScript(DataStreamPtr& stream, const String& groupName);
        /// @copydoc ScriptLoader::getLoadingOrder
        Real getLoadingOrder(void) const;

		/** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static ScriptCompilerManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static ScriptCompilerManager* getSingletonPtr(void);
	};

	/// This enum defines the integer ids for keywords this compiler handles
	enum
	{
		ID_MATERIAL = 3,
		ID_VERTEX_PROGRAM,
		ID_GEOMETRY_PROGRAM,
		ID_FRAGMENT_PROGRAM,
		ID_TECHNIQUE,
		ID_PASS,
		ID_TEXTURE_UNIT,
		ID_VERTEX_PROGRAM_REF,
		ID_GEOMETRY_PROGRAM_REF,
		ID_FRAGMENT_PROGRAM_REF,
		ID_SHADOW_CASTER_VERTEX_PROGRAM_REF,
		ID_SHADOW_RECEIVER_VERTEX_PROGRAM_REF,
		ID_SHADOW_RECEIVER_FRAGMENT_PROGRAM_REF,
		ID_SHADOW_CASTER_MATERIAL,
		ID_SHADOW_RECEIVER_MATERIAL,
		
        ID_LOD_VALUES,
        ID_LOD_STRATEGY,
		ID_LOD_DISTANCES,
		ID_RECEIVE_SHADOWS,
		ID_TRANSPARENCY_CASTS_SHADOWS,
		ID_SET_TEXTURE_ALIAS,

		ID_SOURCE,
		ID_SYNTAX,
		ID_DEFAULT_PARAMS,
		ID_PARAM_INDEXED,
		ID_PARAM_NAMED,
		ID_PARAM_INDEXED_AUTO,
		ID_PARAM_NAMED_AUTO,

		ID_SCHEME,
		ID_LOD_INDEX,
		ID_GPU_VENDOR_RULE,
		ID_GPU_DEVICE_RULE,
		ID_INCLUDE, 
		ID_EXCLUDE, 

		ID_AMBIENT,
		ID_DIFFUSE,
		ID_SPECULAR,
		ID_EMISSIVE,
			ID_VERTEXCOLOUR,
		ID_SCENE_BLEND,
			ID_COLOUR_BLEND,
			ID_ONE,
			ID_ZERO,
			ID_DEST_COLOUR,
			ID_SRC_COLOUR,
			ID_ONE_MINUS_DEST_COLOUR,
			ID_ONE_MINUS_SRC_COLOUR,
			ID_DEST_ALPHA,
			ID_SRC_ALPHA,
			ID_ONE_MINUS_DEST_ALPHA,
			ID_ONE_MINUS_SRC_ALPHA,
		ID_SEPARATE_SCENE_BLEND,
		ID_SCENE_BLEND_OP,
			ID_REVERSE_SUBTRACT,
			ID_MIN,
			ID_MAX,
		ID_SEPARATE_SCENE_BLEND_OP,
		ID_DEPTH_CHECK,
		ID_DEPTH_WRITE,
		ID_DEPTH_FUNC,
		ID_DEPTH_BIAS,
		ID_ITERATION_DEPTH_BIAS,
			ID_ALWAYS_FAIL,
			ID_ALWAYS_PASS,
			ID_LESS_EQUAL,
			ID_LESS,
			ID_EQUAL,
			ID_NOT_EQUAL,
			ID_GREATER_EQUAL,
			ID_GREATER,
		ID_ALPHA_REJECTION,
		ID_ALPHA_TO_COVERAGE,
		ID_LIGHT_SCISSOR,
		ID_LIGHT_CLIP_PLANES,
		ID_TRANSPARENT_SORTING,
		ID_ILLUMINATION_STAGE,
			ID_DECAL,
		ID_CULL_HARDWARE,
			ID_CLOCKWISE,
			ID_ANTICLOCKWISE,
		ID_CULL_SOFTWARE,
			ID_BACK,
			ID_FRONT,
		ID_NORMALISE_NORMALS,
		ID_LIGHTING,
		ID_SHADING,
			ID_FLAT, 
			ID_GOURAUD,
			ID_PHONG,
		ID_POLYGON_MODE,
			ID_SOLID,
			ID_WIREFRAME,
			ID_POINTS,
		ID_POLYGON_MODE_OVERRIDEABLE,
		ID_FOG_OVERRIDE,
			ID_NONE,
			ID_LINEAR,
			ID_EXP,
			ID_EXP2,
		ID_COLOUR_WRITE,
		ID_MAX_LIGHTS,
		ID_START_LIGHT,
		ID_ITERATION,
			ID_ONCE,
			ID_ONCE_PER_LIGHT,
			ID_PER_LIGHT,
			ID_PER_N_LIGHTS,
			ID_POINT,
			ID_SPOT,
			ID_DIRECTIONAL,
		ID_POINT_SIZE,
		ID_POINT_SPRITES,
		ID_POINT_SIZE_ATTENUATION,
		ID_POINT_SIZE_MIN,
		ID_POINT_SIZE_MAX,

		ID_TEXTURE_ALIAS,
		ID_TEXTURE,
			ID_1D,
			ID_2D,
			ID_3D,
			ID_CUBIC,
			ID_UNLIMITED,
			ID_ALPHA,
			ID_GAMMA,
		ID_ANIM_TEXTURE,
		ID_CUBIC_TEXTURE,
			ID_SEPARATE_UV,
			ID_COMBINED_UVW,
		ID_TEX_COORD_SET,
		ID_TEX_ADDRESS_MODE,
			ID_WRAP,
			ID_CLAMP,
			ID_BORDER,
			ID_MIRROR,
		ID_TEX_BORDER_COLOUR,
		ID_FILTERING,
			ID_BILINEAR,
			ID_TRILINEAR,
			ID_ANISOTROPIC,
		ID_MAX_ANISOTROPY,
		ID_MIPMAP_BIAS,
		ID_COLOUR_OP,
			ID_REPLACE,
			ID_ADD,
			ID_MODULATE,
			ID_ALPHA_BLEND,
		ID_COLOUR_OP_EX,
			ID_SOURCE1,
			ID_SOURCE2,
			ID_MODULATE_X2,
			ID_MODULATE_X4,
			ID_ADD_SIGNED,
			ID_ADD_SMOOTH,
			ID_SUBTRACT,
			ID_BLEND_DIFFUSE_COLOUR,
			ID_BLEND_DIFFUSE_ALPHA,
			ID_BLEND_TEXTURE_ALPHA,
			ID_BLEND_CURRENT_ALPHA,
			ID_BLEND_MANUAL,
			ID_DOT_PRODUCT,
			ID_SRC_CURRENT,
			ID_SRC_TEXTURE,
			ID_SRC_DIFFUSE,
			ID_SRC_SPECULAR,
			ID_SRC_MANUAL,
		ID_COLOUR_OP_MULTIPASS_FALLBACK,
		ID_ALPHA_OP_EX,
		ID_ENV_MAP,
			ID_SPHERICAL,
			ID_PLANAR,
			ID_CUBIC_REFLECTION,
			ID_CUBIC_NORMAL,
		ID_SCROLL,
		ID_SCROLL_ANIM,
		ID_ROTATE,
		ID_ROTATE_ANIM,
		ID_SCALE,
		ID_WAVE_XFORM,
			ID_SCROLL_X,
			ID_SCROLL_Y,
			ID_SCALE_X,
			ID_SCALE_Y,
			ID_SINE,
			ID_TRIANGLE,
			ID_SQUARE,
			ID_SAWTOOTH,
			ID_INVERSE_SAWTOOTH,
		ID_TRANSFORM,
		ID_BINDING_TYPE,
			ID_VERTEX,
			ID_FRAGMENT,
		ID_CONTENT_TYPE,
			ID_NAMED,
			ID_SHADOW,
		ID_TEXTURE_SOURCE,

		ID_PARTICLE_SYSTEM,
		ID_EMITTER,
		ID_AFFECTOR,

		ID_COMPOSITOR,
			ID_TARGET,
			ID_TARGET_OUTPUT,

			ID_INPUT,
				ID_PREVIOUS,
				ID_TARGET_WIDTH,
				ID_TARGET_HEIGHT,
				ID_TARGET_WIDTH_SCALED,
				ID_TARGET_HEIGHT_SCALED,
			ID_SHARED,
			//ID_GAMMA, - already registered for material
			ID_NO_FSAA,
			ID_ONLY_INITIAL,
			ID_VISIBILITY_MASK,
			ID_LOD_BIAS,
			ID_MATERIAL_SCHEME,
			ID_SHADOWS_ENABLED,

			ID_CLEAR,
			ID_STENCIL,
			ID_RENDER_SCENE,
			ID_RENDER_QUAD,
			ID_IDENTIFIER,
			ID_FIRST_RENDER_QUEUE,
			ID_LAST_RENDER_QUEUE,
			ID_QUAD_NORMALS,
				ID_CAMERA_FAR_CORNERS_VIEW_SPACE,
				ID_CAMERA_FAR_CORNERS_WORLD_SPACE,

			ID_BUFFERS,
				ID_COLOUR,
				ID_DEPTH,
			ID_COLOUR_VALUE,
			ID_DEPTH_VALUE,
			ID_STENCIL_VALUE,

			ID_CHECK,
			ID_COMP_FUNC,
			ID_REF_VALUE,
			ID_MASK,
			ID_FAIL_OP,
				ID_KEEP,
				ID_INCREMENT,
				ID_DECREMENT,
				ID_INCREMENT_WRAP,
				ID_DECREMENT_WRAP,
				ID_INVERT,
			ID_DEPTH_FAIL_OP,
			ID_PASS_OP,
			ID_TWO_SIDED,
		ID_END_BUILTIN_IDS
	};
}

#endif
