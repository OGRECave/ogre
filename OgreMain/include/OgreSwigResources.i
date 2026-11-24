// Ogre SWIG bindings: pixel formats, images, hardware buffers, vertex data, resources, textures, gpu programs.
// Part of Ogre.i. The python bindings build each part as a separate module, see Components/Python.

ADD_REPR(ColourValue)
%ignore Ogre::PixelUtil::unpackColour(ColourValue*, PixelFormat, const void*);
%include "OgrePixelFormat.h"
%ignore Ogre::GpuProgramParameters::hasPassIterationNumber; // deprecated
%ignore Ogre::GpuProgramParameters::getPassIterationNumberIndex; // deprecated
%ignore Ogre::GpuProgramParameters::setConstantFromTime;
%ignore Ogre::GpuProgramParameters::getConstantDefinitionIterator;
%ignore Ogre::GpuSharedParameters::getConstantDefinitionIterator;
%template(GpuConstantDefinitionMap) std::map<std::string, Ogre::GpuConstantDefinition>;
SHARED_PTR(GpuProgramParameters);
SHARED_PTR(GpuSharedParameters);
namespace Ogre { struct GpuLogicalIndexUse; class GpuSharedParametersUsage; }
%template(ConstantList) std::vector<Ogre::uchar>;
%template(GpuLogicalIndexUseMap) std::map<size_t, Ogre::GpuLogicalIndexUse>;
%ignore std::vector<Ogre::GpuSharedParametersUsage>::resize; // non default constructible
%ignore std::vector<Ogre::GpuSharedParametersUsage>::vector(size_type);
%template(GpuSharedParamUsageList) std::vector<Ogre::GpuSharedParametersUsage>;
%template(GpuConstantDefinitionPair) std::pair<std::string, Ogre::GpuConstantDefinition>;
%template(GpuConstantDefinitionList) std::vector<std::pair<std::string, Ogre::GpuConstantDefinition> >;
%include "OgreGpuProgramParams.h"
%template(setNamedConstant) Ogre::GpuSharedParameters::setNamedConstant<float>;
%template(setNamedConstant) Ogre::GpuSharedParameters::setNamedConstant<int>;
%template(setNamedConstant) Ogre::GpuSharedParameters::setNamedConstant<Ogre::uint>;
%template(setNamedConstant) Ogre::GpuSharedParameters::setNamedConstant<2, Ogre::Real>;
%template(setNamedConstant) Ogre::GpuSharedParameters::setNamedConstant<3, Ogre::Real>;
%template(setNamedConstant) Ogre::GpuSharedParameters::setNamedConstant<4, Ogre::Real>;
%ignore Ogre::Image::getFileExtFromMagic;
%include "OgreImage.h"
%apply unsigned int* OUTPUT { unsigned int* result };
%ignore Ogre::HardwareOcclusionQuery::getLastQuerysPixelcount; // deprecated
%include "OgreHardwareOcclusionQuery.h"
SHARED_PTR(HardwareBuffer);
%include "OgreHardwareBuffer.h"
    SHARED_PTR(Resource);
    %include "OgreResource.h"
        SHARED_PTR(Texture);
        %ignore Ogre::Texture::setTreatLuminanceAsAlpha;

        #ifdef SWIGCSHARP
        // correct out IntPtr for void* pData output parameter
        %typemap(imtype, out="global::System.IntPtr") void *pData "out global::System.IntPtr"
        %typemap(cstype, out="$csclassname") void *pData "out global::System.IntPtr"
        %typemap(csin) void *pData "out $csinput"
        %typecheck(SWIG_TYPECHECK_INT64_PTR) void *pData ""
        #endif

        %template(ConstImagePtrList) std::vector<const Ogre::Image*>;
        %template(TextureList) std::vector<Ogre::TexturePtr>;
        %include "OgreTexture.h"
        SHARED_PTR(GpuProgram);
        %ignore Ogre::GpuProgram::setAdjacencyInfoRequired;
        %include "OgreGpuProgram.h"
            SHARED_PTR(HighLevelGpuProgram);
            %include "OgreHighLevelGpuProgram.h"
%template(StringSet) std::set<std::string>;
%include "OgreRenderSystemCapabilities.h"
%ignore getPropertyName;
%template(BoolStringPair) std::pair<bool, std::string>;
%include "OgreScriptCompiler.h"
SHARED_PTR(HardwareVertexBuffer);
#ifdef SWIGPYTHON
%template(VertexElementList) std::list<Ogre::VertexElement>;
#endif
%ignore Ogre::VertexElement::getBestColourVertexElementType;
%template(VertexBufferBindingMap) std::map<unsigned short, Ogre::SharedPtr<Ogre::HardwareVertexBuffer> >; // not the typedef: breaks Java iterators
%template(BindingIndexMap) std::map<unsigned short, unsigned short>;
%include "OgreHardwareVertexBuffer.h"
SHARED_PTR(HardwareIndexBuffer);
%include "OgreHardwareIndexBuffer.h"
SHARED_PTR(HardwarePixelBuffer);
#ifdef SWIGJAVA
%ignore Ogre::HardwarePixelBuffer::lock;  // duplicate definition
#endif
%include "OgreHardwarePixelBuffer.h"
%ignore Ogre::HardwareBufferManagerBase::_forceReleaseBufferCopies(HardwareVertexBuffer* sourceBuffer);
%include "OgreHardwareBufferManager.h"
%include "OgreDefaultHardwareBufferManager.h"
%include "OgreVertexIndexData.h"
// managers
%ignore Ogre::ResourceGroupManager::openResource(const String&, const String&, bool, Resource*) const;
%ignore Ogre::ResourceGroupManager::openResource(const String&, const String&, bool) const;
%ignore Ogre::ResourceGroupManager::getResourceManagerIterator;
#ifndef SWIGJAVA // older std_list.i clashes with JDK 21 List.removeFirst
%template(DataStreamList) std::list<Ogre::SharedPtr<Ogre::DataStream> >; // not the typedef: breaks Java iterators
#endif
PTR_TRAITS(Ogre::ResourceManager)
%template(ResourceManagerMap) std::map<std::string, Ogre::ResourceManager*>;

%include "OgreResourceGroupManager.h"
#if SWIG_VERSION < 0x030012 || defined(SWIGJAVA)
// conflicts with overloaded versions (GpuProgram, Texture)
%ignore Ogre::ResourceManager::getResourceByName;
%ignore Ogre::ResourceManager::createOrRetrieve;
%ignore Ogre::ResourceManager::prepare;
%ignore Ogre::ResourceManager::load;
#endif
%ignore Ogre::ResourceManager::getResourceIterator;
%template(ResourceCreateOrRetrieveResult) std::pair<Ogre::ResourcePtr, bool>;
%include "OgreResourceManager.h"
    %ignore Ogre::GpuProgramManager::getByName(const String&, const String&,bool) const;
    %include "OgreGpuProgramManager.h"
    %include "OgreHighLevelGpuProgramManager.h"
%include "OgreRenderOperation.h"
