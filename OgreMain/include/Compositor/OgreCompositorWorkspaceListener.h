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

#ifndef _OgreCompositorWorkspaceListener_H_
#define _OgreCompositorWorkspaceListener_H_

#include "OgreHeaderPrefix.h"
#include "Compositor/OgreCompositorCommon.h"
#include "Compositor/Pass/OgreCompositorPassDef.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    class CompositorWorkspaceListener
    {
    public:
        /** Called before all nodes are going to be updated. Use this place to update your own,
            manually updated Workspaces without having to call
            @CompositorWorkspace::_beginUpdate( forceBeginFrame=true )
        */
        virtual void workspacePreUpdate( CompositorWorkspace *workspace ) {}
        /** Called early on in pass' execution. Happens before passPreExecute,
            before the pass has set anything.
            Warning: calling pass->execute can result in recursive calls.
        */
        virtual void passEarlyPreExecute( CompositorPass *pass ) {}
        /** Called when each pass is about to be executed.
            Warning: calling pass->execute can result in recursive calls.
        */
        virtual void passPreExecute( CompositorPass *pass ) {}

        /** Called after a pass has been executed.
            Warning: calling pass->execute can result in recursive calls.
        */
        virtual void passPosExecute( CompositorPass *pass ) {}

        /** Called from CompositorManager2 (not CompositorWorkspace) when we're
            about to begin updating all the workspaces. You'll have to manage
            the RenderSystem and SceneManager to call the adequate begin/end calls
            Warning: Don't add/remove listeners to CompositorManager2 inside this function.
        */
        virtual void allWorkspacesBeforeBeginUpdate(void) {}

        /** Called from CompositorManager2 (not CompositorWorkspace) when we're
            about to update all the workspaces (it's safe to update your own workspaces
            without calling _beginUpdate and _endUpdate)
            Warning: Don't add/remove listeners to CompositorManager2 inside this function.
        */
        virtual void allWorkspacesBeginUpdate(void) {}
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
