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

#ifndef COMPONENTS_BITES_INCLUDE_OGREBITESCONFIGDIALOG_H_
#define COMPONENTS_BITES_INCLUDE_OGREBITESCONFIGDIALOG_H_

#include "OgreBitesPrerequisites.h"
#include "OgreConfigDialog.h"

namespace OgreBites {

    /**
        OGRE comes with it's own renderer configuration dialog, which
        applications can use to easily allow the user to configure the
        settings appropriate to their machine. This defines the
        interface to this standard dialog. Because dialogs are inherently
        tied to a particular platform's windowing system, there will be a
        different subclass for each platform.

     * @return pointer to Ogre::ConfigDialog or NULL if not available on the current platform.
     */
    _OgreBitesExport Ogre::ConfigDialog* getNativeConfigDialog();

} /* namespace OgreBites */

#endif /* COMPONENTS_BITES_INCLUDE_OGREBITESCONFIGDIALOG_H_ */
