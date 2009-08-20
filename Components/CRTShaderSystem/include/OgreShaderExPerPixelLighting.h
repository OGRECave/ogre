/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef _ShaderExPPLighting_
#define _ShaderExPPLighting_

#include "OgrePrerequisites.h"
#include "OgreShaderSubRenderState.h"
#include "OgreVector4.h"
#include "OgreLight.h"
#include "OgreCommon.h"

namespace Ogre {
namespace CRTShader {

class Parameter;
class Function;
/************************************************************************/
/*                                                                      */
/************************************************************************/
class PerPixelLighting : public SubRenderState
{
public:
	struct LightProperties
	{
		String				mName;
		Light::LightTypes	mType;
		bool				mEnableSpecular;
	};
	
// Interface.
public:
	PerPixelLighting();
	virtual ~PerPixelLighting();

	virtual const String&	getType					() const;
	virtual int				getExecutionOrder		() const;
	virtual uint32			getHashCode				();
	virtual void			updateGpuProgramsParams	(Renderable* rend, Pass* pass);
	virtual void			preFindVisibleObjects	(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v);

	virtual void			copyFrom				(const SubRenderState& rhs);

	void					setTrackVertexColourType(TrackVertexColourType type) { mTrackVertexColourType = type; }
	TrackVertexColourType	getTrackVertexColourType() const { return mTrackVertexColourType; }

	
	void					setLightCount			(size_t count);
	size_t					getLightCount			() const;

	void					setLightProperties		(unsigned int index, const LightProperties& properties);
	const LightProperties&	getLightProperties		(unsigned int index) const;
		
	static String Type;

// Protected types:
protected:
	struct LightParams
	{
		LightProperties		mProperties;
		Light*				mLight;
		Parameter*			mPosition;
		Parameter*			mDirection;
		Parameter*			mAttenuatParams;
		Parameter*			mSpotParams;
		Parameter*			mDiffuseColour;
		Parameter*			mSpecularColour;

	};

	typedef std::vector<LightParams>				LightParamsList;
	typedef LightParamsList::iterator				LightParamsIterator;
	typedef LightParamsList::const_iterator			LightParamsConstIterator;

// Protected methods
protected:
	virtual bool			resolveParameters				(ProgramSet* programSet);
	virtual bool			resolveDependencies				(ProgramSet* programSet);
	virtual bool			addFunctionInvocations			(ProgramSet* programSet);
			bool			addGlobalIlluminationInvocation	(Function* vsMain, const int groupOrder, int& internalCounter);
			bool			addIlluminationInvocation		(LightParams* curLightParams, Function* psMain, const int groupOrder, int& internalCounter);


// Attributes.
protected:	
	TrackVertexColourType	mTrackVertexColourType;
	LightParamsList			mLightParamsList;	
	Parameter*				mWorldViewMatrix;
	Parameter*				mWorldViewITMatrix;
	Parameter*				mVSInPosition;
	Parameter*				mVSInNormal;	
	Parameter*				mVSDiffuse;	
	Parameter*				mVSOutDiffuse;
	Parameter*				mVSOutSpecular;	
	Parameter*				mVSOutNormal;
	Parameter*				mPSInDiffuse;
	Parameter*				mPSInSpecular;
	Parameter*				mPSOutDiffuse;	
	Parameter*				mPSOutSpecular;

	Parameter*				mDerivedSceneColour;
	Parameter*				mLightAmbientColour;
	Parameter*				mDerivedAmbientLightColour;
	Parameter*				mSurfaceAmbientColour;
	Parameter*				mSurfaceDiffuseColour;
	Parameter*				mSurfaceSpecularColour;	
	Parameter*				mSurfaceEmissiveColour;
	Parameter*				mSurfaceShininess;
	Camera*					mCurCamera;
	Matrix3					mMatViewIT3;
};


/************************************************************************/
/*                                                                      */
/************************************************************************/
class PerPixelLightingFactory : public SubRenderStateFactory
{
public:
	virtual const String&	getType				() const;
	
protected:
	virtual SubRenderState*	createInstanceImpl	();


};

}
}

#endif

