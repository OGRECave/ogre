/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd

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
#include "OgreStableHeaders.h"
#include "OgreLight.h"

#include "OgreException.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"
#include "OgreSceneManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    Light::Light( IdType id, ObjectMemoryManager *objectMemoryManager )
		: MovableObject( id, objectMemoryManager ),
		  mLightType(LT_POINT),
          mDiffuse(ColourValue::White),
          mSpecular(ColourValue::Black),
		  mSpotOuter(Degree(40.0f)),
          mSpotInner(Degree(30.0f)),
          mSpotFalloff(1.0f),
          mSpotNearClip(0.0f),
		  mRange(100000),
		  mAttenuationConst(1.0f),
		  mAttenuationLinear(0.0f),
          mAttenuationQuad(0.0f),
		  mPowerScale(1.0f),
		  mOwnShadowFarDist(false),
		  mShadowFarDist(0),
		  mShadowFarDistSquared(0),
		  mShadowNearClipDist(-1),
		  mShadowFarClipDist(-1)
    {
		//mMinPixelSize should always be zero for lights otherwise lights will disapear
    	mMinPixelSize = 0;

		mObjectData.mLocalRadius[mObjectData.mIndex] = std::numeric_limits<Real>::infinity();
		mObjectData.mWorldRadius[mObjectData.mIndex] = std::numeric_limits<Real>::infinity();
    }
    //-----------------------------------------------------------------------
    Light::~Light()
    {
    }
    //-----------------------------------------------------------------------
    void Light::setType(LightTypes type)
    {
        mLightType = type;

		switch( mLightType )
		{
		case LT_POINT:
			mObjectData.mLocalRadius[mObjectData.mIndex] = mRange;
			mObjectData.mLocalAabb->setFromAabb( Aabb( Vector3::ZERO, Vector3( mRange ) ),
												 mObjectData.mIndex );
			break;
		case LT_DIRECTIONAL:
			mObjectData.mLocalAabb->setFromAabb( Aabb::BOX_INFINITE, mObjectData.mIndex );
			mObjectData.mLocalRadius[mObjectData.mIndex] = std::numeric_limits<Real>::infinity();
			break;
		case LT_SPOTLIGHT:
			setSpotAabb();
			break;
		}
    }
	//-----------------------------------------------------------------------
	void Light::setSpotAabb(void)
	{
		assert( mLightType == LT_SPOTLIGHT );

		//In local space, lights are centered at origin, facing towards +Z
		Aabb aabb;
		Real lenOpposite = Math::Tan( mSpotOuter ) * mRange;
		aabb.m_center	= Vector3::ZERO;
		aabb.m_halfSize	= Vector3( lenOpposite, mRange, lenOpposite );
		mObjectData.mLocalRadius[mObjectData.mIndex] = aabb.getRadius();
		mObjectData.mLocalAabb->setFromAabb( Aabb( Vector3::ZERO, Vector3( mRange ) ), mObjectData.mIndex );
	}
    //-----------------------------------------------------------------------
    void Light::setDirection(const Vector3& vec)
    {
		assert( dynamic_cast<SceneNode*>( mParentNode ) );
		assert( !static_cast<SceneNode*>( mParentNode )->isYawFixed() && "Attach Lights to a "
				"SceneNode without a fixed yaw! (SceneNode::setFixedYawAxis(false))" );
		static_cast<SceneNode*>( mParentNode )->setDirection( vec, Node::TS_PARENT, Vector3::UNIT_Z );
    }
    //-----------------------------------------------------------------------
    Vector3 Light::getDirection(void) const
    {
        return mParentNode->getOrientation().zAxis();
    }
    //-----------------------------------------------------------------------
    void Light::setSpotlightRange(const Radian& innerAngle, const Radian& outerAngle, Real falloff)
    {
		bool boundsChanged = mSpotOuter != outerAngle;

        mSpotInner = innerAngle;
        mSpotOuter = outerAngle;
        mSpotFalloff = falloff;

		if( boundsChanged && mLightType == LT_SPOTLIGHT )
			setSpotAabb();
    }
	//-----------------------------------------------------------------------
	void Light::setSpotlightInnerAngle(const Radian& val)
	{
		mSpotInner = val;
	}
	//-----------------------------------------------------------------------
	void Light::setSpotlightOuterAngle(const Radian& val)
	{
		bool boundsChanged = mSpotOuter != val;
		mSpotOuter = val;
		if( boundsChanged && mLightType == LT_SPOTLIGHT )
			setSpotAabb();
	}
    //-----------------------------------------------------------------------
    void Light::setAttenuation(Real range, Real constant,
                        Real linear, Real quadratic)
    {
		bool boundsChanged = mRange != range;

        mRange = range;
        mAttenuationConst = constant;
        mAttenuationLinear = linear;
        mAttenuationQuad = quadratic;

		if( boundsChanged )
		{
			if( mLightType == LT_POINT )
			{
				mObjectData.mLocalRadius[mObjectData.mIndex] = mRange;
				mObjectData.mLocalAabb->setFromAabb( Aabb( Vector3::ZERO, Vector3( mRange ) ),
													 mObjectData.mIndex );
			}
			else if( mLightType == LT_SPOTLIGHT )
			{
				setSpotAabb();
			}
		}
    }
    //-----------------------------------------------------------------------
	void Light::setPowerScale(Real power)
	{
		mPowerScale = power;
	}
    //-----------------------------------------------------------------------
    const String& Light::getMovableType(void) const
    {
		return LightFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    Vector3 Light::getDerivedDirection(void) const
    {
        return mParentNode->_getDerivedOrientation().zAxis();
    }
	//-----------------------------------------------------------------------
    Vector3 Light::getDerivedDirectionUpdated(void) const
    {
        return mParentNode->_getDerivedOrientationUpdated().zAxis();
    }
    //-----------------------------------------------------------------------
	Vector4 Light::getAs4DVector(void) const
	{
		Vector4 ret;
        if (mLightType == Light::LT_DIRECTIONAL)
        {
            ret = -(getDerivedDirection()); // negate direction as 'position'
            ret.w = 0.0; // infinite distance
        }	
		else
        {
            ret = mParentNode->_getDerivedPosition();
            ret.w = 1.0;
        }
		return ret;
	}
	//-----------------------------------------------------------------------
	uint32 Light::getTypeFlags(void) const
	{
		return SceneManager::LIGHT_TYPE_MASK;
	}
	//---------------------------------------------------------------------
	void Light::_calcTempSquareDist(const Vector3& worldPos)
	{
		if (mLightType == LT_DIRECTIONAL)
		{
			tempSquareDist = 0;
		}
		else
		{
			tempSquareDist = 
				(worldPos - mParentNode->_getDerivedPosition()).squaredLength();
		}

	}
	//-----------------------------------------------------------------------
	const String& Light::getAnimableDictionaryName(void) const
	{
		return LightFactory::FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------
	void Light::initialiseAnimableDictionary(StringVector& vec) const
	{
		vec.push_back("diffuseColour");
		vec.push_back("specularColour");
		vec.push_back("attenuation");
		vec.push_back("spotlightInner");
		vec.push_back("spotlightOuter");
		vec.push_back("spotlightFalloff");
	}
	//-----------------------------------------------------------------------
	class LightDiffuseColourValue : public AnimableValue
	{
	protected:
		Light* mLight;
	public:
		LightDiffuseColourValue(Light* l) :AnimableValue(COLOUR) 
		{ mLight = l; }
		void setValue(const ColourValue& val)
		{
			mLight->setDiffuseColour(val);
		}
		void applyDeltaValue(const ColourValue& val)
		{
			setValue(mLight->getDiffuseColour() + val);
		}
		void setCurrentStateAsBaseValue(void)
		{
			setAsBaseValue(mLight->getDiffuseColour());
		}

	};
	//-----------------------------------------------------------------------
	class LightSpecularColourValue : public AnimableValue
	{
	protected:
		Light* mLight;
	public:
		LightSpecularColourValue(Light* l) :AnimableValue(COLOUR) 
		{ mLight = l; }
		void setValue(const ColourValue& val)
		{
			mLight->setSpecularColour(val);
		}
		void applyDeltaValue(const ColourValue& val)
		{
			setValue(mLight->getSpecularColour() + val);
		}
		void setCurrentStateAsBaseValue(void)
		{
			setAsBaseValue(mLight->getSpecularColour());
		}

	};
	//-----------------------------------------------------------------------
	class LightAttenuationValue : public AnimableValue
	{
	protected:
		Light* mLight;
	public:
		LightAttenuationValue(Light* l) :AnimableValue(VECTOR4) 
		{ mLight = l; }
		void setValue(const Vector4& val)
		{
			mLight->setAttenuation(val.x, val.y, val.z, val.w);
		}
		void applyDeltaValue(const Vector4& val)
		{
			setValue(mLight->getAs4DVector() + val);
		}
		void setCurrentStateAsBaseValue(void)
		{
			setAsBaseValue(mLight->getAs4DVector());
		}

	};
	//-----------------------------------------------------------------------
	class LightSpotlightInnerValue : public AnimableValue
	{
	protected:
		Light* mLight;
	public:
		LightSpotlightInnerValue(Light* l) :AnimableValue(REAL) 
		{ mLight = l; }
		void setValue(Real val)
		{
			mLight->setSpotlightInnerAngle(Radian(val));
		}
		void applyDeltaValue(Real val)
		{
			setValue(mLight->getSpotlightInnerAngle().valueRadians() + val);
		}
		void setCurrentStateAsBaseValue(void)
		{
			setAsBaseValue(mLight->getSpotlightInnerAngle().valueRadians());
		}

	};
	//-----------------------------------------------------------------------
	class LightSpotlightOuterValue : public AnimableValue
	{
	protected:
		Light* mLight;
	public:
		LightSpotlightOuterValue(Light* l) :AnimableValue(REAL) 
		{ mLight = l; }
		void setValue(Real val)
		{
			mLight->setSpotlightOuterAngle(Radian(val));
		}
		void applyDeltaValue(Real val)
		{
			setValue(mLight->getSpotlightOuterAngle().valueRadians() + val);
		}
		void setCurrentStateAsBaseValue(void)
		{
			setAsBaseValue(mLight->getSpotlightOuterAngle().valueRadians());
		}

	};
	//-----------------------------------------------------------------------
	class LightSpotlightFalloffValue : public AnimableValue
	{
	protected:
		Light* mLight;
	public:
		LightSpotlightFalloffValue(Light* l) :AnimableValue(REAL) 
		{ mLight = l; }
		void setValue(Real val)
		{
			mLight->setSpotlightFalloff(val);
		}
		void applyDeltaValue(Real val)
		{
			setValue(mLight->getSpotlightFalloff() + val);
		}
		void setCurrentStateAsBaseValue(void)
		{
			setAsBaseValue(mLight->getSpotlightFalloff());
		}

	};
	//-----------------------------------------------------------------------
	AnimableValuePtr Light::createAnimableValue(const String& valueName)
	{
		if (valueName == "diffuseColour")
		{
			return AnimableValuePtr(
				OGRE_NEW LightDiffuseColourValue(this));
		}
		else if(valueName == "specularColour")
		{
			return AnimableValuePtr(
				OGRE_NEW LightSpecularColourValue(this));
		}
		else if (valueName == "attenuation")
		{
			return AnimableValuePtr(
				OGRE_NEW LightAttenuationValue(this));
		}
		else if (valueName == "spotlightInner")
		{
			return AnimableValuePtr(
				OGRE_NEW LightSpotlightInnerValue(this));
		}
		else if (valueName == "spotlightOuter")
		{
			return AnimableValuePtr(
				OGRE_NEW LightSpotlightOuterValue(this));
		}
		else if (valueName == "spotlightFalloff")
		{
			return AnimableValuePtr(
				OGRE_NEW LightSpotlightFalloffValue(this));
		}
		else
		{
			return MovableObject::createAnimableValue(valueName);
		}
	}
	//-----------------------------------------------------------------------
	void Light::setShadowFarDistance(Real distance)
	{
		mOwnShadowFarDist = true;
		mShadowFarDist = distance;
		mShadowFarDistSquared = distance * distance;
	}
	//-----------------------------------------------------------------------
	void Light::resetShadowFarDistance(void)
	{
		mOwnShadowFarDist = false;
	}
	//-----------------------------------------------------------------------
	Real Light::getShadowFarDistance(void) const
	{
		if (mOwnShadowFarDist)
			return mShadowFarDist;
		else
			return mManager->getShadowFarDistance ();
	}
	//-----------------------------------------------------------------------
	Real Light::getShadowFarDistanceSquared(void) const
	{
		if (mOwnShadowFarDist)
			return mShadowFarDistSquared;
		else
			return mManager->getShadowFarDistanceSquared ();
	}
	//---------------------------------------------------------------------
	Real Light::_deriveShadowNearClipDistance(const Camera* maincam) const
	{
		if (mShadowNearClipDist > 0)
			return mShadowNearClipDist;
		else
			return maincam->getNearClipDistance();
	}
	//---------------------------------------------------------------------
	Real Light::_deriveShadowFarClipDistance(const Camera* maincam) const
	{
		if (mShadowFarClipDist >= 0)
			return mShadowFarClipDist;
		else
		{
			if (mLightType == LT_DIRECTIONAL)
				return 0;
			else
				return mRange;
		}
	}
	//-----------------------------------------------------------------------
	void Light::setCustomParameter(uint16 index, const Ogre::Vector4 &value)
	{
		mCustomParameters[index] = value;
	}
	//-----------------------------------------------------------------------
	const Vector4 &Light::getCustomParameter(uint16 index) const
	{
		CustomParameterMap::const_iterator i = mCustomParameters.find(index);
        if (i != mCustomParameters.end())
        {
            return i->second;
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Parameter at the given index was not found.",
                "Light::getCustomParameter");
        }
	}
	//-----------------------------------------------------------------------
	void Light::_updateCustomGpuParameter(uint16 paramIndex, const GpuProgramParameters::AutoConstantEntry& constantEntry, GpuProgramParameters *params) const
	{
		CustomParameterMap::const_iterator i = mCustomParameters.find(paramIndex);
        if (i != mCustomParameters.end())
        {
            params->_writeRawConstant(constantEntry.physicalIndex, i->second, 
				constantEntry.elementCount);
        }
	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	String LightFactory::FACTORY_TYPE_NAME = "Light";
	//-----------------------------------------------------------------------
	const String& LightFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------
	MovableObject* LightFactory::createInstanceImpl( IdType id,
											ObjectMemoryManager *objectMemoryManager,
											const NameValuePairList* params )
	{

		Light* light = OGRE_NEW Light( id, objectMemoryManager );
 
		if(params)
		{
			NameValuePairList::const_iterator ni;

			// Setting the light type first before any property specific to a certain light type
			if ((ni = params->find("type")) != params->end())
			{
				if (ni->second == "point")
					light->setType(Light::LT_POINT);
				else if (ni->second == "directional")
					light->setType(Light::LT_DIRECTIONAL);
				else if (ni->second == "spotlight")
					light->setType(Light::LT_SPOTLIGHT);
				else
					OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Invalid light type '" + ni->second + "'.",
						"LightFactory::createInstance");
			}
			if ((ni = params->find("diffuseColour")) != params->end())
				light->setDiffuseColour(StringConverter::parseColourValue(ni->second));

			if ((ni = params->find("specularColour")) != params->end())
				light->setSpecularColour(StringConverter::parseColourValue(ni->second));

			if ((ni = params->find("attenuation")) != params->end())
			{
				Vector4 attenuation = StringConverter::parseVector4(ni->second);
				light->setAttenuation(attenuation.x, attenuation.y, attenuation.z, attenuation.w);
			}

			if ((ni = params->find("castShadows")) != params->end())
				light->setCastShadows(StringConverter::parseBool(ni->second));

			if ((ni = params->find("visible")) != params->end())
				light->setVisible(StringConverter::parseBool(ni->second));

			if ((ni = params->find("powerScale")) != params->end())
				light->setPowerScale(StringConverter::parseReal(ni->second));

			if ((ni = params->find("shadowFarDistance")) != params->end())
				light->setShadowFarDistance(StringConverter::parseReal(ni->second));


			// Spotlight properties
			if ((ni = params->find("spotlightInner")) != params->end())
				light->setSpotlightInnerAngle(StringConverter::parseAngle(ni->second));

			if ((ni = params->find("spotlightOuter")) != params->end())
				light->setSpotlightOuterAngle(StringConverter::parseAngle(ni->second));

			if ((ni = params->find("spotlightFalloff")) != params->end())
				light->setSpotlightFalloff(StringConverter::parseReal(ni->second));
		}

		return light;
	}
	//-----------------------------------------------------------------------
	void LightFactory::destroyInstance( MovableObject* obj)
	{
		OGRE_DELETE obj;
	}




} // Namespace
