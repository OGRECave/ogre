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
#ifndef __CompositorLogic_H__
#define __CompositorLogic_H__

#include "OgrePrerequisites.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */
    /** Interface for compositor logics, which can be automatically binded to compositors,
    *   allowing per-compositor logic (such as attaching a relevant listener) to happen
    *   automatically.
    *   @remarks All methods have empty implementations to not force an implementer into
    *       extending all of them.
    */
    class _OgreExport CompositorLogic
    {
    public:
        /** Called when a compositor instance has been created.

            This happens after its setup was finished, so the chain is also accessible.
            This is an ideal method to automatically attach a compositor listener.
        */
        virtual void compositorInstanceCreated(CompositorInstance* newInstance) {}

        /** Called when a compositor instance has been destroyed

            The chain that contained the compositor is still alive during this call.
        */
        virtual void compositorInstanceDestroyed(CompositorInstance* destroyedInstance) {}

        virtual ~CompositorLogic() {}
    };
    /** @} */
    /** @} */
}

#endif
