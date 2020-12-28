/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-present Torus Knot Software Ltd

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
#ifndef _OgreVulkanRootLayout_H_
#define _OgreVulkanRootLayout_H_

#include "OgreVulkanPrerequisites.h"

#include "OgreRootLayout.h"
#include "OgreVulkanProgram.h"

struct VkDescriptorSetLayoutBinding;
struct VkWriteDescriptorSet;

// Forward declaration for |Document|.
namespace rapidjson
{
    class CrtAllocator;
    template <typename>
    class MemoryPoolAllocator;
    template <typename>
    struct UTF8;

    template <typename BaseAllocator>
    class MemoryPoolAllocator;
    template <typename Encoding, typename>
    class GenericValue;
    typedef GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator> > Value;
}  // namespace rapidjson

namespace Ogre
{
    uint32 toVkDescriptorType( DescBindingTypes::DescBindingTypes type );

    class _OgreVulkanExport VulkanRootLayout : protected RootLayout, public ResourceAlloc
    {
        /// One handle per binding set (up to OGRE_VULKAN_MAX_NUM_BOUND_DESCRIPTOR_SETS)
        /// Doesn't have gaps (e.g. if mDescBindingRanges[3] is not empty, then mSets[3] must exist)
        /// Won't be initialized if this VulkanRootLayout never makes it into a PSO.
        FastArray<VkDescriptorSetLayout> mSets;

        /// The Vulkan Handle to our root layout. Won't be initialized if this
        /// VulkanRootLayout never makes it into a PSO.
        VkPipelineLayout mRootLayout;

        /// There's one VulkanDescriptorPool per binding set
        FastArray<VulkanDescriptorPool *> mPools;

        VulkanGpuProgramManager *mProgramManager;

        inline void bindCommon( VkWriteDescriptorSet &writeDescSet, size_t &numWriteDescSets,
                                uint32 &currBinding, VkDescriptorSet descSet,
                                const DescBindingRange &bindRanges );
        inline void bindParamsBuffer( VkWriteDescriptorSet *writeDescSets, size_t &numWriteDescSets,
                                      uint32 &currBinding, VkDescriptorSet descSet,
                                      const DescBindingRange *descBindingRanges,
                                      const VulkanGlobalBindingTable &table );
        inline void bindConstBuffers( VkWriteDescriptorSet *writeDescSets, size_t &numWriteDescSets,
                                      uint32 &currBinding, VkDescriptorSet descSet,
                                      const DescBindingRange *descBindingRanges,
                                      const VulkanGlobalBindingTable &table );
        inline void bindReadOnlyBuffers( VkWriteDescriptorSet *writeDescSets, size_t &numWriteDescSets,
                                         uint32 &currBinding, VkDescriptorSet descSet,
                                         const DescBindingRange *descBindingRanges,
                                         const VulkanGlobalBindingTable &table );
        inline void bindTexBuffers( VkWriteDescriptorSet *writeDescSets, size_t &numWriteDescSets,
                                    uint32 &currBinding, VkDescriptorSet descSet,
                                    const DescBindingRange *descBindingRanges,
                                    const VulkanGlobalBindingTable &table );
        inline void bindTextures( VkWriteDescriptorSet *writeDescSets, size_t &numWriteDescSets,
                                  uint32 &currBinding, VkDescriptorSet descSet,
                                  const DescBindingRange *descBindingRanges,
                                  const VulkanGlobalBindingTable &table );
        inline void bindSamplers( VkWriteDescriptorSet *writeDescSets, size_t &numWriteDescSets,
                                  uint32 &currBinding, VkDescriptorSet descSet,
                                  const DescBindingRange *descBindingRanges,
                                  const VulkanGlobalBindingTable &table );

        uint32 calculateFirstDirtySet( const VulkanGlobalBindingTable &table ) const;

    public:
        VulkanRootLayout( VulkanGpuProgramManager *programManager );
        ~VulkanRootLayout();

        using RootLayout::dump;
        using RootLayout::findParamsBuffer;
        using RootLayout::getDescBindingRanges;

        /// @copydoc VulkanRootLayout::copyFrom
        void copyFrom( const RootLayout &rootLayout );

        /// @copydoc VulkanRootLayout::parseRootLayout
        void parseRootLayout( const char *rootLayout, const bool bCompute, const String &filename );

        /** Generates all the macros for compiling shaders, based on our layout

            e.g. a layout like this:

            @code
                ## ROOT LAYOUT BEGIN
                {
                    "0" :
                    {
                        "has_params" : true
                        "const_buffers" : [2, 4]
                        "tex_buffers" : [0, 1]
                        "samplers" : [1, 2],
                        "textures" : [1, 2]
                    }
                }
                ## ROOT LAYOUT END
            @endcode

            will generate the following:
            @code
                #define ogre_P0 set = 0, binding = 0 // Params buffer
                #define ogre_B2 set = 0, binding = 1 // Const buffer at slot 2
                #define ogre_B3 set = 0, binding = 2
                #define ogre_T0 set = 0, binding = 3 // Tex buffer at slot 0
                #define ogre_t1 set = 0, binding = 4 // Texture at slot 1
                                                     // (other APIs share tex buffer & texture slots)
                #define ogre_s1 set = 0, binding = 5 // Sampler at slot 1
            @endcode
        @param shaderStage
            See GpuProgramType
        @param inOutString [in/out]
            String to output our macros
        */
        void generateRootLayoutMacros( uint32 shaderStage, ShaderSyntax shaderType,
                                       String &inOutString ) const;

        /** Creates most of our Vulkan handles required to build a PSO.

            This function is not called by parseRootLayout because if two Root Layouts are identical,
            then after calling a->parseRootLayout(), instead of calling a->createVulkanHandles()
            we first must look for an already existing VulkanRootLayout and reuse it.
        @return
            VkPipelineLayout handle for building the PSO.
        */
        VkPipelineLayout createVulkanHandles( void );

        /** Takes an emulated D3D11/Metal-style table and binds it according to this layout's rules

            This updates N descriptors (1 for each set) and binds them
        @param device
        @param vaoManager
            The VaoManager so we can grab new VulkanDescriptorPools shall we need them
        @param table
            The emulated table to bind it
        */
        void bind( VulkanDevice *device, VulkanVaoManager *vaoManager,
                   const VulkanGlobalBindingTable &table );

        /// Two root layouts can be incompatible. If so, we return nullptr
        ///
        /// If a and b are compatible, we return the best one (one that can satisfy both a and b).
        ///
        /// a or b can be nullptr
        static VulkanRootLayout *findBest( VulkanRootLayout *a, VulkanRootLayout *b );

        const DescBindingRange *getDescBindingRanges( size_t setIdx ) const
        {
            return mDescBindingRanges[setIdx];
        }

        bool operator<( const VulkanRootLayout &other ) const;
    };
}  // namespace Ogre

#endif  // __VulkanProgram_H__
