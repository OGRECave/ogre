/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

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
#ifndef __GpuProgramParams_H_
#define __GpuProgramParams_H_

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreSharedPtr.h"
#include "OgreIteratorWrapper.h"
#include "OgreSerializer.h"
#include "OgreAny.h"
#include "Threading/OgreThreadHeaders.h"
#include "OgreHeaderPrefix.h"

namespace Ogre {
    struct TransformBaseReal;

    /** \addtogroup Core
     *  @{
     */
    /** \addtogroup Materials
     *  @{
     */

    enum BaseConstantType
    {
        BCT_FLOAT = 0,
        BCT_INT = 0x10,
        BCT_DOUBLE = 0x20,
        BCT_UINT = 0x30,
        BCT_BOOL = 0x40,
        BCT_SAMPLER = 0x50,
        BCT_SUBROUTINE = 0x60,
        BCT_UNKNOWN = 0x70
    };

    /** Enumeration of the types of constant we may encounter in programs.
        @note Low-level programs, by definition, will always use either
        float4 or int4 constant types since that is the fundamental underlying
        type in assembler.
    */
    enum GpuConstantType
    {
        GCT_FLOAT1 = BCT_FLOAT + 1,
        GCT_FLOAT2 = BCT_FLOAT + 2,
        GCT_FLOAT3 = BCT_FLOAT + 3,
        GCT_FLOAT4 = BCT_FLOAT + 4,
        GCT_SAMPLER1D = BCT_SAMPLER + 1,
        GCT_SAMPLER2D = BCT_SAMPLER + 2,
        GCT_SAMPLER3D = BCT_SAMPLER + 3,
        GCT_SAMPLERCUBE = BCT_SAMPLER + 4,
        GCT_SAMPLERRECT = BCT_SAMPLER +5, //!< @deprecated do not use
        GCT_SAMPLER1DSHADOW = BCT_SAMPLER + 6,
        GCT_SAMPLER2DSHADOW = BCT_SAMPLER + 7,
        GCT_SAMPLER2DARRAY = BCT_SAMPLER + 8,
        GCT_SAMPLER_EXTERNAL_OES = BCT_SAMPLER + 9,
        GCT_MATRIX_2X2 = BCT_FLOAT + 5,
        GCT_MATRIX_2X3 = BCT_FLOAT + 6,
        GCT_MATRIX_2X4 = BCT_FLOAT + 7,
        GCT_MATRIX_3X2 = BCT_FLOAT + 8,
        GCT_MATRIX_3X3 = BCT_FLOAT + 9,
        GCT_MATRIX_3X4 = BCT_FLOAT + 10,
        GCT_MATRIX_4X2 = BCT_FLOAT + 11,
        GCT_MATRIX_4X3 = BCT_FLOAT + 12,
        GCT_MATRIX_4X4 = BCT_FLOAT + 13,
        GCT_INT1 = BCT_INT + 1,
        GCT_INT2 = BCT_INT + 2,
        GCT_INT3 = BCT_INT + 3,
        GCT_INT4 = BCT_INT + 4,
        GCT_SUBROUTINE = BCT_SUBROUTINE,
        GCT_DOUBLE1 = BCT_DOUBLE + 1,
        GCT_DOUBLE2 = BCT_DOUBLE + 2,
        GCT_DOUBLE3 = BCT_DOUBLE + 3,
        GCT_DOUBLE4 = BCT_DOUBLE + 4,
        GCT_MATRIX_DOUBLE_2X2 = BCT_DOUBLE + 5,
        GCT_MATRIX_DOUBLE_2X3 = BCT_DOUBLE + 6,
        GCT_MATRIX_DOUBLE_2X4 = BCT_DOUBLE + 7,
        GCT_MATRIX_DOUBLE_3X2 = BCT_DOUBLE + 8,
        GCT_MATRIX_DOUBLE_3X3 = BCT_DOUBLE + 9,
        GCT_MATRIX_DOUBLE_3X4 = BCT_DOUBLE + 10,
        GCT_MATRIX_DOUBLE_4X2 = BCT_DOUBLE + 11,
        GCT_MATRIX_DOUBLE_4X3 = BCT_DOUBLE + 12,
        GCT_MATRIX_DOUBLE_4X4 = BCT_DOUBLE + 13,
        GCT_UINT1 = BCT_UINT + 1,
        GCT_UINT2 = BCT_UINT + 2,
        GCT_UINT3 = BCT_UINT + 3,
        GCT_UINT4 = BCT_UINT + 4,
        GCT_BOOL1 = BCT_BOOL + 1,
        GCT_BOOL2 = BCT_BOOL + 2,
        GCT_BOOL3 = BCT_BOOL + 3,
        GCT_BOOL4 = BCT_BOOL + 4,
        GCT_UNKNOWN = BCT_UNKNOWN
    };

    /** The variability of a GPU parameter, as derived from auto-params targeting it.
        These values must be powers of two since they are used in masks.
    */
    enum GpuParamVariability
    {
        /// No variation except by manual setting - the default
        GPV_GLOBAL = 1,
        /// Varies per object (based on an auto param usually), but not per light setup
        GPV_PER_OBJECT = 2,
        /// Varies with light setup
        GPV_LIGHTS = 4,
        /// Varies with pass iteration number
        GPV_PASS_ITERATION_NUMBER = 8,


        /// Full mask (16-bit)
        GPV_ALL = 0xFFFF
    };

    /** Information about predefined program constants.
        @note Only available for high-level programs but is referenced generically
        by GpuProgramParameters.
    */
    struct _OgreExport GpuConstantDefinition
    {
        /// Physical start index in buffer (either float, double, int, or uint buffer)
        size_t physicalIndex;
        /// Logical index - used to communicate this constant to the rendersystem
        size_t logicalIndex;
        /** Number of raw buffer slots per element
            (some programs pack each array element to float4, some do not) */
        size_t elementSize;
        /// Length of array
        size_t arraySize;
        /// Data type
        GpuConstantType constType;
        /// How this parameter varies (bitwise combination of GpuProgramVariability)
        mutable uint16 variability;

        //TODO Should offset be added to list?
        // For instance, for GLSL atomic counters:
        // layout(binding = 1, offset = 10) atomic_uint atom_counter;
        // Binding goes in logicalIndex, but where does offset go?
        //size_t offset;

        bool isFloat() const { return isFloat(constType); }
        static bool isFloat(GpuConstantType c) { return getBaseType(c) == BCT_FLOAT; }

        bool isDouble() const { return isDouble(constType); }
        static bool isDouble(GpuConstantType c) { return getBaseType(c) == BCT_DOUBLE; }

        bool isInt() const { return isInt(constType); }
        static bool isInt(GpuConstantType c) { return getBaseType(c) == BCT_INT; }

        bool isUnsignedInt() const { return isUnsignedInt(constType); }
        static bool isUnsignedInt(GpuConstantType c) { return getBaseType(c) == BCT_UINT; }

        bool isBool() const { return isBool(constType); }
        static bool isBool(GpuConstantType c) { return getBaseType(c) == BCT_BOOL; }

        bool isSampler() const { return isSampler(constType); }
        static bool isSampler(GpuConstantType c) { return getBaseType(c) == BCT_SAMPLER; }

        bool isSubroutine() const { return isSubroutine(constType); }
        static bool isSubroutine(GpuConstantType c) { return getBaseType(c) == BCT_SUBROUTINE; }

        static BaseConstantType getBaseType(GpuConstantType ctype)
        {
            return BaseConstantType(ctype / 0x10 * 0x10);
        }

        /** Get the number of elements of a given type, including whether to pad the
            elements into multiples of 4 (e.g. SM1 and D3D does, GLSL doesn't)
        */
        static size_t getElementSize(GpuConstantType ctype, bool padToMultiplesOf4)
        {
            if (padToMultiplesOf4)
            {
                switch(ctype)
                {
                case GCT_FLOAT1:
                case GCT_INT1:
                case GCT_UINT1:
                case GCT_BOOL1:
                case GCT_SAMPLER1D:
                case GCT_SAMPLER2D:
                case GCT_SAMPLER2DARRAY:
                case GCT_SAMPLER3D:
                case GCT_SAMPLERCUBE:
                case GCT_SAMPLER1DSHADOW:
                case GCT_SAMPLER2DSHADOW:
                case GCT_FLOAT2:
                case GCT_INT2:
                case GCT_UINT2:
                case GCT_BOOL2:
                case GCT_FLOAT3:
                case GCT_INT3:
                case GCT_UINT3:
                case GCT_BOOL3:
                case GCT_FLOAT4:
                case GCT_INT4:
                case GCT_UINT4:
                case GCT_BOOL4:
                    return 4;
                case GCT_MATRIX_2X2:
                case GCT_MATRIX_2X3:
                case GCT_MATRIX_2X4:
                case GCT_DOUBLE1:
                case GCT_DOUBLE2:
                case GCT_DOUBLE3:
                case GCT_DOUBLE4:
                    return 8; // 2 float4s
                case GCT_MATRIX_3X2:
                case GCT_MATRIX_3X3:
                case GCT_MATRIX_3X4:
                    return 12; // 3 float4s
                case GCT_MATRIX_4X2:
                case GCT_MATRIX_4X3:
                case GCT_MATRIX_4X4:
                case GCT_MATRIX_DOUBLE_2X2:
                case GCT_MATRIX_DOUBLE_2X3:
                case GCT_MATRIX_DOUBLE_2X4:
                    return 16; // 4 float4s
                case GCT_MATRIX_DOUBLE_3X2:
                case GCT_MATRIX_DOUBLE_3X3:
                case GCT_MATRIX_DOUBLE_3X4:
                    return 24;
                case GCT_MATRIX_DOUBLE_4X2:
                case GCT_MATRIX_DOUBLE_4X3:
                case GCT_MATRIX_DOUBLE_4X4:
                    return 32;
                default:
                    return 4;
                };
            }
            else
            {
                switch(ctype)
                {
                case GCT_SAMPLER1D:
                case GCT_SAMPLER2D:
                case GCT_SAMPLER2DARRAY:
                case GCT_SAMPLER3D:
                case GCT_SAMPLERCUBE:
                case GCT_SAMPLER1DSHADOW:
                case GCT_SAMPLER2DSHADOW:
                    return 1;
                case GCT_MATRIX_2X2:
                case GCT_MATRIX_DOUBLE_2X2:
                    return 4;
                case GCT_MATRIX_2X3:
                case GCT_MATRIX_3X2:
                case GCT_MATRIX_DOUBLE_2X3:
                case GCT_MATRIX_DOUBLE_3X2:
                    return 6;
                case GCT_MATRIX_2X4:
                case GCT_MATRIX_4X2:
                case GCT_MATRIX_DOUBLE_2X4:
                case GCT_MATRIX_DOUBLE_4X2:
                    return 8;
                case GCT_MATRIX_3X3:
                case GCT_MATRIX_DOUBLE_3X3:
                    return 9;
                case GCT_MATRIX_3X4:
                case GCT_MATRIX_4X3:
                case GCT_MATRIX_DOUBLE_3X4:
                case GCT_MATRIX_DOUBLE_4X3:
                    return 12;
                case GCT_MATRIX_4X4:
                case GCT_MATRIX_DOUBLE_4X4:
                    return 16;
                default:
                    return ctype % 0x10;
                };

            }
        }

    GpuConstantDefinition()
        : physicalIndex((std::numeric_limits<size_t>::max)())
            , logicalIndex(0)
            , elementSize(0)
            , arraySize(1)
            , constType(GCT_UNKNOWN)
            , variability(GPV_GLOBAL) {}
    };
    typedef std::map<String, GpuConstantDefinition> GpuConstantDefinitionMap;
    typedef ConstMapIterator<GpuConstantDefinitionMap> GpuConstantDefinitionIterator;

    /// Struct collecting together the information for named constants.
    struct _OgreExport GpuNamedConstants : public GpuParamsAlloc
    {
        /// Total size of the float buffer required
        size_t floatBufferSize;
        /// Total size of the double buffer required
        size_t doubleBufferSize;
        /// Total size of the int buffer required
        size_t intBufferSize;
        /// Map of parameter names to GpuConstantDefinition
        GpuConstantDefinitionMap map;

        GpuNamedConstants();
        ~GpuNamedConstants();

        /// @deprecated obsolete
        OGRE_DEPRECATED void generateConstantDefinitionArrayEntries(const String& paramName,
                                                    const GpuConstantDefinition& baseDef);

        /// @deprecated obsolete
        OGRE_DEPRECATED static bool getGenerateAllConstantDefinitionArrayEntries();

        /// @deprecated obsolete
        OGRE_DEPRECATED static void setGenerateAllConstantDefinitionArrayEntries(bool generateAll);

        /** Saves constant definitions to a file, compatible with GpuProgram::setManualNamedConstantsFile.
            @see GpuProgram::setManualNamedConstantsFile
        */
        void save(const String& filename) const;
        /** Loads constant definitions from a stream, compatible with GpuProgram::setManualNamedConstantsFile.
            @see GpuProgram::setManualNamedConstantsFile
        */
        void load(DataStreamPtr& stream);

        size_t calculateSize(void) const;

    protected:
        static bool msGenerateAllConstantDefinitionArrayEntries;
    };

    /// Simple class for loading / saving GpuNamedConstants
    class _OgreExport GpuNamedConstantsSerializer : public Serializer
    {
    public:
        GpuNamedConstantsSerializer();
        virtual ~GpuNamedConstantsSerializer();
        void exportNamedConstants(const GpuNamedConstants* pConsts, const String& filename,
                                  Endian endianMode = ENDIAN_NATIVE);
        void exportNamedConstants(const GpuNamedConstants* pConsts, DataStreamPtr stream,
                                  Endian endianMode = ENDIAN_NATIVE);
        void importNamedConstants(DataStreamPtr& stream, GpuNamedConstants* pDest);
    };

    /** Structure recording the use of a physical buffer by a logical parameter
        index. Only used for low-level programs.
    */
    struct _OgreExport GpuLogicalIndexUse
    {
        /// Physical buffer index
        size_t physicalIndex;
        /// Current physical size allocation
        size_t currentSize;
        /// How the contents of this slot vary
        mutable uint16 variability;

    GpuLogicalIndexUse()
        : physicalIndex(99999), currentSize(0), variability(GPV_GLOBAL) {}
    GpuLogicalIndexUse(size_t bufIdx, size_t curSz, uint16 v)
        : physicalIndex(bufIdx), currentSize(curSz), variability(v) {}
    };
    typedef std::map<size_t, GpuLogicalIndexUse> GpuLogicalIndexUseMap;
    /// Container struct to allow params to safely & update shared list of logical buffer assignments
    struct _OgreExport GpuLogicalBufferStruct : public GpuParamsAlloc
    {
        OGRE_MUTEX(mutex);

        /// Map from logical index to physical buffer location
        GpuLogicalIndexUseMap map;
        /// Shortcut to know the buffer size needs
        size_t bufferSize;
        GpuLogicalBufferStruct();
        ~GpuLogicalBufferStruct();
    };

    /** Definition of container that holds the current float constants.
        @note Not necessarily in direct index order to constant indexes, logical
        to physical index map is derived from GpuProgram
    */
    typedef std::vector<float> FloatConstantList;
    /** Definition of container that holds the current double constants.
        @note Not necessarily in direct index order to constant indexes, logical
        to physical index map is derived from GpuProgram
    */
    typedef std::vector<double> DoubleConstantList;
    /** Definition of container that holds the current int constants.
        @note Not necessarily in direct index order to constant indexes, logical
        to physical index map is derived from GpuProgram
    */
    typedef std::vector<int> IntConstantList;
    /** Definition of container that holds the current uint constants.
        @note Not necessarily in direct index order to constant indexes, logical
        to physical index map is derived from GpuProgram
    */
    typedef std::vector<uint> UnsignedIntConstantList;

    /** A group of manually updated parameters that are shared between many parameter sets.
        @remarks
        Sometimes you want to set some common parameters across many otherwise
        different parameter sets, and keep them all in sync together. This class
        allows you to define a set of parameters that you can share across many
        parameter sets and have the parameters that match automatically be pulled
        from the shared set, rather than you having to set them on all the parameter
        sets individually.
        @par
        Parameters in a shared set are matched up with instances in a GpuProgramParameters
        structure by matching names. It is up to you to define the named parameters
        that a shared set contains, and ensuring the definition matches.
        @note
        Shared parameter sets can be named, and looked up using the GpuProgramManager.
    */
    class _OgreExport GpuSharedParameters : public GpuParamsAlloc
    {
    protected:
        /// Name of the shared parameter set.
        String mName;

        /// Shared parameter definitions and related data.
        GpuNamedConstants mNamedConstants;

        /// List of float constant values.
        FloatConstantList mFloatConstants;
        /// List of double constants values.
        DoubleConstantList mDoubleConstants;
        /// List of int constant values.
        IntConstantList mIntConstants;

        /// Optional data the rendersystem might want to store.
        mutable Any mRenderSystemData;
        /// Optional rendersystem backed storage
        HardwareBufferPtr mHardwareBuffer;

        /// Not used when copying data, but might be useful to RS using shared buffers.
        size_t mFrameLastUpdated;

        /// Version number of the definitions in this buffer.
        unsigned long mVersion;

		/// Accumulated offset used to calculate uniform location.
		size_t mOffset;

        bool mDirty;

    public:
        GpuSharedParameters(const String& name);

        /// Get the name of this shared parameter set.
        const String& getName() { return mName; }

        /** Add a new constant definition to this shared set of parameters.
            @remarks
            Unlike GpuProgramParameters, where the parameter list is defined by the
            program being compiled, this shared parameter set is defined by the
            user. Only parameters which have been predefined here may be later
            updated.
        */
        void addConstantDefinition(const String& name, GpuConstantType constType, size_t arraySize = 1);

        /// @deprecated removing a constant requires a full rebuild due to changed alignments
        OGRE_DEPRECATED void removeConstantDefinition(const String& name);

        /** Remove a constant definition from this shared set of parameters.
         */
        void removeAllConstantDefinitions();

        /** Get the version number of this shared parameter set, can be used to identify when
            changes have occurred.
        */
        unsigned long getVersion() const { return mVersion; }

        /** Calculate the expected size of the shared parameter buffer based
            on constant definition data types.
        */
        size_t calculateSize(void) const;

        /** True if this parameter set is dirty (values have been modified,
            but the render system has not updated them yet).
        */
        bool isDirty() const { return mDirty; }

        /** Mark the shared set as being clean (values successfully updated
            by the render system).
            @remarks
            You do not need to call this yourself. The set is marked as clean
            whenever the render system updates dirty shared parameters.
        */
        void _markClean();

        /** Mark the shared set as being dirty (values modified and not yet
            updated in render system).
            @remarks
            You do not need to call this yourself. The set is marked as
            dirty whenever setNamedConstant or (non const) getFloatPointer
            et al are called.
        */
        void _markDirty();

        /// Get the frame in which this shared parameter set was last updated
        size_t getFrameLastUpdated() const { return mFrameLastUpdated; }

        /// @deprecated use getConstantDefinitions()
        OGRE_DEPRECATED GpuConstantDefinitionIterator getConstantDefinitionIterator(void) const;

        /** Get a specific GpuConstantDefinition for a named parameter.
         */
        const GpuConstantDefinition& getConstantDefinition(const String& name) const;

        /** Get the full list of GpuConstantDefinition instances.
         */
        const GpuNamedConstants& getConstantDefinitions() const;

        /** @copydoc GpuProgramParameters::setNamedConstant(const String&, Real) */
        template <typename T> void setNamedConstant(const String& name, T val)
        {
            setNamedConstant(name, &val, 1);
        }
        /// @overload
        template <int dims, typename T>
        void setNamedConstant(const String& name, const Vector<dims, T>& vec)
        {
            setNamedConstant(name, vec.ptr(), dims);
        }
        /** @copydoc GpuProgramParameters::setNamedConstant(const String& name, const Matrix4& m) */
        void setNamedConstant(const String& name, const Matrix4& m);
        /** @copydoc GpuProgramParameters::setNamedConstant(const String& name, const Matrix4* m, size_t numEntries) */
        void setNamedConstant(const String& name, const Matrix4* m, size_t numEntries);
        void setNamedConstant(const String& name, const float *val, size_t count);
        void setNamedConstant(const String& name, const double *val, size_t count);
        /** @copydoc GpuProgramParameters::setNamedConstant(const String& name, const ColourValue& colour) */
        void setNamedConstant(const String& name, const ColourValue& colour);
        void setNamedConstant(const String& name, const int *val, size_t count);
        void setNamedConstant(const String& name, const uint *val, size_t count);
        /// Get a pointer to the 'nth' item in the float buffer
        float* getFloatPointer(size_t pos) { _markDirty(); return &mFloatConstants[pos]; }
        /// Get a pointer to the 'nth' item in the float buffer
        const float* getFloatPointer(size_t pos) const { return &mFloatConstants[pos]; }
        /// Get a pointer to the 'nth' item in the double buffer
        double* getDoublePointer(size_t pos) { _markDirty(); return &mDoubleConstants[pos]; }
        /// Get a pointer to the 'nth' item in the double buffer
        const double* getDoublePointer(size_t pos) const { return &mDoubleConstants[pos]; }
        /// Get a pointer to the 'nth' item in the int buffer
        int* getIntPointer(size_t pos) { _markDirty(); return &mIntConstants[pos]; }
        /// Get a pointer to the 'nth' item in the int buffer
        const int* getIntPointer(size_t pos) const { return &mIntConstants[pos]; }
        /// Get a pointer to the 'nth' item in the uint buffer
        uint* getUnsignedIntPointer(size_t pos) { _markDirty(); return (uint*)&mIntConstants[pos]; }
        /// Get a pointer to the 'nth' item in the uint buffer
        const uint* getUnsignedIntPointer(size_t pos) const { return (const uint*)&mIntConstants[pos]; }
        /// Get a reference to the list of float constants
        const FloatConstantList& getFloatConstantList() const { return mFloatConstants; }
        /// Get a reference to the list of double constants
        const DoubleConstantList& getDoubleConstantList() const { return mDoubleConstants; }
        /// Get a reference to the list of int constants
        const IntConstantList& getIntConstantList() const { return mIntConstants; }
        /// @deprecated use _setHardwareBuffer
        OGRE_DEPRECATED void _setRenderSystemData(const Any& data) const { mRenderSystemData = data; }
        /// @deprecated use _getHardwareBuffer
        OGRE_DEPRECATED const Any& _getRenderSystemData() const { return mRenderSystemData; }
        /** Internal method that the RenderSystem might use to store optional data. */
        void _setHardwareBuffer(const HardwareBufferPtr& data) { mHardwareBuffer = data; }
        /** Internal method that the RenderSystem might use to store optional data. */
        const HardwareBufferPtr& _getHardwareBuffer() const { return mHardwareBuffer; }
        /// upload parameter data to GPU memory. Must have a HardwareBuffer
        void _upload() const;
        /// download data from GPU memory. Must have a writable HardwareBuffer
        void download();
    };

    class GpuProgramParameters;

    /** This class records the usage of a set of shared parameters in a concrete
        set of GpuProgramParameters.
    */
    class _OgreExport GpuSharedParametersUsage : public GpuParamsAlloc
    {
    protected:
        GpuSharedParametersPtr mSharedParams;
        // Not a shared pointer since this is also parent
        GpuProgramParameters* mParams;
        // list of physical mappings that we are going to bring in
        struct CopyDataEntry
        {
            const GpuConstantDefinition* srcDefinition;
            const GpuConstantDefinition* dstDefinition;
        };
        typedef std::vector<CopyDataEntry> CopyDataList;

        CopyDataList mCopyDataList;

        // Optional data the rendersystem might want to store
        mutable Any mRenderSystemData;

        /// Version of shared params we based the copydata on
        unsigned long mCopyDataVersion;

        void initCopyData();


    public:
        /// Construct usage
        GpuSharedParametersUsage(GpuSharedParametersPtr sharedParams,
                                 GpuProgramParameters* params);

        /** Update the target parameters by copying the data from the shared
            parameters.
            @note This method  may not actually be called if the RenderSystem
            supports using shared parameters directly in their own shared buffer; in
            which case the values should not be copied out of the shared area
            into the individual parameter set, but bound separately.
        */
        void _copySharedParamsToTargetParams() const;

        /// Get the name of the shared parameter set
        const String& getName() const { return mSharedParams->getName(); }

        GpuSharedParametersPtr getSharedParams() const { return mSharedParams; }
        GpuProgramParameters* getTargetParams() const { return mParams; }

        /// @deprecated use GpuSharedParameters::_setHardwareBuffer
        OGRE_DEPRECATED void _setRenderSystemData(const Any& data) const { mRenderSystemData = data; }
        /// @deprecated use GpuSharedParameters::_getHardwareBuffer
        OGRE_DEPRECATED const Any& _getRenderSystemData() const { return mRenderSystemData; }


    };

    /** Collects together the program parameters used for a GpuProgram.
        @remarks
        Gpu program state includes constant parameters used by the program, and
        bindings to render system state which is propagated into the constants
        by the engine automatically if requested.
        @par
        GpuProgramParameters objects should be created through the GpuProgram and
        may be shared between multiple Pass instances. For this reason they
        are managed using a shared pointer, which will ensure they are automatically
        deleted when no Pass is using them anymore.
        @par
        High-level programs use named parameters (uniforms), low-level programs
        use indexed constants. This class supports both, but you can tell whether
        named constants are supported by calling hasNamedParameters(). There are
        references in the documentation below to 'logical' and 'physical' indexes;
        logical indexes are the indexes used by low-level programs and represent
        indexes into an array of float4's, some of which may be settable, some of
        which may be predefined constants in the program. We only store those
        constants which have actually been set, therefore our buffer could have
        gaps if we used the logical indexes in our own buffers. So instead we map
        these logical indexes to physical indexes in our buffer. When using
        high-level programs, logical indexes don't necessarily exist, although they
        might if the high-level program has a direct, exposed mapping from parameter
        names to logical indexes. In addition, high-level languages may or may not pack
        arrays of elements that are smaller than float4 (e.g. float2/vec2) contiguously.
        This kind of information is held in the ConstantDefinition structure which
        is only populated for high-level programs. You don't have to worry about
        any of this unless you intend to read parameters back from this structure
        rather than just setting them.
    */
    class _OgreExport GpuProgramParameters : public GpuParamsAlloc
    {
    public:
        /** Defines the types of automatically updated values that may be bound to GpuProgram
            parameters, or used to modify parameters on a per-object basis.

            For use in @ref Program-Parameter-Specification, drop the `ACT_` prefix. 
            E.g. `ACT_WORLD_MATRIX` becomes `world_matrix`.
        */
        enum AutoConstantType
        {
            /// The current world matrix
            ACT_WORLD_MATRIX,
            /// The current world matrix, inverted
            ACT_INVERSE_WORLD_MATRIX,
            /** Provides transpose of world matrix.
                Equivalent to RenderMonkey's "WorldTranspose".
            */
            ACT_TRANSPOSE_WORLD_MATRIX,
            /// The current world matrix, inverted & transposed
            ACT_INVERSE_TRANSPOSE_WORLD_MATRIX,

            /// An array of world matrices, each represented as only a 3x4 matrix (3 rows of
            /// 4columns) usually for doing hardware skinning.
            /// You should make enough entries available in your vertex program for the number of
            /// bones in use, i.e. an array of numBones*3 float4’s.
            ACT_WORLD_MATRIX_ARRAY_3x4,
            /// The current array of world matrices, used for blending
            ACT_WORLD_MATRIX_ARRAY,
            /// The current array of world matrices transformed to an array of dual quaternions,
            /// represented as a 2x4 matrix
            ACT_WORLD_DUALQUATERNION_ARRAY_2x4,
            /// The scale and shear components of the current array of world matrices
            ACT_WORLD_SCALE_SHEAR_MATRIX_ARRAY_3x4,

            /// The current view matrix
            ACT_VIEW_MATRIX,
            /// The current view matrix, inverted
            ACT_INVERSE_VIEW_MATRIX,
            /** Provides transpose of view matrix.
                Equivalent to RenderMonkey's "ViewTranspose".
            */
            ACT_TRANSPOSE_VIEW_MATRIX,
            /** Provides inverse transpose of view matrix.
                Equivalent to RenderMonkey's "ViewInverseTranspose".
            */
            ACT_INVERSE_TRANSPOSE_VIEW_MATRIX,

            /// The current projection matrix
            ACT_PROJECTION_MATRIX,
            /** Provides inverse of projection matrix.
                Equivalent to RenderMonkey's "ProjectionInverse".
            */
            ACT_INVERSE_PROJECTION_MATRIX,
            /** Provides transpose of projection matrix.
                Equivalent to RenderMonkey's "ProjectionTranspose".
            */
            ACT_TRANSPOSE_PROJECTION_MATRIX,
            /** Provides inverse transpose of projection matrix.
                Equivalent to RenderMonkey's "ProjectionInverseTranspose".
            */
            ACT_INVERSE_TRANSPOSE_PROJECTION_MATRIX,

            /// The current view & projection matrices concatenated
            ACT_VIEWPROJ_MATRIX,
            /** Provides inverse of concatenated view and projection matrices.
                Equivalent to RenderMonkey's "ViewProjectionInverse".
            */
            ACT_INVERSE_VIEWPROJ_MATRIX,
            /** Provides transpose of concatenated view and projection matrices.
                Equivalent to RenderMonkey's "ViewProjectionTranspose".
            */
            ACT_TRANSPOSE_VIEWPROJ_MATRIX,
            /** Provides inverse transpose of concatenated view and projection matrices.
                Equivalent to RenderMonkey's "ViewProjectionInverseTranspose".
            */
            ACT_INVERSE_TRANSPOSE_VIEWPROJ_MATRIX,

            /// The current world & view matrices concatenated
            ACT_WORLDVIEW_MATRIX,
            /// The current world & view matrices concatenated, then inverted
            ACT_INVERSE_WORLDVIEW_MATRIX,
            /** Provides transpose of concatenated world and view matrices.
                Equivalent to RenderMonkey's "WorldViewTranspose".
            */
            ACT_TRANSPOSE_WORLDVIEW_MATRIX,
            /// The current world & view matrices concatenated, then inverted & transposed
            ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX,
            /** Provides inverse transpose of the upper 3x3 of the worldview matrix.
                Equivalent to "gl_NormalMatrix".
            */
            ACT_NORMAL_MATRIX,

            /// The current world, view & projection matrices concatenated
            ACT_WORLDVIEWPROJ_MATRIX,
            /** Provides inverse of concatenated world, view and projection matrices.
                Equivalent to RenderMonkey's "WorldViewProjectionInverse".
            */
            ACT_INVERSE_WORLDVIEWPROJ_MATRIX,
            /** Provides transpose of concatenated world, view and projection matrices.
                Equivalent to RenderMonkey's "WorldViewProjectionTranspose".
            */
            ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX,
            /** Provides inverse transpose of concatenated world, view and projection
                matrices. Equivalent to RenderMonkey's "WorldViewProjectionInverseTranspose".
            */
            ACT_INVERSE_TRANSPOSE_WORLDVIEWPROJ_MATRIX,

            // render target related values
            /** -1 if requires texture flipping, +1 otherwise. It's useful when you bypassed
                projection matrix transform, still able use this value to adjust transformed y
               position.
            */
            ACT_RENDER_TARGET_FLIPPING,

            /** -1 if the winding has been inverted (e.g. for reflections), +1 otherwise.
             */
            ACT_VERTEX_WINDING,

            /// Fog colour
            ACT_FOG_COLOUR,
            /// Fog params: density, linear start, linear end, 1/(end-start)
            ACT_FOG_PARAMS,

            /// Surface ambient colour, as set in Pass::setAmbient
            ACT_SURFACE_AMBIENT_COLOUR,
            /// Surface diffuse colour, as set in Pass::setDiffuse
            ACT_SURFACE_DIFFUSE_COLOUR,
            /// Surface specular colour, as set in Pass::setSpecular
            ACT_SURFACE_SPECULAR_COLOUR,
            /// Surface emissive colour, as set in Pass::setSelfIllumination
            ACT_SURFACE_EMISSIVE_COLOUR,
            /// Surface shininess, as set in Pass::setShininess
            ACT_SURFACE_SHININESS,
            /// Surface alpha rejection value, not as set in @ref Pass::setAlphaRejectValue, but a
            /// floating number between 0.0f and 1.0f instead (255.0f /
            /// @ref Pass::getAlphaRejectValue())
            ACT_SURFACE_ALPHA_REJECTION_VALUE,

            /// The number of active light sources (better than gl_MaxLights)
            ACT_LIGHT_COUNT,

            /// The ambient light colour set in the scene
            ACT_AMBIENT_LIGHT_COLOUR,

            /// Light diffuse colour (index determined by setAutoConstant call)
            ///
            /// this requires an index in the ’extra_params’ field, and relates to the ’nth’ closest
            /// light which could affect this object
            /// (i.e. 0 refers to the closest light - note that directional lights are always first
            /// in the list and always present).
            /// NB if there are no lights this close, then the parameter will be set to black.
            ACT_LIGHT_DIFFUSE_COLOUR,
            /// Light specular colour (index determined by setAutoConstant call)
            ACT_LIGHT_SPECULAR_COLOUR,
            /// Light attenuation parameters, Vector4(range, constant, linear, quadric)
            ACT_LIGHT_ATTENUATION,
            /** Spotlight parameters, Vector4(innerFactor, outerFactor, falloff, isSpot)
                innerFactor and outerFactor are cos(angle/2)
                The isSpot parameter is 0.0f for non-spotlights, 1.0f for spotlights.
                Also for non-spotlights the inner and outer factors are 1 and nearly 1 respectively
            */
            ACT_SPOTLIGHT_PARAMS,
            /** A light position in world space (index determined by setAutoConstant call)

             This requires an index in the ’extra_params’ field, and relates to the ’nth’ closest
             light which could affect this object (i.e. 0 refers to the closest light).
             NB if there are no lights this close, then the parameter will be set to all zeroes.
             Note that this property will work with all kinds of lights, even directional lights,
             since the parameter is set as a 4D vector.
             Point lights will be (pos.x, pos.y, pos.z, 1.0f) whilst directional lights will be
             (-dir.x, -dir.y, -dir.z, 0.0f).
             Operations like dot products will work consistently on both.
             */
            ACT_LIGHT_POSITION,
            /// A light position in object space (index determined by setAutoConstant call)
            ACT_LIGHT_POSITION_OBJECT_SPACE,
            /// A light position in view space (index determined by setAutoConstant call)
            ACT_LIGHT_POSITION_VIEW_SPACE,
            /// A light direction in world space (index determined by setAutoConstant call)
            /// @deprecated this property only works on directional lights, and we recommend that
            /// you use light_position instead since that returns a generic 4D vector.
            ACT_LIGHT_DIRECTION,
            /// A light direction in object space (index determined by setAutoConstant call)
            ACT_LIGHT_DIRECTION_OBJECT_SPACE,
            /// A light direction in view space (index determined by setAutoConstant call)
            ACT_LIGHT_DIRECTION_VIEW_SPACE,
            /** The distance of the light from the center of the object
                a useful approximation as an alternative to per-vertex distance
                calculations.
            */
            ACT_LIGHT_DISTANCE_OBJECT_SPACE,
            /** Light power level, a single scalar as set in Light::setPowerScale  (index determined
               by setAutoConstant call) */
            ACT_LIGHT_POWER_SCALE,
            /// Light diffuse colour pre-scaled by Light::setPowerScale (index determined by
            /// setAutoConstant call)
            ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED,
            /// Light specular colour pre-scaled by Light::setPowerScale (index determined by
            /// setAutoConstant call)
            ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED,
            /// Array of light diffuse colours (count set by extra param)
            ACT_LIGHT_DIFFUSE_COLOUR_ARRAY,
            /// Array of light specular colours (count set by extra param)
            ACT_LIGHT_SPECULAR_COLOUR_ARRAY,
            /// Array of light diffuse colours scaled by light power (count set by extra param)
            ACT_LIGHT_DIFFUSE_COLOUR_POWER_SCALED_ARRAY,
            /// Array of light specular colours scaled by light power (count set by extra param)
            ACT_LIGHT_SPECULAR_COLOUR_POWER_SCALED_ARRAY,
            /// Array of light attenuation parameters, Vector4(range, constant, linear, quadric)
            /// (count set by extra param)
            ACT_LIGHT_ATTENUATION_ARRAY,
            /// Array of light positions in world space (count set by extra param)
            ACT_LIGHT_POSITION_ARRAY,
            /// Array of light positions in object space (count set by extra param)
            ACT_LIGHT_POSITION_OBJECT_SPACE_ARRAY,
            /// Array of light positions in view space (count set by extra param)
            ACT_LIGHT_POSITION_VIEW_SPACE_ARRAY,
            /// Array of light directions in world space (count set by extra param)
            ACT_LIGHT_DIRECTION_ARRAY,
            /// Array of light directions in object space (count set by extra param)
            ACT_LIGHT_DIRECTION_OBJECT_SPACE_ARRAY,
            /// Array of light directions in view space (count set by extra param)
            ACT_LIGHT_DIRECTION_VIEW_SPACE_ARRAY,
            /** Array of distances of the lights from the center of the object
                a useful approximation as an alternative to per-vertex distance
                calculations. (count set by extra param)
            */
            ACT_LIGHT_DISTANCE_OBJECT_SPACE_ARRAY,
            /** Array of light power levels, a single scalar as set in Light::setPowerScale
                (count set by extra param)
            */
            ACT_LIGHT_POWER_SCALE_ARRAY,
            /** Spotlight parameters array of Vector4(innerFactor, outerFactor, falloff, isSpot)
                innerFactor and outerFactor are cos(angle/2)
                The isSpot parameter is 0.0f for non-spotlights, 1.0f for spotlights.
                Also for non-spotlights the inner and outer factors are 1 and nearly 1 respectively.
                (count set by extra param)
            */
            ACT_SPOTLIGHT_PARAMS_ARRAY,

            /** The derived ambient light colour, with 'r', 'g', 'b' components filled with
                product of surface ambient colour and ambient light colour, respectively,
                and 'a' component filled with surface ambient alpha component.
            */
            ACT_DERIVED_AMBIENT_LIGHT_COLOUR,
            /** The derived scene colour, with 'r', 'g' and 'b' components filled with sum
                of derived ambient light colour and surface emissive colour, respectively,
                and 'a' component filled with surface diffuse alpha component.
            */
            ACT_DERIVED_SCENE_COLOUR,

            /** The derived light diffuse colour (index determined by setAutoConstant call),
                with 'r', 'g' and 'b' components filled with product of surface diffuse colour,
                light power scale and light diffuse colour, respectively, and 'a' component filled
               with surface
                diffuse alpha component.
            */
            ACT_DERIVED_LIGHT_DIFFUSE_COLOUR,
            /** The derived light specular colour (index determined by setAutoConstant call),
                with 'r', 'g' and 'b' components filled with product of surface specular colour
                and light specular colour, respectively, and 'a' component filled with surface
                specular alpha component.
            */
            ACT_DERIVED_LIGHT_SPECULAR_COLOUR,

            /// Array of derived light diffuse colours (count set by extra param)
            ACT_DERIVED_LIGHT_DIFFUSE_COLOUR_ARRAY,
            /// Array of derived light specular colours (count set by extra param)
            ACT_DERIVED_LIGHT_SPECULAR_COLOUR_ARRAY,
            /** The absolute light number of a local light index. Each pass may have
                a number of lights passed to it, and each of these lights will have
                an index in the overall light list, which will differ from the local
                light index due to factors like setStartLight and setIteratePerLight.
                This binding provides the global light index for a local index.
            */
            ACT_LIGHT_NUMBER,
            /// Returns (int) 1 if the  given light casts shadows, 0 otherwise (index set in extra
            /// param)
            ACT_LIGHT_CASTS_SHADOWS,
            /// Returns (int) 1 if the  given light casts shadows, 0 otherwise (index set in extra
            /// param)
            ACT_LIGHT_CASTS_SHADOWS_ARRAY,

            /** The distance a shadow volume should be extruded when using
                finite extrusion programs.
            */
            ACT_SHADOW_EXTRUSION_DISTANCE,
            /// The current camera's position in world space
            ACT_CAMERA_POSITION,
            /// The current camera's position in object space
            ACT_CAMERA_POSITION_OBJECT_SPACE,
            /// The current camera's position in world space even when camera relative rendering is enabled
            ACT_CAMERA_RELATIVE_POSITION,

            /** The view/projection matrix of the assigned texture projection frustum

             Applicable to vertex programs which have been specified as the ’shadow receiver’ vertex
             program alternative, or where a texture unit is marked as content_type shadow; this
             provides details of the view/projection matrix for the current shadow projector. The
             optional ’extra_params’ entry specifies which light the projector refers to (for the
             case of content_type shadow where more than one shadow texture may be present in a
             single pass), where 0 is the default and refers to the first light referenced in this
             pass.
             */
            ACT_TEXTURE_VIEWPROJ_MATRIX,
            /// Array of view/projection matrices of the first n texture projection frustums
            ACT_TEXTURE_VIEWPROJ_MATRIX_ARRAY,
            /** The view/projection matrix of the assigned texture projection frustum,
                combined with the current world matrix
            */
            ACT_TEXTURE_WORLDVIEWPROJ_MATRIX,
            /// Array of world/view/projection matrices of the first n texture projection frustums
            ACT_TEXTURE_WORLDVIEWPROJ_MATRIX_ARRAY,
            /// The view/projection matrix of a given spotlight
            ACT_SPOTLIGHT_VIEWPROJ_MATRIX,
            /// Array of view/projection matrix of a given spotlight
            ACT_SPOTLIGHT_VIEWPROJ_MATRIX_ARRAY,
            /** The view/projection matrix of a given spotlight projection frustum,
                combined with the current world matrix
            */
            ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX,
            /** An array of the view/projection matrix of a given spotlight projection frustum,
                combined with the current world matrix
            */
            ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX_ARRAY,
            /** A custom parameter which will come from the renderable, using 'data' as the
             identifier

             This allows you to map a custom parameter on an individual Renderable (see
             Renderable::setCustomParameter) to a parameter on a GPU program. It requires that you
             complete the ’extra_params’ field with the index that was used in the
             Renderable::setCustomParameter call, and this will ensure that whenever this Renderable
             is used, it will have it’s custom parameter mapped in. It’s very important that this
             parameter has been defined on all Renderables that are assigned the material that
             contains this automatic mapping, otherwise the process will fail.
             */
            ACT_CUSTOM,
            /** provides current elapsed time
             */
            ACT_TIME,
            /** Single float value, which repeats itself based on given as
                parameter "cycle time". Equivalent to RenderMonkey's "Time0_X".
            */
            ACT_TIME_0_X,
            /// Cosine of "Time0_X". Equivalent to RenderMonkey's "CosTime0_X".
            ACT_COSTIME_0_X,
            /// Sine of "Time0_X". Equivalent to RenderMonkey's "SinTime0_X".
            ACT_SINTIME_0_X,
            /// Tangent of "Time0_X". Equivalent to RenderMonkey's "TanTime0_X".
            ACT_TANTIME_0_X,
            /** Vector of "Time0_X", "SinTime0_X", "CosTime0_X",
                "TanTime0_X". Equivalent to RenderMonkey's "Time0_X_Packed".
            */
            ACT_TIME_0_X_PACKED,
            /** Single float value, which represents scaled time value [0..1],
                which repeats itself based on given as parameter "cycle time".
                Equivalent to RenderMonkey's "Time0_1".
            */
            ACT_TIME_0_1,
            /// Cosine of "Time0_1". Equivalent to RenderMonkey's "CosTime0_1".
            ACT_COSTIME_0_1,
            /// Sine of "Time0_1". Equivalent to RenderMonkey's "SinTime0_1".
            ACT_SINTIME_0_1,
            /// Tangent of "Time0_1". Equivalent to RenderMonkey's "TanTime0_1".
            ACT_TANTIME_0_1,
            /** Vector of "Time0_1", "SinTime0_1", "CosTime0_1",
                "TanTime0_1". Equivalent to RenderMonkey's "Time0_1_Packed".
            */
            ACT_TIME_0_1_PACKED,
            /** Single float value, which represents scaled time value [0..2*Pi],
                which repeats itself based on given as parameter "cycle time".
                Equivalent to RenderMonkey's "Time0_2PI".
            */
            ACT_TIME_0_2PI,
            /// Cosine of "Time0_2PI". Equivalent to RenderMonkey's "CosTime0_2PI".
            ACT_COSTIME_0_2PI,
            /// Sine of "Time0_2PI". Equivalent to RenderMonkey's "SinTime0_2PI".
            ACT_SINTIME_0_2PI,
            /// Tangent of "Time0_2PI". Equivalent to RenderMonkey's "TanTime0_2PI".
            ACT_TANTIME_0_2PI,
            /** Vector of "Time0_2PI", "SinTime0_2PI", "CosTime0_2PI",
                "TanTime0_2PI". Equivalent to RenderMonkey's "Time0_2PI_Packed".
            */
            ACT_TIME_0_2PI_PACKED,
            /// provides the scaled frame time, returned as a floating point value.
            ACT_FRAME_TIME,
            /// provides the calculated frames per second, returned as a floating point value.
            ACT_FPS,
            // viewport-related values
            /** Current viewport width (in pixels) as floating point value.
                Equivalent to RenderMonkey's "ViewportWidth".
            */
            ACT_VIEWPORT_WIDTH,
            /** Current viewport height (in pixels) as floating point value.
                Equivalent to RenderMonkey's "ViewportHeight".
            */
            ACT_VIEWPORT_HEIGHT,
            /** This variable represents 1.0/ViewportWidth.
                Equivalent to RenderMonkey's "ViewportWidthInverse".
            */
            ACT_INVERSE_VIEWPORT_WIDTH,
            /** This variable represents 1.0/ViewportHeight.
                Equivalent to RenderMonkey's "ViewportHeightInverse".
            */
            ACT_INVERSE_VIEWPORT_HEIGHT,
            /** Packed of "ViewportWidth", "ViewportHeight", "ViewportWidthInverse",
                "ViewportHeightInverse".
            */
            ACT_VIEWPORT_SIZE,

            // view parameters
            /** This variable provides the view direction vector (world space).
                Equivalent to RenderMonkey's "ViewDirection".
            */
            ACT_VIEW_DIRECTION,
            /** This variable provides the view side vector (world space).
                Equivalent to RenderMonkey's "ViewSideVector".
            */
            ACT_VIEW_SIDE_VECTOR,
            /** This variable provides the view up vector (world space).
                Equivalent to RenderMonkey's "ViewUpVector".
            */
            ACT_VIEW_UP_VECTOR,
            /** This variable provides the field of view as a floating point value.
                Equivalent to RenderMonkey's "FOV".
            */
            ACT_FOV,
            /** This variable provides the near clip distance as a floating point value.
                Equivalent to RenderMonkey's "NearClipPlane".
            */
            ACT_NEAR_CLIP_DISTANCE,
            /** This variable provides the far clip distance as a floating point value.
                Equivalent to RenderMonkey's "FarClipPlane".
            */
            ACT_FAR_CLIP_DISTANCE,

            /** provides the pass index number within the technique
                of the active materil.
            */
            ACT_PASS_NUMBER,

            /** provides the current iteration number of the pass. The iteration
                number is the number of times the current render operation has
                been drawn for the active pass.
            */
            ACT_PASS_ITERATION_NUMBER,

            /** Provides a parametric animation value [0..1], only available
                where the renderable specifically implements it.

               For morph animation, sets the parametric value
               (0..1) representing the distance between the first position keyframe (bound to
               positions) and the second position keyframe (bound to the first free texture
               coordinate) so that the vertex program can interpolate between them. For pose
               animation, indicates a group of up to 4 parametric weight values applying to a
               sequence of up to 4 poses (each one bound to x, y, z and w of the constant), one for
               each pose. The original positions are held in the usual position buffer, and the
               offsets to take those positions to the pose where weight == 1.0 are in the first ’n’
               free texture coordinates; ’n’ being determined by the value passed to
               includes_pose_animation. If more than 4 simultaneous poses are required, then you’ll
               need more than 1 shader constant to hold the parametric values, in which case you
               should use this binding more than once, referencing a different constant entry; the
               second one will contain the parametrics for poses 5-8, the third for poses 9-12, and
               so on.
            */
            ACT_ANIMATION_PARAMETRIC,

            /** Provides the texel offsets required by this rendersystem to map
                texels to pixels. Packed as
                float4(absoluteHorizontalOffset, absoluteVerticalOffset,
                horizontalOffset / viewportWidth, verticalOffset / viewportHeight)
            */
            ACT_TEXEL_OFFSETS,

            /** Provides information about the depth range of the scene as viewed
                from the current camera.
                Passed as float4(minDepth, maxDepth, depthRange, 1 / depthRange)
            */
            ACT_SCENE_DEPTH_RANGE,

            /** Provides information about the depth range of the scene as viewed
                from a given shadow camera. Requires an index parameter which maps
                to a light index relative to the current light list.
                Passed as float4(minDepth, maxDepth, depthRange, 1 / depthRange)
            */
            ACT_SHADOW_SCENE_DEPTH_RANGE,

            /** Provides an array of information about the depth range of the scene as viewed
                from a given shadow camera. Requires an index parameter which maps
                to a light index relative to the current light list.
                Passed as float4(minDepth, maxDepth, depthRange, 1 / depthRange)
            */
            ACT_SHADOW_SCENE_DEPTH_RANGE_ARRAY,

            /** Provides the fixed shadow colour as configured via SceneManager::setShadowColour;
                useful for integrated modulative shadows.
            */
            ACT_SHADOW_COLOUR,
            /** Provides texture size of the texture unit (index determined by setAutoConstant
                call). Packed as float4(width, height, depth, 1)
            */
            ACT_TEXTURE_SIZE,
            /** Provides inverse texture size of the texture unit (index determined by
               setAutoConstant
                call). Packed as float4(1 / width, 1 / height, 1 / depth, 1)
            */
            ACT_INVERSE_TEXTURE_SIZE,
            /** Provides packed texture size of the texture unit (index determined by
               setAutoConstant
                call). Packed as float4(width, height, 1 / width, 1 / height)
            */
            ACT_PACKED_TEXTURE_SIZE,

            /** Provides the current transform matrix of the texture unit (index determined by
               setAutoConstant
                call), as seen by the fixed-function pipeline.

                This requires an index in the ’extra_params’ field, and relates to the ’nth’ texture
               unit of the pass in question.
                NB if the given index exceeds the number of texture units available for this pass,
               then the parameter will be set to Matrix4::IDENTITY.
            */
            ACT_TEXTURE_MATRIX,

            /** Provides the position of the LOD camera in world space, allowing you
                to perform separate LOD calculations in shaders independent of the rendering
                camera. If there is no separate LOD camera then this is the real camera
                position. See Camera::setLodCamera.
            */
            ACT_LOD_CAMERA_POSITION,
            /** Provides the position of the LOD camera in object space, allowing you
                to perform separate LOD calculations in shaders independent of the rendering
                camera. If there is no separate LOD camera then this is the real camera
                position. See Camera::setLodCamera.
            */
            ACT_LOD_CAMERA_POSITION_OBJECT_SPACE,
            /** Binds custom per-light constants to the shaders. */
            ACT_LIGHT_CUSTOM,
            /// Point params: size; constant, linear, quadratic attenuation
            ACT_POINT_PARAMS,

            ACT_UNKNOWN = 999
        };

        /** Defines the type of the extra data item used by the auto constant.

         */
        enum ACDataType {
            /// no data is required
            ACDT_NONE,
            /// the auto constant requires data of type int
            ACDT_INT,
            /// the auto constant requires data of type float
            ACDT_REAL
        };

        /** Defines the base element type of the auto constant
         */
        enum ElementType {
            ET_INT,
            // float
            ET_REAL
        };

        /** Structure defining an auto constant that's available for use in
            a parameters object.
        */
        struct AutoConstantDefinition
        {
            AutoConstantType acType;
            String name;
            size_t elementCount;
            /// The type of the constant in the program
            ElementType elementType;
            /// The type of any extra data
            ACDataType dataType;

        AutoConstantDefinition(AutoConstantType _acType, const String& _name,
                               size_t _elementCount, ElementType _elementType,
                               ACDataType _dataType)
        :acType(_acType), name(_name), elementCount(_elementCount),
                elementType(_elementType), dataType(_dataType)
            {

            }
        };

        /** Structure recording the use of an automatic parameter. */
        class AutoConstantEntry
        {
        public:
            /// The type of parameter
            AutoConstantType paramType;
            /// The target (physical) constant index
            size_t physicalIndex;
            /// Additional information to go with the parameter
            union{
                uint32 data;
                float fData;
            };
            /// The variability of this parameter (see GpuParamVariability)
            uint16 variability;
            /** The number of elements per individual entry in this constant
                Used in case people used packed elements smaller than 4 (e.g. GLSL)
                and bind an auto which is 4-element packed to it */
            uint8 elementCount;

        AutoConstantEntry(AutoConstantType theType, size_t theIndex, uint32 theData,
                          uint16 theVariability, uint8 theElemCount = 4)
            : paramType(theType), physicalIndex(theIndex),
                data(theData), variability(theVariability), elementCount(theElemCount) {}

        AutoConstantEntry(AutoConstantType theType, size_t theIndex, float theData,
                          uint16 theVariability, uint8 theElemCount = 4)
            : paramType(theType), physicalIndex(theIndex),
                fData(theData), variability(theVariability), elementCount(theElemCount) {}

        };
        // Auto parameter storage
        typedef std::vector<AutoConstantEntry> AutoConstantList;

        typedef std::vector<GpuSharedParametersUsage> GpuSharedParamUsageList;

        // Map that store subroutines associated with slots
        typedef std::unordered_map<size_t, String> SubroutineMap;
        typedef std::unordered_map<size_t, String>::const_iterator SubroutineIterator;

    protected:
        SubroutineMap mSubroutineMap;

        static AutoConstantDefinition AutoConstantDictionary[];

        /// Packed list of floating-point constants (physical indexing)
        FloatConstantList mFloatConstants;
        /// Packed list of double-point constants (physical indexing)
        DoubleConstantList mDoubleConstants;
        /// Packed list of integer constants (physical indexing)
        IntConstantList mIntConstants;
        /** Logical index to physical index map - for low-level programs
            or high-level programs which pass params this way. */
        GpuLogicalBufferStructPtr mFloatLogicalToPhysical;
        /** Logical index to physical index map - for low-level programs
            or high-level programs which pass params this way. */
        GpuLogicalBufferStructPtr mDoubleLogicalToPhysical;
        /** Logical index to physical index map - for low-level programs
            or high-level programs which pass params this way. */
        GpuLogicalBufferStructPtr mIntLogicalToPhysical;

        template <typename T>
        GpuLogicalIndexUse*
        getConstantLogicalIndexUse(const GpuLogicalBufferStructPtr& logicalToPhysical,
                                   std::vector<T>& constants, size_t logicalIndex,
                                   size_t requestedSize, uint16 variability);

        /** Gets the low-level structure for a logical index.
         */
        GpuLogicalIndexUse* _getFloatConstantLogicalIndexUse(size_t logicalIndex, size_t requestedSize, uint16 variability);
        /** Gets the low-level structure for a logical index.
         */
        GpuLogicalIndexUse* _getDoubleConstantLogicalIndexUse(size_t logicalIndex, size_t requestedSize, uint16 variability);
        /** Gets the physical buffer index associated with a logical int constant index.
         */
        GpuLogicalIndexUse* _getIntConstantLogicalIndexUse(size_t logicalIndex, size_t requestedSize, uint16 variability);
        /// Mapping from parameter names to def - high-level programs are expected to populate this
        GpuNamedConstantsPtr mNamedConstants;
        /// List of automatically updated parameters
        AutoConstantList mAutoConstants;
        /// The combined variability masks of all parameters
        uint16 mCombinedVariability;
        /// Do we need to transpose matrices?
        bool mTransposeMatrices;
        /// flag to indicate if names not found will be ignored
        bool mIgnoreMissingParams;
        /// physical index for active pass iteration parameter real constant entry;
        size_t mActivePassIterationIndex;

        /// Return the variability for an auto constant
        static uint16 deriveVariability(AutoConstantType act);

        void copySharedParamSetUsage(const GpuSharedParamUsageList& srcList);

        GpuSharedParamUsageList mSharedParamSets;

        // Optional data the rendersystem might want to store
        mutable Any mRenderSystemData;


    public:
        GpuProgramParameters();
        ~GpuProgramParameters();

        /// Copy constructor
        GpuProgramParameters(const GpuProgramParameters& oth);
        /// Operator = overload
        GpuProgramParameters& operator=(const GpuProgramParameters& oth);

        /** Internal method for providing a link to a name->definition map for parameters. */
        void _setNamedConstants(const GpuNamedConstantsPtr& constantmap);

        /** Internal method for providing a link to a logical index->physical index map for
         * parameters. */
        void _setLogicalIndexes(const GpuLogicalBufferStructPtr& floatIndexMap,
                                const GpuLogicalBufferStructPtr& doubleIndexMap,
                                const GpuLogicalBufferStructPtr& intIndexMap);

        /// Does this parameter set include named parameters?
        bool hasNamedParameters() const { return mNamedConstants.get() != 0; }
        /** Does this parameter set include logically indexed parameters?
            @note Not mutually exclusive with hasNamedParameters since some high-level
            programs still use logical indexes to set the parameters on the
            rendersystem.
        */
        bool hasLogicalIndexedParameters() const { return mFloatLogicalToPhysical.get() != 0; }

        /// @name Set constant by logical index
        /// @{
        /** Sets a 4-element floating-point parameter to the program.
            @param index The logical constant index at which to place the parameter
            (each constant is a 4D float)
            @param vec The value to set
        */
        void setConstant(size_t index, const Vector4& vec);
        /** Sets a single floating-point parameter to the program.
            @note This is actually equivalent to calling
            setConstant(index Vector4(val, 0, 0, 0)) since all constants are 4D.
            @param index The logical constant index at which to place the parameter (each constant is
            a 4D float)
            @param val The value to set
        */
        void setConstant(size_t index, Real val);
        /** Sets a 4-element floating-point parameter to the program via Vector3.
            @param index The logical constant index at which to place the parameter (each constant is
            a 4D float).
            Note that since you're passing a Vector3, the last element of the 4-element
            value will be set to 1 (a homogeneous vector)
            @param vec The value to set
        */
        void setConstant(size_t index, const Vector3& vec);
        /** Sets a 4-element floating-point parameter to the program via Vector2.
            @param index The logical constant index at which to place the parameter (each constant is
            a 4D float).
            Note that since you're passing a Vector2, the last 2 elements of the 4-element
            value will be set to 1 (a homogeneous vector)
            @param vec The value to set
        */
        void setConstant(size_t index, const Vector2& vec);
        /** Sets a Matrix4 parameter to the program.
            @param index The logical constant index at which to place the parameter (each constant is
            a 4D float).
            NB since a Matrix4 is 16 floats long, this parameter will take up 4 indexes.
            @param m The value to set
        */
        void setConstant(size_t index, const Matrix4& m);
        /** Sets a list of Matrix4 parameters to the program.
            @param index The logical constant index at which to start placing the parameter (each constant is
            a 4D float).
            NB since a Matrix4 is 16 floats long, so each entry will take up 4 indexes.
            @param m Pointer to an array of matrices to set
            @param numEntries Number of Matrix4 entries
        */
        void setConstant(size_t index, const Matrix4* m, size_t numEntries);
        /** Sets a ColourValue parameter to the program.
            @param index The logical constant index at which to place the parameter (each constant is
            a 4D float)
            @param colour The value to set
        */
        void setConstant(size_t index, const ColourValue& colour);
        /** Sets a multiple value constant floating-point parameter to the program.
            @param index The logical constant index at which to start placing parameters (each constant is
            a 4D float)
            @param val Pointer to the values to write, must contain 4*count floats
            @param count The number of groups of 4 floats to write
        */
        void setConstant(size_t index, const float *val, size_t count);
        /** Sets a multiple value constant floating-point parameter to the program.
            @param index The logical constant index at which to start placing parameters (each constant is
            a 4D float)
            @param val Pointer to the values to write, must contain 4*count floats
            @param count The number of groups of 4 floats to write
        */
        void setConstant(size_t index, const double *val, size_t count);
        /** Sets a multiple value constant integer parameter to the program.
            @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
            @param index The logical constant index at which to place the parameter (each constant is
            a 4D integer)
            @param val Pointer to the values to write, must contain 4*count ints
            @param count The number of groups of 4 ints to write
        */
        void setConstant(size_t index, const int *val, size_t count);
        /** Sets a multiple value constant unsigned integer parameter to the program.
            @remarks
            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.
            @param index The logical constant index at which to place the parameter (each constant is
            a 4D integer)
            @param val Pointer to the values to write, must contain 4*count ints
            @param count The number of groups of 4 ints to write
        */
        void setConstant(size_t index, const uint *val, size_t count);
        /// @}

        /** @name Set constant by physical index
            You can use these methods if you have already derived the physical
            constant buffer location, for a slight speed improvement over using
            the named / logical index versions.
        */
        /// @{
        /** Write a series of floating point values into the underlying float
            constant buffer at the given physical index.
            @param physicalIndex The buffer position to start writing
            @param val Pointer to a list of values to write
            @param count The number of floats to write
        */
        void _writeRawConstants(size_t physicalIndex, const float* val, size_t count);
        /** Write a series of floating point values into the underlying float
            constant buffer at the given physical index.
            @param physicalIndex The buffer position to start writing
            @param val Pointer to a list of values to write
            @param count The number of floats to write
        */
        void _writeRawConstants(size_t physicalIndex, const double* val, size_t count);
        /** Write a series of integer values into the underlying integer
            constant buffer at the given physical index.
            @param physicalIndex The buffer position to start writing
            @param val Pointer to a list of values to write
            @param count The number of ints to write
        */
        void _writeRawConstants(size_t physicalIndex, const int* val, size_t count);
        /** Write a series of unsigned integer values into the underlying integer
            constant buffer at the given physical index.
            @param physicalIndex The buffer position to start writing
            @param val Pointer to a list of values to write
            @param count The number of ints to write
        */
        void _writeRawConstants(size_t physicalIndex, const uint* val, size_t count);
        /** Write a Vector parameter to the program directly to
            the underlying constants buffer.
            @param physicalIndex The physical buffer index at which to place the parameter
            @param vec The value to set
            @param count The number of floats to write; if for example
            the uniform constant 'slot' is smaller than a Vector4
        */
        template <int dims, typename T>
        void _writeRawConstant(size_t physicalIndex, const Vector<dims, T>& vec, size_t count = dims)
        {
            _writeRawConstants(physicalIndex, vec.ptr(), std::min(count, (size_t)dims));
        }
        /** Write a single parameter to the program.
            @param physicalIndex The physical buffer index at which to place the parameter
            @param val The value to set
        */
        template<typename T>
        void _writeRawConstant(size_t physicalIndex, T val)
        {
            _writeRawConstants(physicalIndex, &val, 1);
        }
        /// @deprecated this will crash if count > 1
        OGRE_DEPRECATED void _writeRawConstant(size_t physicalIndex, Real val, size_t count);
        /** Write a Matrix4 parameter to the program.
            @param physicalIndex The physical buffer index at which to place the parameter
            @param m The value to set
            @param elementCount actual element count used with shader
        */
        void _writeRawConstant(size_t physicalIndex, const Matrix4& m, size_t elementCount);
        /// @overload
        void _writeRawConstant(size_t physicalIndex, const Matrix3& m, size_t elementCount);
        /** Write a list of Matrix4 parameters to the program.
            @param physicalIndex The physical buffer index at which to place the parameter
            @param m The value to set
            @param numEntries Number of Matrix4 entries
        */
        void _writeRawConstant(size_t physicalIndex, const TransformBaseReal* m, size_t numEntries);
        /** Write a ColourValue parameter to the program.
            @param physicalIndex The physical buffer index at which to place the parameter
            @param colour The value to set
            @param count The number of floats to write; if for example
            the uniform constant 'slot' is smaller than a Vector4
        */
        void _writeRawConstant(size_t physicalIndex, const ColourValue& colour,
                               size_t count = 4);
        /// @}

        /** Read a series of floating point values from the underlying float
            constant buffer at the given physical index.
            @param physicalIndex The buffer position to start reading
            @param count The number of floats to read
            @param dest Pointer to a buffer to receive the values
        */
        void _readRawConstants(size_t physicalIndex, size_t count, float* dest);
        /** Read a series of integer values from the underlying integer
            constant buffer at the given physical index.
            @param physicalIndex The buffer position to start reading
            @param count The number of ints to read
            @param dest Pointer to a buffer to receive the values
        */
        void _readRawConstants(size_t physicalIndex, size_t count, int* dest);

        /// @deprecated use getConstantDefinitions()
        OGRE_DEPRECATED GpuConstantDefinitionIterator getConstantDefinitionIterator(void) const;

        /** Get a specific GpuConstantDefinition for a named parameter.
            @note
            Only available if this parameters object has named parameters.
        */
        const GpuConstantDefinition& getConstantDefinition(const String& name) const;

        /** Get the full list of GpuConstantDefinition instances.
            @note
            Only available if this parameters object has named parameters.
        */
        const GpuNamedConstants& getConstantDefinitions() const;

        /** Get the current list of mappings from low-level logical param indexes
            to physical buffer locations in the float buffer.
            @note
            Only applicable to low-level programs.
        */
        const GpuLogicalBufferStructPtr& getFloatLogicalBufferStruct() const { return mFloatLogicalToPhysical; }
        /** Get the current list of mappings from low-level logical param indexes
            to physical buffer locations in the double buffer.
            @note
            Only applicable to low-level programs.
        */
        const GpuLogicalBufferStructPtr& getDoubleLogicalBufferStruct() const { return mDoubleLogicalToPhysical; }
        /** Get the current list of mappings from low-level logical param indexes
            to physical buffer locations in the integer buffer.
            @note
            Only applicable to low-level programs.
        */
        const GpuLogicalBufferStructPtr& getIntLogicalBufferStruct() const { return mIntLogicalToPhysical; }

        /** Retrieves the logical index relating to a physical index in the
            buffer, for programs which support that (low-level programs and
            high-level programs which use logical parameter indexes).
            @return std::numeric_limits<size_t>::max() if not found
        */
        size_t getFloatLogicalIndexForPhysicalIndex(size_t physicalIndex);
        /// @copydoc getFloatLogicalIndexForPhysicalIndex
        size_t getDoubleLogicalIndexForPhysicalIndex(size_t physicalIndex);
        /// @copydoc getFloatLogicalIndexForPhysicalIndex
        size_t getIntLogicalIndexForPhysicalIndex(size_t physicalIndex);
        /// Get a reference to the list of float constants
        const FloatConstantList& getFloatConstantList() const { return mFloatConstants; }
        /// Get a pointer to the 'nth' item in the float buffer
        float* getFloatPointer(size_t pos) { return &mFloatConstants[pos]; }
        /// Get a pointer to the 'nth' item in the float buffer
        const float* getFloatPointer(size_t pos) const { return &mFloatConstants[pos]; }
        /// Get a reference to the list of double constants
        const DoubleConstantList& getDoubleConstantList() const { return mDoubleConstants; }
        /// Get a pointer to the 'nth' item in the double buffer
        double* getDoublePointer(size_t pos) { return &mDoubleConstants[pos]; }
        /// Get a pointer to the 'nth' item in the double buffer
        const double* getDoublePointer(size_t pos) const { return &mDoubleConstants[pos]; }
        /// Get a reference to the list of int constants
        const IntConstantList& getIntConstantList() const { return mIntConstants; }
        /// Get a pointer to the 'nth' item in the int buffer
        int* getIntPointer(size_t pos) { return &mIntConstants[pos]; }
        /// Get a pointer to the 'nth' item in the int buffer
        const int* getIntPointer(size_t pos) const { return &mIntConstants[pos]; }
        /// Get a pointer to the 'nth' item in the uint buffer
        uint* getUnsignedIntPointer(size_t pos) { return (uint*)&mIntConstants[pos]; }
        /// Get a pointer to the 'nth' item in the uint buffer
        const uint* getUnsignedIntPointer(size_t pos) const { return (const uint*)&mIntConstants[pos]; }

        /// @name Automatically derived constants
        /// @{

        /// Get a reference to the list of auto constant bindings
        const AutoConstantList& getAutoConstantList() const { return mAutoConstants; }

        /** Sets up a constant which will automatically be updated by the system.
            @remarks
            Vertex and fragment programs often need parameters which are to do with the
            current render state, or particular values which may very well change over time,
            and often between objects which are being rendered. This feature allows you
            to set up a certain number of predefined parameter mappings that are kept up to
            date for you.
            @param index The location in the constant list to place this updated constant every time
            it is changed. Note that because of the nature of the types, we know how big the
            parameter details will be so you don't need to set that like you do for manual constants.
            @param acType The type of automatic constant to set
            @param extraInfo If the constant type needs more information (like a light index or array size) put it here.
        */
        void setAutoConstant(size_t index, AutoConstantType acType, uint32 extraInfo = 0);
        /// @overload
        void setAutoConstantReal(size_t index, AutoConstantType acType, float rData);
        /// @overload
        void setAutoConstant(size_t index, AutoConstantType acType, uint16 extraInfo1, uint16 extraInfo2)
        {
            setAutoConstant(index, acType, (size_t)extraInfo1 | ((size_t)extraInfo2) << 16);
        }

        /** As setAutoConstant, but sets up the auto constant directly against a
            physical buffer index.
        */
        void _setRawAutoConstant(size_t physicalIndex, AutoConstantType acType, uint32 extraInfo,
                                 uint16 variability, uint8 elementSize = 4);
        /** As setAutoConstantReal, but sets up the auto constant directly against a
            physical buffer index.
        */
        void _setRawAutoConstantReal(size_t physicalIndex, AutoConstantType acType, float rData,
                                     uint16 variability, uint8 elementSize = 4);


        /** Unbind an auto constant so that the constant is manually controlled again. */
        void clearAutoConstant(size_t index);

        /// @deprecated use ACT_TIME directly
        OGRE_DEPRECATED void setConstantFromTime(size_t index, Real factor)
        {
            setAutoConstantReal(index, ACT_TIME, factor);
        }

        /** Clears all the existing automatic constants. */
        void clearAutoConstants(void);

        /** Gets the automatic constant bindings currently in place. */
        const AutoConstantList& getAutoConstants() const {
            return mAutoConstants;
        }

        /// Gets the number of int constants that have been set
        size_t getAutoConstantCount(void) const { return mAutoConstants.size(); }
        /** Gets a specific Auto Constant entry if index is in valid range
            otherwise returns a NULL
            @param index which entry is to be retrieved
        */
        AutoConstantEntry* getAutoConstantEntry(const size_t index);
        /** Returns true if this instance has any automatic constants. */
        bool hasAutoConstants(void) const { return !(mAutoConstants.empty()); }
        /** Finds an auto constant that's affecting a given logical parameter
            index for floating-point values.
            @note Only applicable for low-level programs.
        */
        const AutoConstantEntry* findFloatAutoConstantEntry(size_t logicalIndex);
        /** Finds an auto constant that's affecting a given logical parameter
            index for double-point values.
            @note Only applicable for low-level programs.
        */
        const AutoConstantEntry* findDoubleAutoConstantEntry(size_t logicalIndex);
        /** Finds an auto constant that's affecting a given logical parameter
            index for integer values.
            @note Only applicable for low-level programs.
        */
        const AutoConstantEntry* findIntAutoConstantEntry(size_t logicalIndex);
        /** Finds an auto constant that's affecting a given logical parameter
            index for unsigned integer values.
            @note Only applicable for low-level programs.
        */
        const AutoConstantEntry* findUnsignedIntAutoConstantEntry(size_t logicalIndex);
        /** Finds an auto constant that's affecting a given named parameter index.
            @note Only applicable to high-level programs.
        */
        const AutoConstantEntry* findAutoConstantEntry(const String& paramName) const;
        /** Finds an auto constant that's affecting a given physical position in
            the floating-point buffer
        */
        const AutoConstantEntry* _findRawAutoConstantEntryFloat(size_t physicalIndex) const;
        /** Finds an auto constant that's affecting a given physical position in
            the double-point buffer
        */
        const AutoConstantEntry* _findRawAutoConstantEntryDouble(size_t physicalIndex) const;
        /** Finds an auto constant that's affecting a given physical position in
            the integer buffer
        */
        const AutoConstantEntry* _findRawAutoConstantEntryInt(size_t physicalIndex) const;
        /** Finds an auto constant that's affecting a given physical position in
            the unsigned integer buffer
        */
        const AutoConstantEntry* _findRawAutoConstantEntryUnsignedInt(size_t physicalIndex) const;
        /** Finds an auto constant that's affecting a given physical position in
            the boolean buffer
        */
        const AutoConstantEntry* _findRawAutoConstantEntryBool(size_t physicalIndex) const;
        /** Sets up a constant which will automatically be updated by the system.
            @remarks
            Vertex and fragment programs often need parameters which are to do with the
            current render state, or particular values which may very well change over time,
            and often between objects which are being rendered. This feature allows you
            to set up a certain number of predefined parameter mappings that are kept up to
            date for you.
            @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
            @param name The name of the parameter
            @param acType The type of automatic constant to set
            @param extraInfo If the constant type needs more information (like a light index) put it here.
        */
        void setNamedAutoConstant(const String& name, AutoConstantType acType, size_t extraInfo = 0);
        /// @overload
        void setNamedAutoConstantReal(const String& name, AutoConstantType acType, Real rData);
        /// @overload
        void setNamedAutoConstant(const String& name, AutoConstantType acType, uint16 extraInfo1, uint16 extraInfo2)
        {
            setNamedAutoConstant(name, acType, (size_t)extraInfo1 | ((size_t)extraInfo2) << 16);
        }

        /// @deprecated use ACT_TIME directly
        void setNamedConstantFromTime(const String& name, Real factor)
        {
            setNamedAutoConstantReal(name, ACT_TIME, factor);
        }

        /** Unbind an auto constant so that the constant is manually controlled again. */
        void clearNamedAutoConstant(const String& name);
        /// @}

        /** Update automatic parameters.
            @param source The source of the parameters
            @param variabilityMask A mask of GpuParamVariability which identifies which autos will need updating
        */
        void _updateAutoParams(const AutoParamDataSource* source, uint16 variabilityMask);

        /** Tells the program whether to ignore missing parameters or not.
         */
        void setIgnoreMissingParams(bool state) { mIgnoreMissingParams = state; }

        /// @name Set constant by name
        /// @{
        /** Sets a single value constant parameter to the program.

            Different types of GPU programs support different types of constant parameters.
            For example, it's relatively common to find that vertex programs only support
            floating point constants, and that fragment programs only support integer (fixed point)
            parameters. This can vary depending on the program version supported by the
            graphics card being used. You should consult the documentation for the type of
            low level program you are using, or alternatively use the methods
            provided on RenderSystemCapabilities to determine the options.

            Another possible limitation is that some systems only allow constants to be set
            on certain boundaries, e.g. in sets of 4 values for example. Again, see
            RenderSystemCapabilities for full details.
            @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
            @param name The name of the parameter
            @param val The value to set
        */
        void setNamedConstant(const String& name, Real val);
        /// @overload
        void setNamedConstant(const String& name, int val);
        /// @overload
        void setNamedConstant(const String& name, uint val);
        /// @overload
        void setNamedConstant(const String& name, const Vector4& val);
        /// @overload
        void setNamedConstant(const String& name, const Vector3& val);
        /// @overload
        void setNamedConstant(const String& name, const Vector2& val);
        /// @overload
        void setNamedConstant(const String& name, const Matrix4& val);
        /// @overload
        void setNamedConstant(const String& name, const ColourValue& colour);
        /** Sets a list of Matrix4 parameters to the program.
            @param name The name of the parameter; this must be the first index of an array,
            for examples 'matrices[0]'
            NB since a Matrix4 is 16 floats long, so each entry will take up 4 indexes.
            @param m Pointer to an array of matrices to set
            @param numEntries Number of Matrix4 entries
        */
        void setNamedConstant(const String& name, const Matrix4* m, size_t numEntries);
        /** Sets a multiple value constant parameter to the program.

            Some systems only allow constants to be set on certain boundaries,
            e.g. in sets of 4 values for example. The 'multiple' parameter allows
            you to control that although you should only change it if you know
            your chosen language supports that (at the time of writing, only
            GLSL allows constants which are not a multiple of 4).
            @note
            This named option will only work if you are using a parameters object created
            from a high-level program (HighLevelGpuProgram).
            @param name The name of the parameter
            @param val Pointer to the values to write
            @param count The number of 'multiples' of floats to write
            @param multiple The number of raw entries in each element to write,
            the default is 4 so count = 1 would write 4 floats.
        */
        void setNamedConstant(const String& name, const float *val, size_t count,
                              size_t multiple = 4);
        /// @overload
        void setNamedConstant(const String& name, const double *val, size_t count,
                              size_t multiple = 4);
        /// @overload
        void setNamedConstant(const String& name, const int *val, size_t count,
                              size_t multiple = 4);
        /// @overload
        void setNamedConstant(const String& name, const uint *val, size_t count,
                              size_t multiple = 4);
        /// @}
        /** Find a constant definition for a named parameter.
            @remarks
            This method returns null if the named parameter did not exist, unlike
            getConstantDefinition which is more strict; unless you set the
            last parameter to true.
            @param name The name to look up
            @param throwExceptionIfMissing If set to true, failure to find an entry
            will throw an exception.
        */
        const GpuConstantDefinition* _findNamedConstantDefinition(
            const String& name, bool throwExceptionIfMissing = false) const;
        /** Gets the physical buffer index associated with a logical float constant index.
            @note Only applicable to low-level programs.
            @param logicalIndex The logical parameter index
            @param requestedSize The requested size - pass 0 to ignore missing entries
            and return std::numeric_limits<size_t>::max()
            @param variability
        */
        size_t _getFloatConstantPhysicalIndex(size_t logicalIndex, size_t requestedSize, uint16 variability);
        /** Gets the physical buffer index associated with a logical double constant index.
            @copydetails _getFloatConstantPhysicalIndex
        */
        size_t _getDoubleConstantPhysicalIndex(size_t logicalIndex, size_t requestedSize, uint16 variability);
        /** Gets the physical buffer index associated with a logical int constant index.
            @copydetails _getFloatConstantPhysicalIndex
        */
        size_t _getIntConstantPhysicalIndex(size_t logicalIndex, size_t requestedSize, uint16 variability);
        /** Sets whether or not we need to transpose the matrices passed in from the rest of OGRE.
            @remarks
            D3D uses transposed matrices compared to GL and OGRE; this is not important when you
            use programs which are written to process row-major matrices, such as those generated
            by Cg, but if you use a program written to D3D's matrix layout you will need to enable
            this flag.
        */
        void setTransposeMatrices(bool val) { mTransposeMatrices = val; }
        /// Gets whether or not matrices are to be transposed when set
        bool getTransposeMatrices(void) const { return mTransposeMatrices; }

        /** Copies the values of all constants (including auto constants) from another
            GpuProgramParameters object.
            @note This copes the internal storage of the paarameters object and therefore
            can only be used for parameters objects created from the same GpuProgram.
            To merge parameters that match from different programs, use copyMatchingNamedConstantsFrom.
        */
        void copyConstantsFrom(const GpuProgramParameters& source);

        /** Copies the values of all matching named constants (including auto constants) from
            another GpuProgramParameters object.
            @remarks
            This method iterates over the named constants in another parameters object
            and copies across the values where they match. This method is safe to
            use when the 2 parameters objects came from different programs, but only
            works for named parameters.
        */
        void copyMatchingNamedConstantsFrom(const GpuProgramParameters& source);

        /** gets the auto constant definition associated with name if found else returns NULL
            @param name The name of the auto constant
        */
        static const AutoConstantDefinition* getAutoConstantDefinition(const String& name);
        /** gets the auto constant definition using an index into the auto constant definition array.
            If the index is out of bounds then NULL is returned;
            @param idx The auto constant index
        */
        static const AutoConstantDefinition* getAutoConstantDefinition(const size_t idx);
        /** Returns the number of auto constant definitions
         */
        static size_t getNumAutoConstantDefinitions(void);


        /** increments the multipass number entry by 1 if it exists
         */
        void incPassIterationNumber(void);
        /// @deprecated query by GPV_PASS_ITERATION_NUMBER instead
        OGRE_DEPRECATED bool hasPassIterationNumber() const
        { return mActivePassIterationIndex != (std::numeric_limits<size_t>::max)(); }
        /// @deprecated query by GPV_PASS_ITERATION_NUMBER instead
        OGRE_DEPRECATED size_t getPassIterationNumberIndex() const
        { return mActivePassIterationIndex; }

        /// @name Shared Parameters
        /// @{
        /** Use a set of shared parameters in this parameters object.
            @remarks
            Allows you to use a set of shared parameters to automatically update
            this parameter set.
        */
        void addSharedParameters(GpuSharedParametersPtr sharedParams);

        /** Use a set of shared parameters in this parameters object.
            @remarks
            Allows you to use a set of shared parameters to automatically update
            this parameter set.
            @param sharedParamsName The name of a shared parameter set as defined in
            GpuProgramManager
        */
        void addSharedParameters(const String& sharedParamsName);

        /** Returns whether this parameter set is using the named shared parameter set. */
        bool isUsingSharedParameters(const String& sharedParamsName) const;

        /** Stop using the named shared parameter set. */
        void removeSharedParameters(const String& sharedParamsName);

        /** Stop using all shared parameter sets. */
        void removeAllSharedParameters();

        /** Get the list of shared parameter sets. */
        const GpuSharedParamUsageList& getSharedParameters() const;

        /** Update the parameters by copying the data from the shared
            parameters.
            @note This method  may not actually be called if the RenderSystem
            supports using shared parameters directly in their own shared buffer; in
            which case the values should not be copied out of the shared area
            into the individual parameter set, but bound separately.
        */
        void _copySharedParams();

        /** Update the HardwareBuffer based backing of referenced shared parameters
         *
         * falls back to _copySharedParams() if a shared parameter is not hardware backed
         */
        void _updateSharedParams();
        /// @}

        /** Internal method that the RenderSystem might use to store optional data. */
        void _setRenderSystemData(const Any& data) const { mRenderSystemData = data; }
        /** Internal method that the RenderSystem might use to store optional data. */
        const Any& _getRenderSystemData() const { return mRenderSystemData; }

        size_t calculateSize(void) const;

        /** Set subroutine name by slot name
         */
        void setNamedSubroutine(const String& subroutineSlot, const String& subroutine);

        /** Set subroutine name by slot index
         */
        void setSubroutine(size_t index, const String& subroutine);

        /** Get map with
         */
        const SubroutineMap& getSubroutineMap() const { return mSubroutineMap; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
