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

#ifndef __CompositorPass_H__
#define __CompositorPass_H__

#include "OgreHeaderPrefix.h"

#include "Compositor/Pass/OgreCompositorPassDef.h"

namespace Ogre
{
    class RenderTarget;
    struct CompositorChannel;
    class CompositorNode;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    typedef vector<TexturePtr>::type TextureVec;

    struct CompositorTexture
    {
        IdString            name;
        TextureVec const    *textures;

        CompositorTexture( IdString _name, const TextureVec *_textures ) :
                name( _name ), textures( _textures ) {}

        bool operator == ( IdString right ) const
        {
            return name == right;
        }
    };

    typedef vector<CompositorTexture>::type CompositorTextureVec;

    struct BoundUav
    {
        GpuResource                    *rttOrBuffer;
        ResourceAccess::ResourceAccess  boundAccess;
    };

    /** Abstract class for compositor passes. A pass can be a fullscreen quad, a scene
        rendering, a clear. etc.
        Derived classes are responsible for performing an actual job.
        Note that passes do not own RenderTargets, therefore we're not responsible
        for destroying it.
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    class _OgreExport CompositorPass : public CompositorInstAlloc
    {
        CompositorPassDef const *mDefinition;
    protected:
        static const Quaternion CubemapRotations[6];

        RenderTarget    *mTarget;
        Viewport        *mViewport;

        uint32          mNumPassesLeft;

        CompositorNode  *mParentNode;

        CompositorTexture       mTargetTexture;
        CompositorTextureVec    mTextureDependencies;

        typedef vector<ResourceTransition>::type ResourceTransitionVec;
        ResourceTransitionVec   mResourceTransitions;
        /// In OpenGL, only the first entry in mResourceTransitions contains a real
        /// memory barrier. The rest is just kept for debugging purposes. So
        /// mNumValidResourceTransitions is either 0 or 1.
        /// In D3D12/Vulkan/Mantle however,
        /// mNumValidResourceTransitions = mResourceTransitions.size()
        uint32                  mNumValidResourceTransitions;

        void populateTextureDependenciesFromExposedTextures(void);

        void executeResourceTransitions(void);

        RenderTarget* calculateRenderTarget( size_t rtIndex, const CompositorChannel &source );

    public:
        CompositorPass( const CompositorPassDef *definition, const CompositorChannel &target,
                        CompositorNode *parentNode );
        virtual ~CompositorPass();

        void profilingBegin(void);
        void profilingEnd(void);

        virtual void execute( const Camera *lodCameraconst ) = 0;

        void addResourceTransition( ResourceLayoutMap::iterator currentLayout,
                                    ResourceLayout::Layout newLayout,
                                    uint32 readBarrierBits );

        /** Emulates the execution of a UAV to understand memory dependencies,
            and adds a memory barrier / resource transition if we need to.
        @remarks
            Note that an UAV->UAV resource transition is just a memory barrier.
        @param boundUavs [in/out]
            An array of the currently bound UAVs by slot.
            The derived class CompositorPassUav will write to them as part of the
            emulation. The base implementation reads from this value.
        @param uavsAccess [in/out]
            A map with the last access flag used for each RenderTarget. We need it
            to identify RaR situations, which are the only ones that don't need
            a barrier (and also WaW hazards, when explicitly allowed by the pass).
            Note: We will set the access to ResourceAccess::Undefined to signal other
            passes  that the UAV hazard already has a barrier (just in case there was
            one already created).
        @param resourcesLayout [in/out]
            A map with the current layout of every RenderTarget used so far.
            Needed to identify if we need to change the resource layout
            to an UAV.
        */
        virtual void _placeBarriersAndEmulateUavExecution( BoundUav boundUavs[64],
                                                           ResourceAccessMap &uavsAccess,
                                                           ResourceLayoutMap &resourcesLayout );
        void _removeAllBarriers(void);

        /// @See CompositorNode::notifyRecreated
        virtual void notifyRecreated( const CompositorChannel &oldChannel,
                                        const CompositorChannel &newChannel );
        virtual void notifyRecreated( const UavBufferPacked *oldBuffer, UavBufferPacked *newBuffer );

        /// @See CompositorNode::notifyDestroyed
        virtual void notifyDestroyed( const CompositorChannel &channel );
        virtual void notifyDestroyed( const UavBufferPacked *buffer );

        /// @See CompositorNode::_notifyCleared
        virtual void notifyCleared(void);

        void resetNumPassesLeft(void);

        CompositorPassType getType() const  { return mDefinition->getType(); }

        Viewport* getViewport() const       { return mViewport; }

        RenderTarget* getRenderTarget(void) const           { return mTarget; }

        const CompositorPassDef* getDefinition(void) const  { return mDefinition; }

		const CompositorNode* getParentNode(void) const		{ return mParentNode; }

        const CompositorTexture& getTargetTexture(void) const           { return mTargetTexture; }
        const CompositorTextureVec& getTextureDependencies(void) const  { return mTextureDependencies; }
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
