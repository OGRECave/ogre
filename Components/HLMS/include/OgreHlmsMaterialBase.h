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
#include "OgreHlmsPropertyMap.h"
#include "OgreHlmsDatablock.h"

namespace Ogre
{
	/** \addtogroup Optional
	*  @{
	*/
	/** \addtogroup Hlms
	*  @{
	*/
	class _OgreHlmsExport HlmsMaterialBase : public PassAlloc
    {
    public:
		HlmsMaterialBase();
		virtual ~HlmsMaterialBase();

		HlmsDatablock* getVertexDatablock(){ return &mVertexDatablock; }
		HlmsDatablock* getFragmentDatablock(){ return &mFragmentDatablock; }

		PropertyMap& getPropertyMap(){ return mPropertyMap; }

		/// this is called once per frame
		virtual void updatePropertyMap(Camera* camera, const LightList* pLightList){}

		/// this is called once per frame if the shader has changed. (it is guaranteed that there are not texture units in the pass)
		virtual void createTextureUnits(Pass* pass){}

		/// this is called for every renderable before it is renderd with the given pass
		virtual void updateUniforms(const Pass* pass, const AutoParamDataSource* source, const LightList* pLightList) {}

		bool IsDirty;

	protected:
		HlmsDatablock mVertexDatablock;
		HlmsDatablock mFragmentDatablock;

		PropertyMap mPropertyMap;
    };
}

