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

#ifndef __CompositorChannel_H__
#define __CompositorChannel_H__

#include "OgreHeaderPrefix.h"
#include "Compositor/OgreCompositorCommon.h"

#include "OgreTexture.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    /** A channel in the compositor transports textures between nodes.
        Unfortunately, Ogre's design of RenderTargets, Textures & MRTs isn't as straightforward
        and encapsulated as we wanted them to be. Therefore we need this Channel structure
        to abstract them.
    @par
        In short, when we want to render to a texture, we need a RenderTarget. When we want
        to sample from, we need a Texture. Until here there is an almost 1:1 relationship.
        However when MRTs come into play, this relationship is destroyed and not handled
        very well by Ogre. We do.
    */
    struct CompositorChannel
    {
        typedef vector<TexturePtr>::type TextureVec;
        RenderTarget    *target;
        TextureVec      textures;

        CompositorChannel() : target( 0 ) {}

        bool isMrt() const                              { return textures.size() > 1; }
        bool isValid() const                            { return target != 0; }
        bool operator == ( const CompositorChannel &right ) const   { return target == right.target; }
    };

    typedef vector<CompositorChannel>::type CompositorChannelVec;

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
