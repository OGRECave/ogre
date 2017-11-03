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

#ifndef __CompositorTextureDefinitionBase_H__
#define __CompositorTextureDefinitionBase_H__

#include "OgreHeaderPrefix.h"
#include "Compositor/OgreCompositorCommon.h"
#include "Compositor/OgreCompositorChannel.h"
#include "Compositor/OgreCompositorNamedBuffer.h"
#include "OgreIdString.h"
#include "OgreId.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    /** Centralized class for dealing with declarations of textures in Node &
        Workspace definitions. Note that shadow nodes use their own system
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    class _OgreExport TextureDefinitionBase : public CompositorInstAlloc
    {
    public:
        enum TextureSource
        {
            TEXTURE_INPUT,      /// We got the texture through an input channel
            TEXTURE_LOCAL,      /// We own the texture
            TEXTURE_GLOBAL,     /// It's a global texture. Ask the manager for it.
            NUM_TEXTURES_SOURCES
        };

        enum BoolSetting
        {
            BoolFalse,
            BoolTrue,
            BoolUndefined,
        };

        /// Local texture definition
        class _OgreExport TextureDefinition : public CompositorInstAlloc
        {
            IdString name;
        public:
            TextureType textureType;
            uint width;       // 0 means adapt to target width
            uint height;      // 0 means adapt to target height
            uint depth;       // Must be 1 for textureType == TEX_TYPE_2D
            int numMipmaps;     // 0 to disable mipmaps, Negative to generate until the max
                                //(note it's inconsistent with MIP_UNLIMITED)
                                //Will be set to -1 if value is 0 and automipmaps is true
            float widthFactor;  // multiple of target width to use (if width = 0)
            float heightFactor; // multiple of target height to use (if height = 0)
            PixelFormatList formatList; // more than one means MRT
            bool fsaa;          // FSAA enabled; True = Use main target's, False = disable
            bool uav;
            bool automipmaps;
            BoolSetting hwGammaWrite;   // Do sRGB gamma correction on write (only 8-bit per channel formats) 
            uint16 depthBufferId;//Depth Buffer's pool ID.

            /// @see RenderTarget::setPreferDepthTexture
            bool preferDepthTexture;
            PixelFormat depthBufferFormat;

            /** In D3D9, reading from an fsaa texture is not possible, hence it always has
                to be resolved before using it. When resolves are implicit, trying to read
                a render target as a texture will make Ogre to automatically resolve the
                FSAA rt into a normal one.
            @par
                For implicit resolves, try to render everything first, then use it as a texture
                instead of mixing reads & writes, to avoid excessive resolves on the same frame.
            @par
                In D3D10+ & GL 3.3+; FSAA surfaces can be read as textures (that is, without
                resolving) for advanced image manipulation (or even performing custom resolves).
                For this reason, turning Explicit resolves on will force Ogre not to resolve
                targets when used as a texture; resolving can still be done using a PASS_RESOLVE
                or using a PASS_QUAD with a custom resolve pixel shader.
            @par
                Explicit resolves are obviously not supported in D3D9, thus this flag is forced
                always to false, and PASS_RESOLVE passes are skipped.
            */
            bool    fsaaExplicitResolve;

            /// Do not call directly. @see TextureDefinition::renameTexture instead.
            void _setName( IdString newName )   { name = newName; }
            IdString getName(void) const        { return name; }

            TextureDefinition( IdString _name ) : name(_name), textureType( TEX_TYPE_2D ),
                    width(0), height(0), depth(1), numMipmaps(0), widthFactor(1.0f), heightFactor(1.0f),
                    fsaa(true), uav(false), automipmaps(false), hwGammaWrite(BoolUndefined),
                    depthBufferId(1), preferDepthTexture(false), depthBufferFormat(PF_UNKNOWN),
                    fsaaExplicitResolve(false) {}
        };
        typedef vector<TextureDefinition>::type     TextureDefinitionVec;

        struct _OgreExport BufferDefinition : public CompositorInstAlloc
        {
            IdString name;
        public:
            size_t numElements;
            uint32 bytesPerElement;
            uint32 bindFlags;

            /// Sometimes buffers can be used as a plain-array contiguous image (instead of
            /// the swizzled pattern from textures). The formula to calculate final
            /// num elements is :
            ///     finalNumElements = numElements;
            ///     if( widthFactor > 0 )
            ///         finalNumElements *= (widthFactor * width);
            ///     if( heightFactor > 0 )
            ///         finalNumElements *= (heightFactor * height);
            /// For example  if you want to do 512 x height; just set numElements to 512
            /// and heightFactor to 1.
            /// Since there are no pixel formats, the bytesPerElement controls such
            /// such thing (eg. 4 bytes for RGBA8888)
            float widthFactor;  // multiple of target width to use (activates if > 0)
            float heightFactor; // multiple of target height to use (activates if > 0)

            /// Do not call directly. @see TextureDefinition::renameBuffer instead.
            void _setName( IdString newName )   { name = newName; }
            IdString getName(void) const        { return name; }

            BufferDefinition( IdString _name, size_t _numElements,
                              uint32 _bytesPerElement, uint32 _bindFlags,
                              float _widthFactor, float _heightFactor ) :
                    name(_name), numElements( _numElements ),
                    bytesPerElement( _bytesPerElement ), bindFlags( _bindFlags ),
                    widthFactor( _widthFactor ), heightFactor( _heightFactor ) {}
        };
        typedef vector<BufferDefinition>::type     BufferDefinitionVec;

    protected:
        friend class CompositorNode;
        friend class CompositorWorkspace;
        typedef map<IdString, uint32>::type         NameToChannelMap;

        /** TextureSource to use by addLocalTextureDefinition. Could be either
            TEXTURE_LOCAL or TEXTURE_GLOBAL (!!depends on our derived class!!)
        */
        TextureSource           mDefaultLocalTextureSource;
        TextureDefinitionVec    mLocalTextureDefs;
        BufferDefinitionVec     mLocalBufferDefs;
        IdStringVec             mInputBuffers;

        /** Similar to @see CompositorNodeDef::mOutChannelMapping,
            associates a given name with the input, local or global textures.
        */
        NameToChannelMap        mNameToChannelMap;

        static inline uint32 encodeTexSource( size_t index, TextureSource textureSource )
        {
            assert( index <= 0x3FFFFFFF && "Texture Source Index out of supported range" );
            return (index & 0x3FFFFFFF)|(textureSource<<30);
        }

        static void decodeTexSource( uint32 encodedVal, size_t &outIdx, TextureSource &outTexSource );

    public:
        TextureDefinitionBase( TextureSource defaultSource );

        /// This has O(N) complexity! (not cached, we look in mNameToChannelMap)
        size_t getNumInputChannels(void) const;
        size_t getNumInputBufferChannels(void) const;

        /** Adds a texture name, whether a real one or an alias, and where to grab it from.
        @remarks
            Throws if a texture with same name already exists, or if the name makes improper
            usage of the 'global' prefix.
        @par
            This is a generic way to add input channels, by calling:

                addTextureSourceName( "myRT", 0, TextureDefinitionBase::TEXTURE_INPUT );

            You're assigning an alias named "myRT" to channel Input #0
            For local or global textures, the index parameter documentation

        @param fullName
            The name of the texture. Names are usually valid only throughout this node.
            We need the name, not its hash because we need to validate the global_ prefix
            is used correctly.
        @param index
            Index in the container where the texture is located, eg. this->mLocalTextureDefs[index]
            for local textures, workspace->mLocalTextureDefs[index] for global textures, and
            this->mInTextures[index] for input channels.
        @param textureSource
            Source where the index must be used (eg. TEXTURE_LOCAL means mLocalTextureDefs)
        @return
            IdString of the fullName paremeter, for convenience
        */
        virtual IdString addTextureSourceName( const String &name, size_t index,
                                                TextureSource textureSource );

        /** WARNING: Be very careful with this function.
            Removes a texture.
            * If the texture is from an input channel (TEXTURE_INPUT),
              the input channel is removed.
            * If the texture is a local definition (TEXTURE_LOCAL) the texture definition
              is removed and all the references to mLocalTextureDefs[i+1] ...
              mLocalTextureDefs[i+n] are updated.
              However, the output channels will now contain an invalid index and will
              only be removed if it was the last output channel (since we can't alter
              the order). It is your responsability to call
              CompositorNodeDef::mapOutputChannel again with a valid texture name to
              the channel it was occupying.
            * If the texture is a global texture (TEXTURE_GLOBAL), the global texture
              can no longer be accessed until
              addTextureSourceName( name, 0, TEXTURE_GLOBAL ) is called again.
        @param name
            Name of the texture to remove.
        */
        virtual void removeTexture( IdString name );

        /** Changes the name of a texture. Texture can come from an input channel,
            be a global texture, or a locally defined one.
            You can't rename a global texture to avoid the "global_" prefix, or
            add the "global_" prefix to a texture that wasn't global.
        */
        void renameTexture( IdString oldName, const String &newName );

        /** Retrieves in which container to look for when looking to which texture is a given name
            associated with.
        @remarks
            Throws if name is not found.
        @param name
            The name of the texture. Names are usually valid only throughout this node.
        @param index [out]
            The index at the container in which the texture associated with the output channel
            is stored
        @param textureSource [out]
            Where to get this texture from
        */
        void getTextureSource( IdString name, size_t &index, TextureSource &textureSource ) const;

        /** Reserves enough memory for all texture definitions
        @remarks
            Calling this function is not obligatory, but recommended
        @param numPasses
            The number of texture definitions expected to contain.
        */
        void setNumLocalTextureDefinitions( size_t numTDs )     { mLocalTextureDefs.reserve( numTDs ); }

        /** Creates a TextureDefinition with a given name, must be unique.
        @remarks
            WARNING: Calling this function may invalidate all previous returned pointers
            unless you've properly called setLocalTextureDefinitions
        @par
            @See addTextureSourceName remarks for what it can throw
        @par
            Textures are local when the derived class is a Node definition, and
            it's global when the derived class is a Workspace definition
        @param name
            The name of the texture. Names are usually valid only throughout this node.
            We need the name, not its hash because we need to validate the global_ prefix
            is used correctly.
        */
        TextureDefinition* addTextureDefinition( const String &name );

        const TextureDefinitionVec& getLocalTextureDefinitions(void) const  { return mLocalTextureDefs; }

        /** Returns the local texture definitions.
        @remarks
            WARNING: Use with care. You should not add/remove elements or change the name
            as mNameToChannelMap needs to be kept in sync. @see addTextureDefinition,
            @see removeTexture and @see renameTexture to perform these actions
        */
        TextureDefinitionVec& getLocalTextureDefinitionsNonConst(void)      { return mLocalTextureDefs; }

        const NameToChannelMap& getNameToChannelMap(void) const             { return mNameToChannelMap; }

        /** Utility function to create the textures based on a given set of
            texture definitions and put them in a container.
        @remarks
            Useful because both Workspace & CompositorNode share the same functionality
            (create global/local textures respectively) without having to create a whole
            base class just for one function. It's confusing that Nodes & Workspace would
            share the same base class, as if they were the same base object or share
            similar functionality (when in fact, workspace manages nodes)
        @param textureDefs
            Array of texture definitions
        @param inOutTexContainer
            Where we'll store the newly created RTs & textures
        @param id
            Unique id in the case we want textures to have unique names (uniqueNames must be true)
        @param uniqueNames
            Set to true if each RT will have a unique name based on given Id, or we don't. The
            latter is useful for global textures (let them get access through materials)
        @param finalTarget
            The final render target (usually the render window) we have to clone parameters from
            (eg. when using auto width & height, or fsaa settings)
        @param renderSys
            The RenderSystem to use
        */
        static void createTextures( const TextureDefinitionVec &textureDefs,
                                    CompositorChannelVec &inOutTexContainer,
                                    IdType id, const RenderTarget *finalTarget,
                                    RenderSystem *renderSys );

        static CompositorChannel createTexture( const TextureDefinition &textureDef,
                                                const String &texName, const RenderTarget *finalTarget,
                                                RenderSystem *renderSys );

        /// @See createTextures
        static void destroyTextures( CompositorChannelVec &inOutTexContainer, RenderSystem *renderSys );

        /** Destroys & recreates only the textures that depend on the main RT
            (i.e. the RenderWindow) resolution
        @param textureDefs
            Array of texture definitions, so we know which ones depend on main RT's resolution
        @param inOutTexContainer
            Where we'll replace the RTs & textures
        @param finalTarget
            The final render target (usually the render window) we have to clone parameters from
            (eg. when using auto width & height, or fsaa settings)
        @param renderSys
            The RenderSystem to use
        @param connectedNodes
            Array of connected nodes that may be using our textures and need to be notified.
        @param passes
            Array of Compositor Passes which may contain the texture being recreated
            When the pointer is null, we don't iterate through it.
        */
        static void recreateResizableTextures( const TextureDefinitionVec &textureDefs,
                                                CompositorChannelVec &inOutTexContainer,
                                                const RenderTarget *finalTarget,
                                                RenderSystem *renderSys,
                                                const CompositorNodeVec &connectedNodes,
                                                const CompositorPassVec *passes );


        /////////////////////////////////////////////////////////////////////////////////
        /// Buffers
        /////////////////////////////////////////////////////////////////////////////////

        /** Specifies that buffer incoming from channel 'inputChannel'
            will be referenced by the name 'name'
        @remarks
            Don't leave gaps. (i.e. set channel 0 & 2, without setting channel 1)
            It's ok to map them out of order (i.e. set channel 2, then 0, then 1)
        @param inputChannel
            Input channel # the buffer comes from.
        @param name
            Name to give to this buffer for referencing it locally from this scope.
            Duplicate names (including names from addBufferDefinition) will raise an
            exception when trying to instantiate the workspace.
        */
        virtual void addBufferInput( size_t inputChannel, IdString name );

        /** Creates an UAV buffer.
        @param name
            Name to give to this buffer for referencing it locally from this scope.
            Duplicate names (including names from addBufferInput) will raise an
            exception when trying to instantiate the workspace.
        @param bindFlags
            Bitmask. @see BufferBindFlags
        @param widthFactor
            @see BufferDefinition::widthFactor
        @param heightFactor
            @see BufferDefinition::widthFactor
        */
        void addBufferDefinition( IdString name, size_t numElements,
                                  uint32 bytesPerElement, uint32 bindFlags,
                                  float widthFactor, float heightFactor );

        /// Remove a buffer. Buffer can come from an input channel, or a locally defined one.
        virtual void removeBuffer( IdString name );

        /** Changes the name of a buffer. Buffer can come from
            an input channel, or a locally defined one.
        */
        void renameBuffer( IdString oldName, const String &newName );

        /** Reserves enough memory for all texture definitions
        @remarks
            Calling this function is not obligatory, but recommended
        @param numPasses
            The number of texture definitions expected to contain.
        */
        void setNumLocalBufferDefinitions( size_t numTDs )      { mLocalBufferDefs.reserve( numTDs ); }

        const BufferDefinitionVec& getLocalBufferDefinitions(void) const    { return mLocalBufferDefs; }

        /** Returns the local buffer definitions.
        @remarks
            WARNING: Use with care. You should not add/remove elements or change the name
            @see addBufferDefinition, @see removeBuffer and @see renameBuffer to perform these actions
        */
        BufferDefinitionVec& getLocalBufferDefinitionsNonConst(void)        { return mLocalBufferDefs; }

        /** Utility function to create the buffers based on a given set of
            buffer definitions and put them in a container.
        @remarks
            Useful because both Workspace & CompositorNode share the same functionality
            (create global/local buffers respectively) without having to create a whole
            base class just for one function. It's confusing that Nodes & Workspace would
            share the same base class, as if they were the same base object or share
            similar functionality (when in fact, workspace manages nodes)
        */
        static void createBuffers( const BufferDefinitionVec &bufferDefs,
                                   CompositorNamedBufferVec &inOutBufContainer,
                                   const RenderTarget *finalTarget, RenderSystem *renderSys );

        static UavBufferPacked* createBuffer( const BufferDefinition &bufferDef,
                                              const RenderTarget *finalTarget, VaoManager *vaoManager );

        /// @see createBuffers
        /// We need the definition because, unlike textures, the container passed in may
        /// contain textures that were not created by us (i.e. global & input textures)
        /// that we shouldn't delete.
        /// It is illegal for two buffers to have the same name, so it's invalid that a
        /// e.g. an input and a local texture would share the same name.
        static void destroyBuffers( const BufferDefinitionVec &bufferDefs,
                                    CompositorNamedBufferVec &inOutBufContainer,
                                    RenderSystem *renderSys );

        /** Destroys & recreates only the buffers that depend on the main RT
            (i.e. the RenderWindow) resolution
        @param textureDefs
            Array of texture definitions, so we know which ones depend on main RT's resolution
        @param inOutTexContainer
            Where we'll replace the RTs & textures
        @param finalTarget
            The final render target (usually the render window) we have to clone parameters from
            (eg. when using auto width & height, or fsaa settings)
        @param renderSys
            The RenderSystem to use
        @param connectedNodes
            Array of connected nodes that may be using our buffers and need to be notified.
        @param passes
            Array of Compositor Passes which may contain the texture being recreated
            When the pointer is null, we don't iterate through it.
        */
        static void recreateResizableBuffers( const BufferDefinitionVec &bufferDefs,
                                              CompositorNamedBufferVec &inOutBufContainer,
                                              const RenderTarget *finalTarget,
                                              RenderSystem *renderSys,
                                              const CompositorNodeVec &connectedNodes,
                                              const CompositorPassVec *passes );
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
