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

#ifndef __CompositorNodeDef_H__
#define __CompositorNodeDef_H__

#include "OgreHeaderPrefix.h"
#include "Compositor/OgreTextureDefinition.h"
#include "Compositor/Pass/OgreCompositorPassDef.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    typedef vector<uint32>::type                ChannelMappings;
    typedef vector<CompositorTargetDef>::type   CompositorTargetDefVec;

    /** Compositor nodes are the core subject of compositing.
    @par
        They can define local textures and pass them to other nodes within the same
        workspace through their output channels.
        They can also receive textures through their input channels and send them through a
        different output channel to the next node(s).
        They can access global textures defined by the Workspace they belong to, but can't
        pass them to the output channels.
    @par
        How nodes input and output channels are wired together depends on the Workspace (See
        @CompositorWorkspace and @CompositorWorkspaceDef)
        A node cannot receive input from two nodes in the same channel, but it can send output
        to more than one node using the same output channel.
    @par
        A node whose workspace didn't connect all of its input channels cannot be executed,
        and unless it is disabled, the Compositor will be unable to render.
    @par
        The other core feature of nodes (besides textures and channels) is that they perform
        passes on RTs. A pass is the basic way to render: it can be a @PASS_SCENE, @PASS_QUAD,
        @PASS_CLEAR, @PASS_STENCIL, @PASS_RESOLVE
    @par
        This is the definition. For the instantiation, @see CompositorNode
    @remarks
        We own the local textures, so it's our job to destroy them
    @author
        Matias N. Goldberg
    @version
        1.1
    */
    class _OgreExport CompositorNodeDef : public TextureDefinitionBase
    {
    protected:
        friend class CompositorNode;

        IdString    mName;

        /** Tells where to grab the RenderTarget from for the output channel.
            They can come either from an input channel, or from local textures.
            The first 30 bits indicate the channel #, the last 30th & 31sts bit are used
            to determine whether it comes from the input channel, the local texture, or
            it is global.
        */
        ChannelMappings         mOutChannelMapping;
        IdStringVec             mOutBufferChannelMapping;
        CompositorTargetDefVec  mTargetPasses;

        bool        mStartEnabled;
        String      mNameStr;

        CompositorManager2 *mCompositorManager;

    public:
        IdString    mCustomIdentifier;

        CompositorNodeDef( const String &name, CompositorManager2 *compositorManager ) :
                TextureDefinitionBase( TEXTURE_LOCAL ),
                mName( name ), mStartEnabled( true ), mNameStr( name ),
                mCompositorManager( compositorManager ) {}
        virtual ~CompositorNodeDef() {}

        IdString getName(void) const                        { return mName; }
        String getNameStr(void) const                       { return mNameStr; }

        /// Whether the node should be start as enabled when instantiated
        void setStartEnabled( bool enabled )                { mStartEnabled = enabled; }
        bool getStartEnabled(void ) const                   { return mStartEnabled; }

        /// See http://www.research.att.com/~bs/bs_faq2.html#overloadderived
        using TextureDefinitionBase::getTextureSource;
        /** Retrieves in which container to look for when wanting to know the output texture
            using the mappings from input/local texture -> output.
        @param outputChannel [in]
            The output channel we want to know about
        @param index [out]
            The index at the container in which the texture associated with the output channel
            is stored
        @param textureSource [out]
            Where to get this texture from
        */
        void getTextureSource( size_t outputChannel, size_t &index, TextureSource &textureSource ) const;

        /** Called right after we create a pass definition. Derived
            classes may want to do something with it
        @param passDef
            Newly created pass to toy with.
        */
        virtual void postInitializePassDef( CompositorPassDef *passDef ) {}

        /** Reserves enough memory for all passes
        @remarks
            Calling this function is obligatory, otherwise unexpected crashes may occur.
            CompositorTargetDef doesn't follow the Rule of Three.
        @param numPasses
            The number of passes expected to contain.
        */
        void setNumTargetPass( size_t numPasses )           { mTargetPasses.reserve( numPasses ); }

        /** Adds a new Target pass.
        @remarks
            WARNING: Calling this function may invalidate all previous returned pointers
            unless you've properly called setNumTargetPass
        @param renderTargetName
            We need the full name, not just the hash; so we can check whether it has the global_ prefix
        @param rtIndex
            The RT to address if it is intended to use with a 3D texture (or a cubemap or a 2D Array)
            @See CompositorPassDef::mRtIndex. Default: 0
        */
        CompositorTargetDef* addTargetPass( const String &renderTargetName, uint32 rtIndex=0 );

        /// Retrieves an existing pass by it's given index.
        CompositorTargetDef* getTargetPass( size_t passIndex )  { return &mTargetPasses[passIndex]; }

        /// Gets the number of passes in this node.
        size_t getNumTargetPasses(void) const                   { return mTargetPasses.size(); }

        /** Reserves enough memory for all output channel mappings (efficient allocation, better than
            using linked lists or other containers with two level of indirections)
        @remarks
            Calling this function is not obligatory, but recommended
        @param numPasses
            The number of output channels expected to contain.
        */
        void setNumOutputChannels( size_t numOuts )         { mOutChannelMapping.reserve( numOuts ); }

        /// Returns the number of output channels
        size_t getNumOutputChannels(void) const             { return mOutChannelMapping.size(); }

        /** Maps the output channel to the given texture name, which can be either a
            local texture or a reference to an input channel. Global textures can't
            be used as output.
        @remarks
            Don't leave gaps. (i.e. set channel 0 & 2, without setting channel 1)
            It's ok to map them out of order (i.e. set channel 2, then 0, then 1)
            Prefer calling @see setNumOutputChannels beforehand
            Will throw if the local texture hasn't been declared yet or the input
            channel name hasn't been set yet (declaration order is important!).
        @param outChannel
            Output channel # to map
        @param textureName
            Name of the texture, which can be to a local texture, or an input
            channel's name. Global textures aren't supported.
        */
        void mapOutputChannel( size_t outChannel, IdString textureName );

        /// @copydoc TextureDefinitionBase::removeTexture
        virtual void removeTexture( IdString name );

        /** Reserves enough memory for all output channel mappings (efficient allocation, better
            than using linked lists or other containers with two level of indirections)
        @remarks
            Calling this function is not obligatory, but recommended
        @param numPasses
            The number of output buffer channels expected to contain.
        */
        void setNumOutputBufferChannels( size_t numOuts )
                                                { mOutBufferChannelMapping.reserve( numOuts ); }

        /** Maps the output channel to the given buffer name which can be either a
            local buffer or a reference to an input channel. Global buffers can't
            be used as output.
        @remarks
            Don't leave gaps. (i.e. set channel 0 & 2, without setting channel 1)
            It's ok to map them out of order (i.e. set channel 2, then 0, then 1)
            Prefer calling @see setNumOutputChannels beforehand
            Will throw if the local texture hasn't been declared yet or the input
            channel name hasn't been set yet (declaration order is important!).
        @param outChannel
            Output channel # to map
        @param textureName
            Name of the buffer
        */
        void mapOutputBufferChannel( size_t outChannel, IdString bufferName );

        /** Returns the pass # of the given pass definition in this node.
            This operation is O(N). Useful for debug output.
        @param passDef
            The pass definition to look for
        @return
            Value in range [0; total_numer_of_passes_in_this_node)
            -1 if not found (pass doesn't belong to this node)
        */
        size_t getPassNumber( const CompositorPassDef *passDef ) const;

        CompositorManager2* getCompositorManager(void) const { return mCompositorManager; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
