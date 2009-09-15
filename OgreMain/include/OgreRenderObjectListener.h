/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#ifndef __RenderObjectListener_H__
#define __RenderObjectListener_H__

#include "OgrePrerequisites.h"
#include "OgreRenderable.h"
#include "OgrePass.h"

namespace Ogre {

	class Pass;
	class Renderable;

    /** Abstract interface which classes must implement if they wish to receive
        events from the scene manager when single object is about to be rendered. 
    */
	class _OgreExport RenderObjectListener
	{
	public:
		virtual ~RenderObjectListener() {}
		/** Event raised when render single object started.
		@remarks
		This method is called by the SceneManager.
		@param pass The renderable that is going to be rendered.
		@param pass The pass which was set.
		@param source The auto parameter source used within this render call.
		@pLightList The light list in use.
		@suppressRenderStateChanges True if render state changes should be suppressed.
		*/
		virtual void notifyRenderSingleObject(Renderable* rend, const Pass* pass, const AutoParamDataSource* source, 
			const LightList* pLightList, bool suppressRenderStateChanges) = 0;
	};


}

#endif

