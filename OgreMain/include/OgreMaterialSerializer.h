/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __MaterialSerializer_H__
#define __MaterialSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreMaterial.h"
#include "OgreBlendMode.h"
#include "OgreTextureUnitState.h"
#include "OgreGpuProgram.h"
#include "OgreStringVector.h"

namespace Ogre {

	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Materials
	*  @{
	*/
	/** Enum to identify material sections. */
    enum MaterialScriptSection
    {
        MSS_NONE,
        MSS_MATERIAL,
        MSS_TECHNIQUE,
        MSS_PASS,
        MSS_TEXTUREUNIT,
        MSS_PROGRAM_REF,
		MSS_PROGRAM,
        MSS_DEFAULT_PARAMETERS,
		MSS_TEXTURESOURCE
    };
	/** Struct for holding a program definition which is in progress. */
	struct MaterialScriptProgramDefinition
	{
		String name;
		GpuProgramType progType;
        String language;
		String source;
		String syntax;
        bool supportsSkeletalAnimation;
		bool supportsMorphAnimation;
		ushort supportsPoseAnimation; // number of simultaneous poses supported
		bool usesVertexTextureFetch;
		vector<std::pair<String, String> >::type customParameters;
	};
    /** Struct for holding the script context while parsing. */
    struct MaterialScriptContext 
    {
        MaterialScriptSection section;
		String groupName;
        MaterialPtr material;
        Technique* technique;
        Pass* pass;
        TextureUnitState* textureUnit;
        GpuProgramPtr program; // used when referencing a program, not when defining it
        bool isProgramShadowCaster; // when referencing, are we in context of shadow caster
        bool isVertexProgramShadowReceiver; // when referencing, are we in context of shadow caster
		bool isFragmentProgramShadowReceiver; // when referencing, are we in context of shadow caster
        GpuProgramParametersSharedPtr programParams;
		ushort numAnimationParametrics;
		MaterialScriptProgramDefinition* programDef; // this is used while defining a program

		int techLev,	//Keep track of what tech, pass, and state level we are in
			passLev,
			stateLev;
        StringVector defaultParamLines;

		// Error reporting state
        size_t lineNo;
        String filename;
        AliasTextureNamePairList textureAliases;
    };
    /// Function def for material attribute parser; return value determines if the next line should be {
    typedef bool (*ATTRIBUTE_PARSER)(String& params, MaterialScriptContext& context);

    /** Class for serializing Materials to / from a .material script.*/
	class _OgreExport MaterialSerializer : public SerializerAlloc
    {	
	public:

		// Material serizliae event.
		enum SerializeEvent
		{
			MSE_PRE_WRITE,
			MSE_WRITE_BEGIN,
			MSE_WRITE_END,
			MSE_POST_WRITE,
		};

		/** Class that allows listening in on the various stages of material serialization process.
		Sub-classing it enable extending the attribute set of any part in the material.
		*/
		class Listener
		{
		public:
			virtual ~Listener() {}
			
			/** Called when material section event raised.					
			@param ser The MaterialSerializer instance that writes the given material.
			@param stage The current section writing stage.
			@param skip May set to true by sub-class instances in order to skip the following section write.
			This parameter relevant only when stage equals MSE_PRE_WRITE. 
			@param mat The material that is being written.			
			*/
			virtual void materialEventRaised(MaterialSerializer* ser, 
				SerializeEvent event, bool& skip, const Material* mat) {}
			
			/** Called when technique section event raised.				
			@param ser The MaterialSerializer instance that writes the given material.
			@param stage The current section writing stage.
			@param skip May set to true by sub-class instances in order to skip the following section write.
			This parameter relevant only when stage equals MSE_PRE_WRITE. 
			@param tech The technique that is being written.		
			*/
			virtual void techniqueEventRaised(MaterialSerializer* ser, 
				SerializeEvent event, bool& skip, const Technique* tech) {}
		
			/** Called when pass section event raised.					
			@param ser The MaterialSerializer instance that writes the given material.
			@param stage The current section writing stage.
			@param skip May set to true by sub-class instances in order to skip the following section write.
			This parameter relevant only when stage equals MSE_PRE_WRITE. 
			@param pass The pass that is being written.		
			*/
			virtual void passEventRaised(MaterialSerializer* ser, 
				SerializeEvent event, bool& skip, const Pass* pass) {}

			/** Called when GPU program reference section event raised.				
			@param ser The MaterialSerializer instance that writes the given material.
			@param stage The current section writing stage.
			@param skip May set to true by sub-class instances in order to skip the following section write.
			This parameter relevant only when stage equals MSE_PRE_WRITE. 
			@param attrib The GPU program reference description (vertex_program_ref, fragment_program_ref, etc).		
			@param program The program being written.
			@param params The program parameters.
			@param defaultParams The default program parameters.
			*/
			void gpuProgramRefEventRaised(MaterialSerializer* ser, 
				SerializeEvent event, bool& skip,
				const String& attrib, 
				const GpuProgramPtr& program, 
				const GpuProgramParametersSharedPtr& params,
				GpuProgramParameters* defaultParams) {}

			/** Called when texture unit state section event raised.					
			@param ser The MaterialSerializer instance that writes the given material.
			@param stage The current section writing stage.
			@param skip May set to true by sub-class instances in order to skip the following section write.
			This parameter relevant only when stage equals MSE_PRE_WRITE. 
			@param textureUnit The texture unit state that is being written.		
			*/
			virtual void textureUnitStateEventRaised(MaterialSerializer* ser, 
				SerializeEvent event, bool& skip, const TextureUnitState* textureUnit) {}			
		};

    protected:
        /// Keyword-mapped attribute parsers.
        typedef map<String, ATTRIBUTE_PARSER>::type AttribParserList;

        MaterialScriptContext mScriptContext;

        /** internal method for parsing a material
        @returns true if it expects the next line to be a {
        */
        bool parseScriptLine(String& line);
        /** internal method for finding & invoking an attribute parser. */
        bool invokeParser(String& line, AttribParserList& parsers);
		/** Internal method for saving a program definition which has been
		    built up.
		*/
		void finishProgramDefinition(void);
        /// Parsers for the root of the material script
        AttribParserList mRootAttribParsers;
        /// Parsers for the material section of a script
        AttribParserList mMaterialAttribParsers;
        /// Parsers for the technique section of a script
        AttribParserList mTechniqueAttribParsers;
        /// Parsers for the pass section of a script
        AttribParserList mPassAttribParsers;
        /// Parsers for the texture unit section of a script
        AttribParserList mTextureUnitAttribParsers;
        /// Parsers for the program reference section of a script
        AttribParserList mProgramRefAttribParsers;
        /// Parsers for the program definition section of a script
        AttribParserList mProgramAttribParsers;
        /// Parsers for the program definition section of a script
        AttribParserList mProgramDefaultParamAttribParsers;

		/// Listeners list of this Serializer.
		typedef vector<Listener*>::type			ListenerList;
		typedef ListenerList::iterator			ListenerListIterator;
		typedef ListenerList::const_iterator	ListenerListConstIterator;
		ListenerList mListeners;


        void writeMaterial(const MaterialPtr& pMat);
        void writeTechnique(const Technique* pTech);
        void writePass(const Pass* pPass);
        void writeVertexProgramRef(const Pass* pPass);
        void writeShadowCasterVertexProgramRef(const Pass* pPass);
        void writeShadowReceiverVertexProgramRef(const Pass* pPass);
        void writeShadowReceiverFragmentProgramRef(const Pass* pPass);
        void writeFragmentProgramRef(const Pass* pPass);
        void writeGpuProgramRef(const String& attrib, const GpuProgramPtr& program, const GpuProgramParametersSharedPtr& params);
        void writeGpuPrograms(void);
        void writeGPUProgramParameters(const GpuProgramParametersSharedPtr& params, GpuProgramParameters* defaultParams,
            const int level = 4, const bool useMainBuffer = true);
		void writeNamedGpuProgramParameters(const GpuProgramParametersSharedPtr& params, GpuProgramParameters* defaultParams,
			const int level = 4, const bool useMainBuffer = true);
		void writeLowLevelGpuProgramParameters(const GpuProgramParametersSharedPtr& params, GpuProgramParameters* defaultParams,
			const int level = 4, const bool useMainBuffer = true);
		void writeGpuProgramParameter(
			const String& commandName, const String& identifier, 
			const GpuProgramParameters::AutoConstantEntry* autoEntry, 
			const GpuProgramParameters::AutoConstantEntry* defaultAutoEntry, 
			bool isFloat, size_t physicalIndex, size_t physicalSize,
			const GpuProgramParametersSharedPtr& params, GpuProgramParameters* defaultParams,
			const int level, const bool useMainBuffer);
		void writeTextureUnit(const TextureUnitState *pTex);
		void writeSceneBlendFactor(const SceneBlendFactor c_src, const SceneBlendFactor c_dest, 
			const SceneBlendFactor a_src, const SceneBlendFactor a_dest);
		void writeSceneBlendFactor(const SceneBlendFactor sbf_src, const SceneBlendFactor sbf_dest);
		void writeSceneBlendFactor(const SceneBlendFactor sbf);
		void writeCompareFunction(const CompareFunction cf);
		void writeColourValue(const ColourValue &colour, bool writeAlpha = false);
		void writeLayerBlendOperationEx(const LayerBlendOperationEx op);
		void writeLayerBlendSource(const LayerBlendSource lbs);
		
		typedef multimap<TextureUnitState::TextureEffectType, TextureUnitState::TextureEffect>::type EffectMap;

		void writeRotationEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex);
		void writeTransformEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex);
		void writeScrollEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex);
		void writeEnvironmentMapEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex);

        String convertFiltering(FilterOptions fo);

		
		/** Internal methods that invokes registered listeners callback.
		@see Listener::materialEventRaised.
		*/
		void fireMaterialEvent(SerializeEvent event, bool& skip, const Material* mat);

		/** Internal methods that invokes registered listeners callback.
		@see Listener::techniqueEventRaised.
		*/
		void fireTechniqueEvent(SerializeEvent event, bool& skip, const Technique* tech);
		
		/** Internal methods that invokes registered listeners callback.
		@see Listener::passEventRaised.
		*/
		void firePassEvent(SerializeEvent event, bool& skip, const Pass* pass);
		
		/** Internal methods that invokes registered listeners callback.
		@see Listener::gpuProgramRefEventRaised.
		*/
		void fireGpuProgramRefEvent(SerializeEvent event, bool& skip,
			const String& attrib, 
			const GpuProgramPtr& program, 
			const GpuProgramParametersSharedPtr& params,
			GpuProgramParameters* defaultParams);
	

		/** Internal methods that invokes registered listeners callback.
		@see Listener::textureUnitStateEventRaised.
		*/
		void fireTextureUnitStateEvent(SerializeEvent event, bool& skip, const TextureUnitState* textureUnit);
		
   public:		
		/** default constructor*/
		MaterialSerializer();
		/** default destructor*/
		virtual ~MaterialSerializer() {};

		/** Queue an in-memory Material to the internal buffer for export.
		@param pMat Material pointer
		@param clearQueued If true, any materials already queued will be removed
		@param exportDefaults If true, attributes which are defaulted will be
			included in the script exported, otherwise they will be omitted
		*/
        void queueForExport(const MaterialPtr& pMat, bool clearQueued = false, 
			bool exportDefaults = false);
        /** Exports queued material(s) to a named material script file.
        @param filename the file name of the material script to be exported
		@param includeProgDef If true, vertex program and fragment program 
			definitions will be written at the top of the material script
        @param programFilename the file name of the vertex / fragment program 
			script to be exported. This is only used if there are program definitions
            to be exported and includeProgDef is false 
			when calling queueForExport.
        */
		void exportQueued(const String& filename, const bool includeProgDef = false, const String& programFilename = "");
        /** Exports a single in-memory Material to the named material script file.
        @param exportDefaults if true then exports all values including defaults
        @param includeProgDef if true includes Gpu shader program definitions in the
            export material script otherwise if false then program definitions will
            be exported to a separate file with name programFilename if
            programFilename is not empty
        @param programFilename the file name of the vertex / fragment program 
			script to be exported. This is only used if includeProgDef is false.
        */
        void exportMaterial(const MaterialPtr& pMat, const String& filename, bool exportDefaults = false,
            const bool includeProgDef = false, const String& programFilename = "");
		/** Returns a string representing the parsed material(s) */
		const String &getQueuedAsString() const;
		/** Clears the internal buffer */
		void clearQueue();

        /** Parses a Material script file passed as a stream.
        */
        void parseScript(DataStreamPtr& stream, const String& groupName);

		/** Register a listener to this Serializer.
		@see MaterialSerializer::Listener
		*/
		void addListener(Listener* listener);

		/** Remove a listener from this Serializer.
		@see MaterialSerializer::Listener
		*/
		void removeListener(Listener* listener);

	private:
		String mBuffer;
        String mGpuProgramBuffer;
        typedef set<String>::type GpuProgramDefinitionContainer;
        typedef GpuProgramDefinitionContainer::iterator GpuProgramDefIterator;
        GpuProgramDefinitionContainer mGpuProgramDefinitionContainer;
		bool mDefaults;
		
	public:
        void beginSection(unsigned short level, const bool useMainBuffer = true)
		{
            String& buffer = (useMainBuffer ? mBuffer : mGpuProgramBuffer);
			buffer += "\n";
            for (unsigned short i = 0; i < level; ++i)
            {
                buffer += "\t";
            }
            buffer += "{";
		}
		void endSection(unsigned short level, const bool useMainBuffer = true)
		{
            String& buffer = (useMainBuffer ? mBuffer : mGpuProgramBuffer);
			buffer += "\n";
            for (unsigned short i = 0; i < level; ++i)
            {
                buffer += "\t";
            }
            buffer += "}";
		}

		void writeAttribute(unsigned short level, const String& att, const bool useMainBuffer = true)
		{
            String& buffer = (useMainBuffer ? mBuffer : mGpuProgramBuffer);
			buffer += "\n";
            for (unsigned short i = 0; i < level; ++i)
            {
                buffer += "\t";
            }
            buffer += att;
		}

		void writeValue(const String& val, const bool useMainBuffer = true)
		{
            String& buffer = (useMainBuffer ? mBuffer : mGpuProgramBuffer);
			buffer += (" " + val);
		}

		void writeComment(unsigned short level, const String& comment, const bool useMainBuffer = true)
		{
            String& buffer = (useMainBuffer ? mBuffer : mGpuProgramBuffer);
			buffer += "\n";
            for (unsigned short i = 0; i < level; ++i)
            {
                buffer += "\t";
            }
            buffer += "// " + comment;
		}



    };
	/** @} */
	/** @} */
}
#endif
