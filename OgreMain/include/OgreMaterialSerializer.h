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



	private:
		String mBuffer;
        String mGpuProgramBuffer;
        typedef set<String>::type GpuProgramDefinitionContainer;
        typedef GpuProgramDefinitionContainer::iterator GpuProgramDefIterator;
        GpuProgramDefinitionContainer mGpuProgramDefinitionContainer;
		bool mDefaults;

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
