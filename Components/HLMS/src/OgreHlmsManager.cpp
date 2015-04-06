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
#include "OgreHlmsPbsMaterial.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------------
	HlmsManager::HlmsManager(Ogre::Camera* camera) : mCamera(camera)
	{
		mSceneManager = mCamera->getSceneManager();

		mCamera->addListener(this);
		mSceneManager->addRenderObjectListener(this);
	}
	//-----------------------------------------------------------------------------------
	HlmsManager::~HlmsManager()
	{
		mSceneManager->removeRenderObjectListener(this);
		mCamera->removeListener(this);
	}
	//-----------------------------------------------------------------------------------
	void HlmsManager::cameraPreRenderScene(Ogre::Camera* cam)
	{
		//mViewMatrix = cam->getViewMatrix();
	}
	//-----------------------------------------------------------------------------------
	void HlmsManager::notifyRenderSingleObject(Ogre::Renderable* rend, const Ogre::Pass* pass,
		const Ogre::AutoParamDataSource* source, const Ogre::LightList* pLightList, bool suppressRenderStateChanges)
	{
		if (pass->getName() == "pbs")
		{
			HlmsMaterialBase* materialAny = any_cast<HlmsMaterialBase*>(rend->getUserObjectBindings().getUserAny("hlmsMat"));
			if (materialAny)
			{
				HlmsMaterialBase* material = materialAny;//materialAny.get<HlmsMaterialBase*>();
				PropertyMap propMap = material->getPropertyMap();

				if (!pass->isProgrammable())
				{
					// This has to be done because it is not called if the pass is not programmable
					const_cast<Ogre::AutoParamDataSource*>(source)->setCurrentRenderable(rend);
				}

				material->updatePropertyMap(mCamera, pLightList);

				Ogre::Pass* p = const_cast<Ogre::Pass*>(pass);

				bool shaderHasChanged = false;

				// Vertex program
				HlmsDatablock* vertexDatablock = material->getVertexDatablock();
				if (vertexDatablock)
				{
					GpuProgramPtr gpuProgram = mShaderManager.getGpuProgram(vertexDatablock);

					if (!p->hasVertexProgram() || p->getVertexProgram() != gpuProgram)
					{
						p->setVertexProgram(gpuProgram->getName());
						p->setVertexProgramParameters(gpuProgram->createParameters());
						shaderHasChanged = true;
					}
				}

				// Fragment program
				HlmsDatablock* fragmentDatablock = material->getFragmentDatablock();
				if (fragmentDatablock)
				{
					GpuProgramPtr gpuProgram = mShaderManager.getGpuProgram(fragmentDatablock);

					if (!p->hasFragmentProgram() || p->getFragmentProgram() != gpuProgram)
					{
						p->setFragmentProgram(gpuProgram->getName());
						GpuProgramParametersSharedPtr params = gpuProgram->createParameters();
						p->setFragmentProgramParameters(params);
						shaderHasChanged = true;
					}
				}

				material->updateUniforms(mCamera, p, source, pLightList, shaderHasChanged);
			}
		}
	}
	//-----------------------------------------------------------------------------------
	void HlmsManager::bind(Ogre::Renderable* rend, HlmsMaterialBase* material)
	{
		rend->getUserObjectBindings().setUserAny("hlmsMat", Ogre::Any(material));
		//mBindedMaterials[rend] = material;
	}
	//-----------------------------------------------------------------------------------
}
