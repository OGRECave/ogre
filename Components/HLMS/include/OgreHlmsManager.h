/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

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

#pragma once

#include "Ogre.h"	  
#include "OgreHlmsPrerequisites.h"
#include "OgreHlmsShaderManager.h"
#include "Threading/OgreThreadHeaders.h"

namespace Ogre
{
	class HlmsMaterialBase;

	/** \addtogroup Optional
	*  @{
	*/
	/** \addtogroup Hlms
	*  @{
	*/
	class _OgreHlmsExport HlmsManager : public PassAlloc, public RenderObjectListener, public SceneManager::Listener
    {
    public:
		HlmsManager(SceneManager* sceneManager, const String& pieseFilesResorceGroup = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		virtual ~HlmsManager();

		virtual void notifyRenderSingleObject(
			Renderable* rend, 
			const Pass* pass,
			const AutoParamDataSource* source,
			const LightList* pLightList,
			bool suppressRenderStateChanges);

		virtual void preFindVisibleObjects(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v);

		void bind(Renderable* rend, HlmsMaterialBase* material, const String& passName);
		void unbind(Renderable* rend, const String& passName);
		void unbindAll(const String& passName);
		bool hasBinding(Renderable* rend, const String& passName);

	protected:
		typedef std::unordered_map<String, HlmsMaterialBase*> HlmsMatBindingMap;
		typedef std::vector<Renderable*> RenderableVector;

		SceneManager* mSceneManager;
		ShaderManager mShaderManager;
		RenderableVector mBindedRenderables;
    };
}

