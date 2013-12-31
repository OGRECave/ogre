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
#ifndef __CustomCompositionPass_H__
#define __CustomCompositionPass_H__

#include "OgrePrerequisites.h"
#include "OgreCompositionPass.h"

namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup Effects
	*  @{
	*/
	/** Interface for custom composition passes, allowing custom operations (in addition to
	*	the quad, scene and clear operations) in composition passes.
	*	@see CompositorManager::registerCustomCompositionPass
    */
    class _OgreExport CustomCompositionPass
    {
	public:
		/** Create a custom composition operation.
			@param pass The CompositionPass that triggered the request
			@param instance The compositor instance that this operation will be performed in
			@remarks This call only happens once during creation. The RenderSystemOperation will
			get called each render.
			@remarks The created operation must be instanciated using the OGRE_NEW macro.
		*/
		virtual CompositorInstance::RenderSystemOperation* createOperation(
			CompositorInstance* instance, const CompositionPass* pass) = 0;

	protected:
		virtual ~CustomCompositionPass() {}
	};
	/** @} */
	/** @} */
}

#endif
