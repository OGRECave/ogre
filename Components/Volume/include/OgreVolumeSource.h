/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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
#ifndef __Ogre_Volume_Source_H__
#define __Ogre_Volume_Source_H__

#include <float.h>
#include "OgreVector3.h"
#include "OgreVolumePrerequisites.h"

namespace Ogre {
namespace Volume {

    /** Abstract class defining the density function.
    */
    class _OgreVolumeExport Source
    {
    public:
        
        /** Destructor.
        */
        virtual ~Source(void);

        /** Gets the density value and gradient.
        @param position
            The position.
        @param gradient
            Where the gradient will be stored.
        @return
            The density.
        */
        virtual Real getValueAndGradient(const Vector3 &position, Vector3 &gradient) const = 0;

        /** Gets the density value.
        @param position
            The position.
        @return
            The density.
        */
        virtual Real getValue(const Vector3 &position) const = 0;

    };

}
}

#endif
