/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreD3D9HLSLProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreStringConverter.h"
#include "OgreD3D9GpuProgram.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    D3D9HLSLProgram::CmdEntryPoint D3D9HLSLProgram::msCmdEntryPoint;
    D3D9HLSLProgram::CmdTarget D3D9HLSLProgram::msCmdTarget;
    D3D9HLSLProgram::CmdPreprocessorDefines D3D9HLSLProgram::msCmdPreprocessorDefines;
    D3D9HLSLProgram::CmdColumnMajorMatrices D3D9HLSLProgram::msCmdColumnMajorMatrices;

	class HLSLIncludeHandler : public ID3DXInclude
	{
	public:
		HLSLIncludeHandler(Resource* sourceProgram) 
			: mProgram(sourceProgram) {}
		~HLSLIncludeHandler() {}
		
		STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType,
			LPCSTR pFileName,
			LPCVOID pParentData,
			LPCVOID *ppData,
			UINT *pByteLen
			)
		{
			// find & load source code
			DataStreamPtr stream = 
				ResourceGroupManager::getSingleton().openResource(
				String(pFileName), mProgram->getGroup(), true, mProgram);

			String source = stream->getAsString();
			// copy into separate c-string
			// Note - must NOT copy the null terminator, otherwise this will terminate
			// the entire program string!
			*pByteLen = static_cast<UINT>(source.length());
			char* pChar = new char[*pByteLen];
			memcpy(pChar, source.c_str(), *pByteLen);
			*ppData = pChar;

			return S_OK;
		}

		STDMETHOD(Close)(LPCVOID pData)
		{
			char* pChar = (char*)pData;
			delete [] pChar;
			return S_OK;
		}
	protected:
		Resource* mProgram;


	};

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    void D3D9HLSLProgram::loadFromSource(void)
    {
        // Populate preprocessor defines
        String stringBuffer;

        std::vector<D3DXMACRO> defines;
        const D3DXMACRO* pDefines = 0;
        if (!mPreprocessorDefines.empty())
        {
            stringBuffer = mPreprocessorDefines;

            // Split preprocessor defines and build up macro array
            D3DXMACRO macro;
            String::size_type pos = 0;
            while (pos != String::npos)
            {
                macro.Name = &stringBuffer[pos];
                macro.Definition = 0;

				String::size_type start_pos=pos;

                // Find delims
                pos = stringBuffer.find_first_of(";,=", pos);

				if(start_pos==pos)
				{
					if(pos==stringBuffer.length())
					{
						break;
					}
					pos++;
					continue;
				}

                if (pos != String::npos)
                {
                    // Check definition part
                    if (stringBuffer[pos] == '=')
                    {
                        // Setup null character for macro name
                        stringBuffer[pos++] = '\0';
                        macro.Definition = &stringBuffer[pos];
                        pos = stringBuffer.find_first_of(";,", pos);
                    }
                    else
                    {
                        // No definition part, define as "1"
                        macro.Definition = "1";
                    }

                    if (pos != String::npos)
                    {
                        // Setup null character for macro name or definition
                        stringBuffer[pos++] = '\0';
                    }
                }
				else
				{
					macro.Definition = "1";
				}
				if(strlen(macro.Name)>0)
				{
					defines.push_back(macro);
				}
				else
				{
					break;
				}
            }

            // Add NULL terminator
            macro.Name = 0;
            macro.Definition = 0;
            defines.push_back(macro);

            pDefines = &defines[0];
        }

        // Populate compile flags
        DWORD compileFlags = 0;
        if (mColumnMajorMatrices)
            compileFlags |= D3DXSHADER_PACKMATRIX_COLUMNMAJOR;
        else
            compileFlags |= D3DXSHADER_PACKMATRIX_ROWMAJOR;

#if OGRE_DEBUG_MODE
		compileFlags |= D3DXSHADER_DEBUG;
#endif

        LPD3DXBUFFER errors = 0;

		// include handler
		HLSLIncludeHandler includeHandler(this);

        // Compile & assemble into microcode
        HRESULT hr = D3DXCompileShader(
            mSource.c_str(),
            static_cast<UINT>(mSource.length()),
            pDefines,
            &includeHandler, 
            mEntryPoint.c_str(),
            mTarget.c_str(),
            compileFlags,
            &mpMicroCode,
            &errors,
            &mpConstTable);

        if (FAILED(hr))
        {
            String message = "Cannot assemble D3D9 high-level shader " + mName + " Errors:\n" +
                static_cast<const char*>(errors->GetBufferPointer());
            errors->Release();
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, message,
                "D3D9HLSLProgram::loadFromSource");
        }


    }
    //-----------------------------------------------------------------------
    void D3D9HLSLProgram::createLowLevelImpl(void)
    {
		if (!mCompileError)
		{
			// Create a low-level program, give it the same name as us
			mAssemblerProgram = 
				GpuProgramManager::getSingleton().createProgramFromString(
					mName, 
					mGroup,
					"",// dummy source, since we'll be using microcode
					mType, 
					mTarget);
			static_cast<D3D9GpuProgram*>(mAssemblerProgram.get())->setExternalMicrocode(mpMicroCode);
		}

    }
    //-----------------------------------------------------------------------
    void D3D9HLSLProgram::unloadHighLevelImpl(void)
    {
        SAFE_RELEASE(mpMicroCode);
        SAFE_RELEASE(mpConstTable);

    }
    //-----------------------------------------------------------------------
    void D3D9HLSLProgram::buildConstantDefinitions() const
    {
        // Derive parameter names from const table
        assert(mpConstTable && "Program not loaded!");
        // Get contents of the constant table
        D3DXCONSTANTTABLE_DESC desc;
        HRESULT hr = mpConstTable->GetDesc(&desc);

		mFloatLogicalToPhysical.bufferSize = 0;
		mIntLogicalToPhysical.bufferSize = 0;
		mConstantDefs.floatBufferSize = 0;
		mConstantDefs.intBufferSize = 0;

        if (FAILED(hr))
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Cannot retrieve constant descriptions from HLSL program.", 
                "D3D9HLSLProgram::buildParameterNameMap");
        }
        // Iterate over the constants
        for (unsigned int i = 0; i < desc.Constants; ++i)
        {
            // Recursively descend through the structure levels
            processParamElement(NULL, "", i);
        }

        
    }
    //-----------------------------------------------------------------------
    void D3D9HLSLProgram::processParamElement(D3DXHANDLE parent, String prefix, 
        unsigned int index) const
    {
        D3DXHANDLE hConstant = mpConstTable->GetConstant(parent, index);

        // Since D3D HLSL doesn't deal with naming of array and struct parameters
        // automatically, we have to do it by hand

        D3DXCONSTANT_DESC desc;
        unsigned int numParams = 1;
        HRESULT hr = mpConstTable->GetConstantDesc(hConstant, &desc, &numParams);
        if (FAILED(hr))
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, 
                "Cannot retrieve constant description from HLSL program.", 
                "D3D9HLSLProgram::processParamElement");
        }

        String paramName = desc.Name;
        // trim the odd '$' which appears at the start of the names in HLSL
        if (paramName.at(0) == '$')
            paramName.erase(paramName.begin());

		// Also trim the '[0]' suffix if it exists, we will add our own indexing later
		if (StringUtil::endsWith(paramName, "[0]", false))
		{
			paramName.erase(paramName.size() - 3);
		}


        if (desc.Class == D3DXPC_STRUCT)
        {
            // work out a new prefix for nested members, if it's an array, we need an index
            prefix = prefix + paramName + ".";
            // Cascade into struct
            for (unsigned int i = 0; i < desc.StructMembers; ++i)
            {
                processParamElement(hConstant, prefix, i);
            }
        }
        else
        {
            // Process params
            if (desc.Type == D3DXPT_FLOAT || desc.Type == D3DXPT_INT || desc.Type == D3DXPT_BOOL)
            {
                size_t paramIndex = desc.RegisterIndex;
                String name = prefix + paramName;
                
				GpuConstantDefinition def;
				def.logicalIndex = paramIndex;
				// populate type, array size & element size
				populateDef(desc, def);
				if (def.isFloat())
				{
					def.physicalIndex = mFloatLogicalToPhysical.bufferSize;
					OGRE_LOCK_MUTEX(mFloatLogicalToPhysical.mutex)
					mFloatLogicalToPhysical.map.insert(
						GpuLogicalIndexUseMap::value_type(paramIndex, 
						GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize)));
					mFloatLogicalToPhysical.bufferSize += def.arraySize * def.elementSize;
					mConstantDefs.floatBufferSize = mFloatLogicalToPhysical.bufferSize;
				}
				else
				{
					def.physicalIndex = mIntLogicalToPhysical.bufferSize;
					OGRE_LOCK_MUTEX(mIntLogicalToPhysical.mutex)
					mIntLogicalToPhysical.map.insert(
						GpuLogicalIndexUseMap::value_type(paramIndex, 
						GpuLogicalIndexUse(def.physicalIndex, def.arraySize * def.elementSize)));
					mIntLogicalToPhysical.bufferSize += def.arraySize * def.elementSize;
					mConstantDefs.intBufferSize = mIntLogicalToPhysical.bufferSize;
				}

                mConstantDefs.map.insert(GpuConstantDefinitionMap::value_type(name, def));

				// Now deal with arrays
				mConstantDefs.generateConstantDefinitionArrayEntries(name, def);
            }
        }
            
    }
	//-----------------------------------------------------------------------
	void D3D9HLSLProgram::populateDef(D3DXCONSTANT_DESC& d3dDesc, GpuConstantDefinition& def) const
	{
		def.arraySize = d3dDesc.Elements;
		switch(d3dDesc.Type)
		{
		case D3DXPT_INT:
			switch(d3dDesc.Columns)
			{
			case 1:
				def.constType = GCT_INT1;
				def.elementSize = 4; // HLSL always packs
				break;
			case 2:
				def.constType = GCT_INT2;
				def.elementSize = 4; // HLSL always packs
				break;
			case 3:
				def.constType = GCT_INT3;
				def.elementSize = 4; // HLSL always packs
				break;
			case 4:
				def.constType = GCT_INT4;
				def.elementSize = 4; 
				break;
			} // columns
			break;
		case D3DXPT_FLOAT:
			switch(d3dDesc.Rows)
			{
			case 1:
				switch(d3dDesc.Columns)
				{
				case 1:
					def.constType = GCT_FLOAT1;
					def.elementSize = 4; // HLSL always packs
					break;
				case 2:
					def.constType = GCT_FLOAT2;
					def.elementSize = 4; // HLSL always packs
					break;
				case 3:
					def.constType = GCT_FLOAT3;
					def.elementSize = 4; // HLSL always packs
					break;
				case 4:
					def.constType = GCT_FLOAT4;
					def.elementSize = 4; 
					break;
				} // columns
				break;
			case 2:
				switch(d3dDesc.Columns)
				{
				case 2:
					def.constType = GCT_MATRIX_2X2;
					def.elementSize = 8; // HLSL always packs
					break;
				case 3:
					def.constType = GCT_MATRIX_2X3;
					def.elementSize = 8; // HLSL always packs
					break;
				case 4:
					def.constType = GCT_MATRIX_2X4;
					def.elementSize = 8; 
					break;
				} // columns
				break;
			case 3:
				switch(d3dDesc.Columns)
				{
				case 2:
					def.constType = GCT_MATRIX_3X2;
					def.elementSize = 12; // HLSL always packs
					break;
				case 3:
					def.constType = GCT_MATRIX_3X3;
					def.elementSize = 12; // HLSL always packs
					break;
				case 4:
					def.constType = GCT_MATRIX_3X4;
					def.elementSize = 12; 
					break;
				} // columns
				break;
			case 4:
				switch(d3dDesc.Columns)
				{
				case 2:
					def.constType = GCT_MATRIX_4X2;
					def.elementSize = 16; // HLSL always packs
					break;
				case 3:
					def.constType = GCT_MATRIX_4X3;
					def.elementSize = 16; // HLSL always packs
					break;
				case 4:
					def.constType = GCT_MATRIX_4X4;
					def.elementSize = 16; 
					break;
				} // columns
				break;

			} // rows
			break;
			
		default:
			// not mapping samplers, don't need to take the space 
			break;
		};

	}
    //-----------------------------------------------------------------------
    D3D9HLSLProgram::D3D9HLSLProgram(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader)
        : HighLevelGpuProgram(creator, name, handle, group, isManual, loader)
        , mTarget()
        , mEntryPoint()
        , mPreprocessorDefines()
        , mColumnMajorMatrices(true)
        , mpMicroCode(NULL), mpConstTable(NULL)
    {
        if (createParamDictionary("D3D9HLSLProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("entry_point", 
                "The entry point for the HLSL program.",
                PT_STRING),&msCmdEntryPoint);
            dict->addParameter(ParameterDef("target", 
                "Name of the assembler target to compile down to.",
                PT_STRING),&msCmdTarget);
            dict->addParameter(ParameterDef("preprocessor_defines", 
                "Preprocessor defines use to compile the program.",
                PT_STRING),&msCmdPreprocessorDefines);
            dict->addParameter(ParameterDef("column_major_matrices", 
                "Whether matrix packing in column-major order.",
                PT_BOOL),&msCmdColumnMajorMatrices);
        }
        
    }
    //-----------------------------------------------------------------------
    D3D9HLSLProgram::~D3D9HLSLProgram()
    {
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        if (isLoaded())
        {
            unload();
        }
        else
        {
            unloadHighLevel();
        }
    }
    //-----------------------------------------------------------------------
    bool D3D9HLSLProgram::isSupported(void) const
    {
        if (mCompileError || !isRequiredCapabilitiesSupported())
            return false;

        return GpuProgramManager::getSingleton().isSyntaxSupported(mTarget);
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr D3D9HLSLProgram::createParameters(void)
    {
        // Call superclass
        GpuProgramParametersSharedPtr params = HighLevelGpuProgram::createParameters();

        // Need to transpose matrices if compiled with column-major matrices
        params->setTransposeMatrices(mColumnMajorMatrices);

        return params;
    }
    //-----------------------------------------------------------------------
    void D3D9HLSLProgram::setTarget(const String& target)
    {
        mTarget = target;
    }

    //-----------------------------------------------------------------------
    const String& D3D9HLSLProgram::getLanguage(void) const
    {
        static const String language = "hlsl";

        return language;
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String D3D9HLSLProgram::CmdEntryPoint::doGet(const void *target) const
    {
        return static_cast<const D3D9HLSLProgram*>(target)->getEntryPoint();
    }
    void D3D9HLSLProgram::CmdEntryPoint::doSet(void *target, const String& val)
    {
        static_cast<D3D9HLSLProgram*>(target)->setEntryPoint(val);
    }
    //-----------------------------------------------------------------------
    String D3D9HLSLProgram::CmdTarget::doGet(const void *target) const
    {
        return static_cast<const D3D9HLSLProgram*>(target)->getTarget();
    }
    void D3D9HLSLProgram::CmdTarget::doSet(void *target, const String& val)
    {
        static_cast<D3D9HLSLProgram*>(target)->setTarget(val);
    }
    //-----------------------------------------------------------------------
    String D3D9HLSLProgram::CmdPreprocessorDefines::doGet(const void *target) const
    {
        return static_cast<const D3D9HLSLProgram*>(target)->getPreprocessorDefines();
    }
    void D3D9HLSLProgram::CmdPreprocessorDefines::doSet(void *target, const String& val)
    {
        static_cast<D3D9HLSLProgram*>(target)->setPreprocessorDefines(val);
    }
    //-----------------------------------------------------------------------
    String D3D9HLSLProgram::CmdColumnMajorMatrices::doGet(const void *target) const
    {
        return StringConverter::toString(static_cast<const D3D9HLSLProgram*>(target)->getColumnMajorMatrices());
    }
    void D3D9HLSLProgram::CmdColumnMajorMatrices::doSet(void *target, const String& val)
    {
        static_cast<D3D9HLSLProgram*>(target)->setColumnMajorMatrices(StringConverter::parseBool(val));
    }

}
