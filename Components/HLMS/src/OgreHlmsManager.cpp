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

#include "OgreHlmsManager.h"
#include "OgreHlmsMaterialBase.h"

namespace Ogre
{
	static const String& HLMS_KEY = "hlmsMatBinding";
	//-----------------------------------------------------------------------------------
    HlmsManager::HlmsManager(SceneManager* sceneManager, const String& pieseFilesResorceGroup)
            : mSceneManager(sceneManager), mShaderManager(pieseFilesResorceGroup)
    {
        mSceneManager->addListener(this);
        mSceneManager->addRenderObjectListener(this);
	}
	//-----------------------------------------------------------------------------------
	HlmsManager::~HlmsManager()
	{
		mSceneManager->removeListener(this);
		mSceneManager->removeRenderObjectListener(this);
	}
	//-----------------------------------------------------------------------------------
	void HlmsManager::preFindVisibleObjects(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v)
	{
		// Before the frame is renderd, check all binded renderables if there shaders have to be changed
		const String& curMaterialScheme = v->getMaterialScheme();
		const LightList& lightList = source->_getLightsAffectingFrustum();

		// set all materials to dirty (IsUpToDate = false)
		RenderableVector::iterator rendIt = mBindedRenderables.begin();
		RenderableVector::iterator rendItEnd = mBindedRenderables.end();
		for (; rendIt != rendItEnd; rendIt++)
		{
			Renderable* rend = *rendIt;

			Any hlmsMatBindingAny = rend->getUserObjectBindings().getUserAny(HLMS_KEY);

			// this check should not fail
			if (!hlmsMatBindingAny.has_value())
				continue;

			HlmsMatBindingMap* hlmsMatBindingMap = any_cast<HlmsMatBindingMap*>(hlmsMatBindingAny);

			HlmsMatBindingMap::iterator bindingIt = hlmsMatBindingMap->begin();
			HlmsMatBindingMap::iterator bindingItEnd = hlmsMatBindingMap->end();
			for (; bindingIt != bindingItEnd; bindingIt++)
			{
				bindingIt->second->IsDirty = true;
			}
		}

		// itreate over all binded renderables
		rendIt = mBindedRenderables.begin();
		rendItEnd = mBindedRenderables.end();
		for (; rendIt != rendItEnd; rendIt++)
		{
			Renderable* rend = *rendIt;

			Any hlmsMatBindingAny = rend->getUserObjectBindings().getUserAny(HLMS_KEY);

			// this check should not fail
			if (!hlmsMatBindingAny.has_value())
				continue;

			HlmsMatBindingMap* hlmsMatBindingMap = any_cast<HlmsMatBindingMap*>(hlmsMatBindingAny);

			HlmsMatBindingMap::iterator bindingIt = hlmsMatBindingMap->begin();
			HlmsMatBindingMap::iterator bindingItEnd = hlmsMatBindingMap->end();
			for (; bindingIt != bindingItEnd; bindingIt++)
			{
				String passName = bindingIt->first;
				HlmsMaterialBase* hlmsMaterial = bindingIt->second;

				MaterialPtr mat = rend->getMaterial();

				unsigned short numTechniques = mat->getNumTechniques();
				for (int t = 0; t < numTechniques; t++)
				{
					Technique* tech = mat->getTechnique(t);

					// do not update technics which are not used
					if (tech->getSchemeName() != curMaterialScheme)
						continue;

					unsigned short numPasses = tech->getNumPasses();
					for (int p = 0; p < numPasses; p++)
					{
						Pass* pass = tech->getPass(p);
						if (passName == pass->getName())
						{
							if (hlmsMaterial->IsDirty)
							{
								// update the property map of the material with the lightList
								hlmsMaterial->updatePropertyMap(v->getCamera(), &lightList);
								hlmsMaterial->IsDirty = false;
							}

							bool HasShaderChanged = false;

							// Vertex program
							HlmsDatablock* vertexDatablock = hlmsMaterial->getVertexDatablock();
							if (vertexDatablock)
							{
								GpuProgramPtr gpuProgram = mShaderManager.getGpuProgram(vertexDatablock);

								if (!pass->hasVertexProgram() || pass->getVertexProgram() != gpuProgram)
								{
									pass->removeAllTextureUnitStates();
									pass->setVertexProgram(gpuProgram->getName());
									pass->setVertexProgramParameters(gpuProgram->createParameters());
									HasShaderChanged = true;
								}
							}

							// Fragment program
							HlmsDatablock* fragmentDatablock = hlmsMaterial->getFragmentDatablock();
							if (fragmentDatablock)
							{
								GpuProgramPtr gpuProgram = mShaderManager.getGpuProgram(fragmentDatablock);

								if (!pass->hasFragmentProgram() || pass->getFragmentProgram() != gpuProgram)
								{
									pass->removeAllTextureUnitStates();
									pass->setFragmentProgram(gpuProgram->getName());
									GpuProgramParametersSharedPtr params = gpuProgram->createParameters();
									pass->setFragmentProgramParameters(params);
									HasShaderChanged = true;
								}
							}

							// Recreate all texture unit states
							if (HasShaderChanged)
							{
								hlmsMaterial->createTextureUnits(pass);
							}
						}
					}
				}
			}
		}
	}
	//-----------------------------------------------------------------------------------
	void HlmsManager::notifyRenderSingleObject(Renderable* rend, const Pass* pass,
		const AutoParamDataSource* source, const LightList* pLightList, bool suppressRenderStateChanges)
	{
		// check if the renderable has bounded hlmsMaterials
		Any hlmsMatBindingAny = rend->getUserObjectBindings().getUserAny(HLMS_KEY);
		if (!hlmsMatBindingAny.has_value())
			return;

		// get the bounded material for the current pass
		HlmsMatBindingMap* hlmsMatBindingMap = any_cast<HlmsMatBindingMap*>(hlmsMatBindingAny);
		HlmsMatBindingMap::iterator bindingIt = hlmsMatBindingMap->find(pass->getName());
		if (bindingIt != hlmsMatBindingMap->end())
		{
			HlmsMaterialBase* material = bindingIt->second;

			// update the uniforms
			material->updateUniforms(pass, source, pLightList);
		}
	}
	//-----------------------------------------------------------------------------------
	void HlmsManager::bind(Renderable* rend, HlmsMaterialBase* material, const String& passName)
	{
		HlmsMatBindingMap* hlmsMatMap;
		if (!rend->getUserObjectBindings().getUserAny(HLMS_KEY).has_value())
		{
			hlmsMatMap = new HlmsMatBindingMap();
			rend->getUserObjectBindings().setUserAny(HLMS_KEY, Any(hlmsMatMap));

			// add the randerable to the mBindedRenderables list
			RenderableVector::iterator it = std::find(mBindedRenderables.begin(), mBindedRenderables.end(), rend);
			if (it == mBindedRenderables.end())
				mBindedRenderables.push_back(rend);
		}
		else
		{
			hlmsMatMap = any_cast<HlmsMatBindingMap*>(rend->getUserObjectBindings().getUserAny(HLMS_KEY));
		}

		(*hlmsMatMap)[passName] = material;
	}
	//-----------------------------------------------------------------------------------
	void HlmsManager::unbind(Renderable* rend, const String& passName)
	{
		if (rend->getUserObjectBindings().getUserAny(HLMS_KEY).has_value())
		{
			HlmsMatBindingMap* hlmsMatMap = any_cast<HlmsMatBindingMap*>(rend->getUserObjectBindings().getUserAny(HLMS_KEY));
			hlmsMatMap->erase(passName);

			// if the hasmap is empty delete it
			if (hlmsMatMap->size() <= 0)
			{
				rend->getUserObjectBindings().eraseUserAny(HLMS_KEY);
				delete hlmsMatMap;

				// remove the randerable from the mBindedRenderables list
				RenderableVector::iterator it = std::find(mBindedRenderables.begin(), mBindedRenderables.end(), rend);
				if (it != mBindedRenderables.end())
					mBindedRenderables.erase(it);
			}
		}
	}
	//-----------------------------------------------------------------------------------
	void HlmsManager::unbindAll(const String& passName)
	{
		RenderableVector::iterator bindingIt = mBindedRenderables.begin();
		RenderableVector::iterator bindingItEnd = mBindedRenderables.end();
		for (; bindingIt != bindingItEnd; bindingIt++)
		{
			Renderable* rend = *bindingIt;
			HlmsMatBindingMap* hlmsMatMap = any_cast<HlmsMatBindingMap*>(rend->getUserObjectBindings().getUserAny(HLMS_KEY));
			hlmsMatMap->erase(passName);

			// if the hasmap is empty delete it
			if (hlmsMatMap->size() <= 0)
			{
				rend->getUserObjectBindings().eraseUserAny(HLMS_KEY);
				delete hlmsMatMap;
			}
		}
		mBindedRenderables.clear();
	}
	//-----------------------------------------------------------------------------------
	bool HlmsManager::hasBinding(Renderable* rend, const String& passName)
	{
		if (rend->getUserObjectBindings().getUserAny(HLMS_KEY).has_value())
		{
		    HlmsMatBindingMap* hlmsMatMap = any_cast<HlmsMatBindingMap*>(rend->getUserObjectBindings().getUserAny(HLMS_KEY));
			return hlmsMatMap->find(passName) != hlmsMatMap->end();
		}

		return false;
	}
	//-----------------------------------------------------------------------------------
}
