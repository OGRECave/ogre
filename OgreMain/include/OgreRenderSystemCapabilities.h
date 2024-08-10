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
#ifndef __RenderSystemCapabilities__
#define __RenderSystemCapabilities__

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreStringVector.h"
#include "OgreStringConverter.h"
#include "OgreHeaderPrefix.h"
#include "OgreGpuProgram.h"

// Because there are more than 32 possible Capabilities, more than 1 int is needed to store them all.
// In fact, an array of integers is used to store capabilities. However all the capabilities are defined in the single
// enum. The only way to know which capabilities should be stored where in the array is to use some of the 32 bits
// to record the category of the capability.  These top few bits are used as an index into mCapabilities array
// The lower bits are used to identify each capability individually by setting 1 bit for each

// Identifies how many bits are reserved for categories
// NOTE: Although 4 bits (currently) are enough
#define CAPS_CATEGORY_SIZE 4
#define OGRE_CAPS_BITSHIFT (32 - CAPS_CATEGORY_SIZE)
#define CAPS_CATEGORY_MASK (((1 << CAPS_CATEGORY_SIZE) - 1) << OGRE_CAPS_BITSHIFT)
#define OGRE_CAPS_VALUE(cat, val) ((cat << OGRE_CAPS_BITSHIFT) | (1 << val))

namespace Ogre 
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */

    /// Enumerates the categories of capabilities
    enum CapabilitiesCategory
    {
        CAPS_CATEGORY_COMMON = 0,
        CAPS_CATEGORY_COMMON_2 = 1,
        CAPS_CATEGORY_D3D9 = 2,
        CAPS_CATEGORY_GL = 3,
        CAPS_CATEGORY_COMMON_3 = 4,
        /// Placeholder for max value
        CAPS_CATEGORY_COUNT = 5
    };

    /// Enum describing the different hardware capabilities we want to check for
    /// OGRE_CAPS_VALUE(a, b) defines each capability
    /// a is the category (which can be from 0 to 15)
    /// b is the value (from 0 to 27)
    enum Capabilities
    {
        /// specifying a "-1" in the index buffer starts a new draw command.
        RSC_PRIMITIVE_RESTART = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 0),
        /// GL ES2/ES3 does not support generating mipmaps for compressed formats in hardware
        RSC_AUTOMIPMAP_COMPRESSED = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 1),
        /// Supports anisotropic texture filtering
        RSC_ANISOTROPY              = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 2),
        /// Supports depth clamping
        RSC_DEPTH_CLAMP             = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 3),
        /// Supports linewidth != 1.0
        RSC_WIDE_LINES              = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 4),
        /// Supports hardware stencil buffer
        RSC_HWSTENCIL               = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 5),
        /// Supports read/write buffers with atomic counters (e.g. RWStructuredBuffer or SSBO)
        RSC_READ_WRITE_BUFFERS      = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 6),
        /// Supports compressed textures in the ASTC format
        RSC_TEXTURE_COMPRESSION_ASTC = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 7),
        /// Supports 32bit hardware index buffers
        RSC_32BIT_INDEX             = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 8),
        /// Supports vertex programs (vertex shaders)
        /// @deprecated All targeted APIs by Ogre support this feature
        RSC_VERTEX_PROGRAM          = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 9),
        /// Supports tessellation domain and hull programs
        RSC_TESSELLATION_PROGRAM = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 10),
        /// Supports 2D Texture Arrays
        RSC_TEXTURE_2D_ARRAY        = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 11),
        /// Supports separate stencil updates for both front and back faces
        RSC_TWO_SIDED_STENCIL       = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 12),
        /// Supports wrapping the stencil value at the range extremeties
        RSC_STENCIL_WRAP            = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 13),
        /// Supports hardware occlusion queries
        RSC_HWOCCLUSION             = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 14),
        /// Supports user clipping planes
        RSC_USER_CLIP_PLANES        = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 15),
        /// Supports hardware compute programs
        RSC_COMPUTE_PROGRAM         = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 16),
        /// Supports 1d textures
        RSC_TEXTURE_1D              = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 17),
        /// Supports hardware render-to-texture (bigger than framebuffer)
        RSC_HWRENDER_TO_TEXTURE     = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 18),
        /// Supports float textures and render targets
        RSC_TEXTURE_FLOAT           = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 19),
        /// Supports non-power of two textures
        RSC_NON_POWER_OF_2_TEXTURES = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 20),
        /// Supports 3d (volume) textures
        RSC_TEXTURE_3D              = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 21),
        /// Supports basic point sprite rendering
        RSC_POINT_SPRITES           = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 22),
        /// @deprecated same as RSC_POINT_SPRITES
        RSC_POINT_EXTENDED_PARAMETERS = RSC_POINT_SPRITES,
        /// Supports rendering to vertex buffers
        RSC_HWRENDER_TO_VERTEX_BUFFER = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 23),
        /// Supports vertex texture fetch
        RSC_VERTEX_TEXTURE_FETCH = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 24),
        /// Supports mipmap LOD biasing
        RSC_MIPMAP_LOD_BIAS = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 25),
        /// Supports hardware geometry programs
        RSC_GEOMETRY_PROGRAM = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 26),
        /// Supports unaligned #VET_HALF3, #VET_SHORT3, #VET_USHORT3 formats
        RSC_VERTEX_FORMAT_16X3 = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON, 27),

        /// Supports compressed textures
        RSC_TEXTURE_COMPRESSION = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 0),
        /// Supports compressed textures in the DXT/ST3C formats
        RSC_TEXTURE_COMPRESSION_DXT = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 1),
        /// Supports compressed textures in the VTC format
        RSC_TEXTURE_COMPRESSION_VTC = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 2),
        /// Supports compressed textures in the PVRTC format
        RSC_TEXTURE_COMPRESSION_PVRTC = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 3),
        /// Supports compressed textures in the ATC format
        RSC_TEXTURE_COMPRESSION_ATC = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 4),
        /// Supports compressed textures in the ETC1 format
        RSC_TEXTURE_COMPRESSION_ETC1 = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 5),
        /// Supports compressed textures in the ETC2 format
        RSC_TEXTURE_COMPRESSION_ETC2 = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 6),
        /// Supports compressed textures in BC4 and BC5 format (DirectX feature level 10_0)
        RSC_TEXTURE_COMPRESSION_BC4_BC5 = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 7),
        /// Supports compressed textures in BC6H and BC7 format (DirectX feature level 11_0)
        RSC_TEXTURE_COMPRESSION_BC6H_BC7 = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 8),
        /// Supports fixed-function pipeline
        RSC_FIXED_FUNCTION = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 9),
        /// Supports #VET_INT_10_10_10_2_NORM
        RSC_VERTEX_FORMAT_INT_10_10_10_2 = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 10),
        /// Supports Alpha to Coverage (A2C)
        RSC_ALPHA_TO_COVERAGE = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 11),
        /// Supports reading back compiled shaders
        RSC_CAN_GET_COMPILED_SHADER_BUFFER = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 12),
        /// Supports HW gamma, both in the framebuffer and as texture.
        RSC_HW_GAMMA = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 13),
        /// Supports using the MAIN depth buffer for RTTs. D3D 9&10, OGL w/FBO support unknown
        /// (undefined behavior?), OGL w/ copy supports it
        RSC_RTT_MAIN_DEPTHBUFFER_ATTACHABLE = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 14),
        /// Supports attaching a depth buffer to an RTT that has width & height less or equal than RTT's.
        /// Otherwise must be of _exact_ same resolution. D3D 9, OGL 3.0 (not 2.0, not D3D10)
        RSC_RTT_DEPTHBUFFER_RESOLUTION_LESSEQUAL = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 15),
        /// Supports using vertex buffers for instance data
        RSC_VERTEX_BUFFER_INSTANCE_DATA = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 16),
        /// Supports mesh and task programs
        RSC_MESH_PROGRAM = OGRE_CAPS_VALUE(CAPS_CATEGORY_COMMON_2, 17),

        // ***** DirectX specific caps *****
        /// Is DirectX feature "per stage constants" supported
        RSC_PERSTAGECONSTANT = OGRE_CAPS_VALUE(CAPS_CATEGORY_D3D9, 0),
        /// D3D11: supports reading back the inactive depth-stencil buffer as texture
        RSC_READ_BACK_AS_TEXTURE = OGRE_CAPS_VALUE(CAPS_CATEGORY_D3D9, 1),
        /// the renderer will try to use W-buffers when available
        /// W-buffers are enabled by default for 16bit depth buffers and disabled for all other
        /// depths.
        RSC_WBUFFER              = OGRE_CAPS_VALUE(CAPS_CATEGORY_D3D9, 2),
        /// D3D11: Supports asynchronous hardware occlusion queries
        RSC_HWOCCLUSION_ASYNCHRONOUS = OGRE_CAPS_VALUE(CAPS_CATEGORY_D3D9, 3),
        RSC_HWRENDER_TO_TEXTURE_3D = OGRE_CAPS_VALUE(CAPS_CATEGORY_D3D9, 4),
        /// All MRTs must have same bit depths
        RSC_MRT_SAME_BIT_DEPTHS = OGRE_CAPS_VALUE(CAPS_CATEGORY_D3D9, 5),

        // ***** GL Specific Caps *****
        /// Support for PBuffer
        RSC_PBUFFER          = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 0),
        /// Support for Separate Shader Objects
        RSC_SEPARATE_SHADER_OBJECTS = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 1),
        /// Support for Vertex Array Objects (VAOs)
        RSC_VAO              = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 2),
        /// with Separate Shader Objects the gl_PerVertex interface block must be redeclared
        /// but some drivers misbehave and do not compile if we do so
        RSC_GLSL_SSO_REDECLARE = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 3),
        /// Supports debugging/ profiling events
        RSC_DEBUG = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 4),
        /// RS can map driver buffer storage directly instead of using a shadow buffer
        RSC_MAPBUFFER = OGRE_CAPS_VALUE(CAPS_CATEGORY_GL, 5),

        // deprecated caps, all aliasing to RSC_VERTEX_PROGRAM
        /// @deprecated assume present
        RSC_INFINITE_FAR_PLANE = RSC_VERTEX_PROGRAM,
        /// @deprecated assume present
        RSC_FRAGMENT_PROGRAM = RSC_VERTEX_PROGRAM,
        RSC_TESSELLATION_DOMAIN_PROGRAM = RSC_TESSELLATION_PROGRAM,
        RSC_TESSELLATION_HULL_PROGRAM = RSC_TESSELLATION_PROGRAM
    };

    /// DriverVersion is used by RenderSystemCapabilities and both GL and D3D9
    /// to store the version of the current GPU driver
    struct _OgreExport DriverVersion 
    {
        int major;
        int minor;
        int release;
        int build;

        DriverVersion() 
        {
            major = minor = release = build = 0;
        }

        String toString() const;
        void fromString(const String& versionString);
    };

    /** Enumeration of GPU vendors. */
    enum GPUVendor
    {
        GPU_UNKNOWN = 0,
        GPU_NVIDIA,
        GPU_AMD,
        GPU_INTEL,
        GPU_IMAGINATION_TECHNOLOGIES,
        GPU_APPLE,  //!< Apple Software Renderer
        GPU_NOKIA,
        GPU_MS_SOFTWARE, //!< Microsoft software device
        GPU_MS_WARP, //!< Microsoft WARP (Windows Advanced Rasterization Platform) software device - http://msdn.microsoft.com/en-us/library/dd285359.aspx
        GPU_ARM, //!< For the Mali chipsets
        GPU_QUALCOMM,
        GPU_MOZILLA, //!<  WebGL on Mozilla/Firefox based browser
        GPU_WEBKIT, //!< WebGL on WebKit/Chrome base browser
        /// placeholder
        GPU_VENDOR_COUNT
    };

    /** This class stores the capabilities of the graphics card.

    This information is set by the individual render systems.
    */
    class _OgreExport RenderSystemCapabilities : public RenderSysAlloc
    {

    public:

        typedef std::set<String> ShaderProfiles;
    private:
        /// This is used to build a database of RSC's
        /// if a RSC with same name, but newer version is introduced, the older one 
        /// will be removed
        DriverVersion mDriverVersion;
        /// GPU Vendor
        GPUVendor mVendor;

        static String msGPUVendorStrings[GPU_VENDOR_COUNT];
        static void initVendorStrings();

        /// The number of texture units available
        ushort mNumTextureUnits;
        /// The stencil buffer bit depth
        ushort mStencilBufferBitDepth;
        /// Stores the capabilities flags.
        int mCapabilities[CAPS_CATEGORY_COUNT];
        /// Which categories are relevant
        bool mCategoryRelevant[CAPS_CATEGORY_COUNT];
        /// The name of the device as reported by the render system
        String mDeviceName;
        /// The identifier associated with the render system for which these capabilities are valid
        String mRenderSystemName;

        /// The number of floating-point 4-vector constants
        ushort mConstantFloatCount[GPT_COUNT];
        /// The number of simultaneous render targets supported
        ushort mNumMultiRenderTargets;
        /// The maximum point size
        Real mMaxPointSize;
        /// Are non-POW2 textures feature-limited?
        bool mNonPOW2TexturesLimited;
        /// The maximum supported anisotropy
        Real mMaxSupportedAnisotropy;
        /// The number of vertex texture units supported
        ushort mNumVertexTextureUnits;
        /// The number of vertices a geometry program can emit in a single run
        int mGeometryProgramNumOutputVertices;

        /// The list of supported shader profiles
        ShaderProfiles mSupportedShaderProfiles;

        /// The number of vertex attributes available
        ushort mNumVertexAttributes;
    public: 
        RenderSystemCapabilities ();

        /** Set the driver version. */
        void setDriverVersion(const DriverVersion& version)
        {
            mDriverVersion = version;
        }

        void parseDriverVersionFromString(const String& versionString)
        {
            DriverVersion version;
            version.fromString(versionString);
            setDriverVersion(version);
        }


        DriverVersion getDriverVersion() const
        {
            return mDriverVersion;
        }

        GPUVendor getVendor() const
        {
            return mVendor;
        }

        void setVendor(GPUVendor v)
        {
            mVendor = v;
        }

        /// Parse and set vendor
        void parseVendorFromString(const String& vendorString)
        {
            setVendor(vendorFromString(vendorString));
        }

        /// Convert a vendor string to an enum
        static GPUVendor vendorFromString(const String& vendorString);
        /// Convert a vendor enum to a string
        static const String& vendorToString(GPUVendor v);

        bool isDriverOlderThanVersion(const DriverVersion &v) const
        {
            if (mDriverVersion.major < v.major)
                return true;
            else if (mDriverVersion.major == v.major && 
                mDriverVersion.minor < v.minor)
                return true;
            else if (mDriverVersion.major == v.major && 
                mDriverVersion.minor == v.minor && 
                mDriverVersion.release < v.release)
                return true;
            else if (mDriverVersion.major == v.major && 
                mDriverVersion.minor == v.minor && 
                mDriverVersion.release == v.release &&
                mDriverVersion.build < v.build)
                return true;
            return false;
        }

        void setNumTextureUnits(ushort num)
        {
            mNumTextureUnits = num;
        }

        /// @deprecated do not use
        void setStencilBufferBitDepth(ushort num)
        {
            mStencilBufferBitDepth = num;
        }

        /// The number of simultaneous render targets supported
        void setNumMultiRenderTargets(ushort num)
        {
            mNumMultiRenderTargets = num;
        }

        void setNumVertexAttributes(ushort num)
        {
            mNumVertexAttributes = num;
        }

        ushort getNumVertexAttributes(void) const
        {
            return mNumVertexAttributes;
        }

        /** Returns the number of texture units the current output hardware
        supports.

        For use in rendering, this determines how many texture units the
        are available for multitexturing (i.e. rendering multiple 
        textures in a single pass). Where a Material has multiple 
        texture layers, it will try to use multitexturing where 
        available, and where it is not available, will perform multipass
        rendering to achieve the same effect. This property only applies
        to the fixed-function pipeline, the number available to the 
        programmable pipeline depends on the shader model in use.
        */
        ushort getNumTextureUnits(void) const
        {
            return mNumTextureUnits;
        }

        /// @deprecated assume 8-bit stencil buffer
        ushort getStencilBufferBitDepth(void) const
        {
            return mStencilBufferBitDepth;
        }

        /// The number of simultaneous render targets supported
        ushort getNumMultiRenderTargets(void) const
        {
            return mNumMultiRenderTargets;
        }

        /** Returns true if capability is render system specific
        */
        bool isCapabilityRenderSystemSpecific(const Capabilities c) const
        {
            int cat = c >> OGRE_CAPS_BITSHIFT;
            if(cat == CAPS_CATEGORY_GL || cat == CAPS_CATEGORY_D3D9)
                return true;
            return false;
        }

        /** Adds a capability flag
        */
        void setCapability(const Capabilities c) 
        { 
            int index = (CAPS_CATEGORY_MASK & c) >> OGRE_CAPS_BITSHIFT;
            // zero out the index from the stored capability
            mCapabilities[index] |= (c & ~CAPS_CATEGORY_MASK);
        }

        /** Remove a capability flag
        */
        void unsetCapability(const Capabilities c) 
        { 
            int index = (CAPS_CATEGORY_MASK & c) >> OGRE_CAPS_BITSHIFT;
            // zero out the index from the stored capability
            mCapabilities[index] &= (~c | CAPS_CATEGORY_MASK);
        }

        /** Checks for a capability
        */
        bool hasCapability(const Capabilities c) const
        {
            int index = (CAPS_CATEGORY_MASK & c) >> OGRE_CAPS_BITSHIFT;
            // test against
            if(mCapabilities[index] & (c & ~CAPS_CATEGORY_MASK))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        /** Adds the profile to the list of supported profiles
        */
        void addShaderProfile(const String& profile);

        /** Remove a given shader profile, if present.
        */
        void removeShaderProfile(const String& profile);

        /** Returns true if profile is in the list of supported profiles
        */
        bool isShaderProfileSupported(const String& profile) const;

        /** Returns a set of all supported shader profiles
        * */
        const ShaderProfiles& getSupportedShaderProfiles() const
        {
            return mSupportedShaderProfiles;
        }


        /// The number of floating-point 4-vector constants vertex programs support
        ushort getConstantFloatCount(GpuProgramType programType) const
        {
            return mConstantFloatCount[programType];
        }

        /// sets the device name for Render system
        void setDeviceName(const String& name)
        {
            mDeviceName = name;
        }

        /// gets the device name for render system
        String getDeviceName() const
        {
            return mDeviceName;
        }

        /// The number of floating-point 4-vector constants vertex programs support
        void setVertexProgramConstantFloatCount(ushort c)
        {
            mConstantFloatCount[GPT_VERTEX_PROGRAM] = c;
        }
        /// The number of floating-point 4-vector constants geometry programs support
        void setGeometryProgramConstantFloatCount(ushort c)
        {
            mConstantFloatCount[GPT_GEOMETRY_PROGRAM] = c;
        }
        /// The number of floating-point 4-vector constants fragment programs support
        void setFragmentProgramConstantFloatCount(ushort c)
        {
            mConstantFloatCount[GPT_FRAGMENT_PROGRAM] = c;
        }

        /// Maximum point screen size in pixels
        void setMaxPointSize(Real s)
        {
            mMaxPointSize = s;
        }
        /// Maximum point screen size in pixels
        Real getMaxPointSize(void) const
        {
            return mMaxPointSize;
        }
        /// Non-POW2 textures limited
        void setNonPOW2TexturesLimited(bool l)
        {
            mNonPOW2TexturesLimited = l;
        }
        /** Are non-power of two textures limited in features?

        If the RSC_NON_POWER_OF_2_TEXTURES capability is set, but this
        method returns true, you can use non power of 2 textures only if:
        <ul><li>You load them explicitly with no mip maps</li>
        <li>You don't use DXT texture compression</li>
        <li>You use clamp texture addressing</li></ul>
        */
        bool getNonPOW2TexturesLimited(void) const
        {
            return mNonPOW2TexturesLimited;
        }
        /// Set the maximum supported anisotropic filtering
        void setMaxSupportedAnisotropy(Real s)
        {
            mMaxSupportedAnisotropy = s;
        }
        /// Get the maximum supported anisotropic filtering
        Real getMaxSupportedAnisotropy() const
        {
            return mMaxSupportedAnisotropy;
        }

        /// Set the number of vertex texture units supported
        void setNumVertexTextureUnits(ushort n)
        {
            mNumVertexTextureUnits = n;
        }
        /// Get the number of vertex texture units supported
        ushort getNumVertexTextureUnits(void) const
        {
            return mNumVertexTextureUnits;
        }

        /// Set the number of vertices a single geometry program run can emit
        void setGeometryProgramNumOutputVertices(int numOutputVertices)
        {
            mGeometryProgramNumOutputVertices = numOutputVertices;
        }
        /// Get the number of vertices a single geometry program run can emit
        int getGeometryProgramNumOutputVertices(void) const
        {
            return mGeometryProgramNumOutputVertices;
        }

        /// Get the identifier of the rendersystem from which these capabilities were generated
        const String& getRenderSystemName(void) const
        {
            return mRenderSystemName;
        }
        ///  Set the identifier of the rendersystem from which these capabilities were generated
        void setRenderSystemName(const String& rs)
        {
            mRenderSystemName = rs;
        }

        /// Mark a category as 'relevant' or not, ie will it be reported
        void setCategoryRelevant(CapabilitiesCategory cat, bool relevant)
        {
            mCategoryRelevant[cat] = relevant;
        }

        /// Return whether a category is 'relevant' or not, ie will it be reported
        bool isCategoryRelevant(CapabilitiesCategory cat)
        {
            return mCategoryRelevant[cat];
        }



        /** Write the capabilities to the pass in Log */
        void log(Log* pLog) const;

        /// The number of floating-point 4-vector constants compute programs support
        void setComputeProgramConstantFloatCount(ushort c)
        {
            mConstantFloatCount[GPT_COMPUTE_PROGRAM] = c;
        }
        /// The number of floating-point 4-vector constants tessellation Domain programs support
        void setTessellationDomainProgramConstantFloatCount(ushort c)
        {
            mConstantFloatCount[GPT_DOMAIN_PROGRAM] = c;
        }
        /// The number of floating-point 4-vector constants tessellation Hull programs support
        void setTessellationHullProgramConstantFloatCount(ushort c)
        {
            mConstantFloatCount[GPT_HULL_PROGRAM] = c;
        }

    };

    inline String to_string(GPUVendor v) { return RenderSystemCapabilities::vendorToString(v); }
    inline String to_string(const DriverVersion& v) { return v.toString(); }

    /** @} */
    /** @} */
} // namespace


#include "OgreHeaderSuffix.h"

#endif // __RenderSystemCapabilities__

