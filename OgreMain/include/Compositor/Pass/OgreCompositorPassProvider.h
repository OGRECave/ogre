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

#ifndef _OgreCompositorPassProvider_H_
#define _OgreCompositorPassProvider_H_

#include "OgreHeaderPrefix.h"
#include "Compositor/OgreCompositorCommon.h"
#include "OgreCompositorPassDef.h"
#include "OgreScriptParser.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */

    /** Base class that users can derive from in order to implement custom passes for the compositor.
    @par
        How to implement one:
        You need to derive three classes:
            1. CompositorPassProvider
            2. CompositorPassDef
            3. CompositorPass

        The first one is this class. You need to overload the two public virtuals. i.e.
            CompositorPassDef* MyProvider::addPassDef( ... )
            {
                return OGRE_NEW MyPassDef( ... );
            }

            CompositorPass* MyProvider::addPass( ... )
            {
                return OGRE_NEW MyPass( ... );
            }

        The second and third one are the implementations.
        @see CompositorPassScene and @see CompositorPassSceneDef,
        @see CompositorPassQuad and @see CompositorPassQuadDef,
        @see CompositorPassClear and @see CompositorPassClearDef for examples on how
        to implement your own pass. For example, you could literally copy the code
        from CompositorPassClear & CompositorPassClearDef and implement your own
        custom pass that clears the render target.
    */
    class _OgreExport CompositorPassProvider : public ResourceAlloc
    {
    public:
        /** Called from CompositorTargetDef::addPass when adding a Compositor Pass of type 'custom'
        @param passType
        @param customId
            Arbitrary ID in case there is more than one type of custom pass you want to implement.
            Defaults to IdString()
        @param rtIndex
        @param parentNodeDef
        @return
        */
        virtual CompositorPassDef* addPassDef( CompositorPassType passType,
                                               IdString customId,
                                               CompositorTargetDef *parentTargetDef,
                                               CompositorNodeDef *parentNodeDef ) = 0;

        /** Creates a CompositorPass from a CompositorPassDef for Compositor Pass of type 'custom'
        @remarks    If you have multiple custom pass types then you will need to use dynamic_cast<>()
                    on the CompositorPassDef to determine what custom pass it is.
        */
        virtual CompositorPass* addPass( const CompositorPassDef *definition, Camera *defaultCamera,
                                         CompositorNode *parentNode, const CompositorChannel &target,
                                         SceneManager *sceneManager ) = 0;

        /** Optional override which allows users to define custom properties in the compositor scripts for custom passes.
        @remarks    Please note this is called after CompositorPassProvider::addPassDef and similar to addPass
                    you will need to dynamic_cast<>() to determine custom pass type.
        @param node             The AST node for this pass
        @param customPassDef    The CompositorPassDef returned in CompositorPassProvider::addPassDef

        */
        virtual void translateCustomPass(const AbstractNodePtr &node, CompositorPassDef* customPassDef) {}
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
