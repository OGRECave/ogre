/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd
Copyright (c) 2006 Matthias Fink, netAllied GmbH <matthias.fink@web.de>

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
#ifndef __ShadowCameraSetupFocused_H__
#define __ShadowCameraSetupFocused_H__

#include "OgrePrerequisites.h"
#include "OgreShadowCameraSetup.h"

#include "OgreHeaderPrefix.h"
#include "OgreAxisAlignedBox.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Scene
    *  @{
    */
    /** Implements the uniform shadow mapping algorithm in focused mode.
    @remarks
        Differs from the default shadow mapping projection in that it focuses the
        shadow map on the visible areas of the scene. This results in better
        shadow map texel usage, at the expense of some 'swimming' of the shadow
        texture on receivers as the basis is constantly being reevaluated.
    @note
        Original implementation by Matthias Fink <matthias.fink@web.de>, 2006.
    */
    class _OgreExport FocusedShadowCameraSetup : public DefaultShadowCameraSetup
    {
    public:
        /** Default constructor.
        @remarks
            Temporary frustum and camera set up here.
        */
        FocusedShadowCameraSetup(void);

        /** Default destructor.
        @remarks
            Temporary frustum and camera destroyed here.
        */
        virtual ~FocusedShadowCameraSetup(void);

        /** Returns a uniform shadow camera with a focused view.
        */
        virtual void getShadowCamera( const SceneManager *sm, const Camera *cam,
                                      const Light *light, Camera *texCam, size_t iteration,
                                      const Vector2 &viewportRealSize ) const;

    };

    /** @} */
    /** @} */

} // namespace Ogre

#include "OgreHeaderSuffix.h"

#endif // __ShadowCameraSetupFocused_H__
