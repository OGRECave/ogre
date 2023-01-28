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

#include "OgreD3D11Prerequisites.h"

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
#ifndef __D3D11StereoDriverBridge_H__
#define __D3D11StereoDriverBridge_H__

#include "OgreCommon.h"
#include "OgreSingleton.h"
#include "OgreD3D11RenderWindow.h"

namespace Ogre {

  class D3D11StereoDriverImpl;
  typedef bool StereoModeType;

  /** Bridge interface from the render system to the stereo driver. Loads the
   correct driver and forwards the methods to the stereo driver implementation.*/
  class _OgreD3D11Export D3D11StereoDriverBridge : public Singleton<D3D11StereoDriverBridge>, public StereoDriverAlloc
  {
    public:
      D3D11StereoDriverBridge(StereoModeType stereoMode);
      virtual ~D3D11StereoDriverBridge();
      static D3D11StereoDriverBridge& getSingleton(void);
      static D3D11StereoDriverBridge* getSingletonPtr(void);
      StereoModeType getStereoMode() const;
	  bool addRenderWindow(D3D11RenderWindowBase* renderWindow) const;
      bool removeRenderWindow(const String& renderWindowName) const;
      bool isStereoEnabled(const String& renderWindowName) const;
      bool setDrawBuffer(const ColourBufferType colourBuffer) const;

    protected:
      bool mIsNvapiInitialized;
      D3D11StereoDriverImpl* mPimpl;
      StereoModeType mStereoMode;
  };
}
#endif
#endif
