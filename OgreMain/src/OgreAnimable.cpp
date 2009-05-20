/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreStableHeaders.h"
#include "OgreAnimable.h"

namespace Ogre {
	//--------------------------------------------------------------------------
	AnimableObject::AnimableDictionaryMap AnimableObject::msAnimableDictionary;
	//--------------------------------------------------------------------------
	void AnimableValue::resetToBaseValue(void)
	{
		switch(mType)
		{
		case INT:
			setValue(mBaseValueInt);
			break;
		case REAL:
			setValue(mBaseValueReal[0]);
			break;
		case VECTOR2:
			setValue(Vector2(mBaseValueReal));
			break;
		case VECTOR3:
			setValue(Vector3(mBaseValueReal));
			break;
		case VECTOR4:
			setValue(Vector4(mBaseValueReal));
			break;
		case QUATERNION:
			setValue(Quaternion(mBaseValueReal));
			break;
		case COLOUR:
			setValue(ColourValue(mBaseValueReal[0], mBaseValueReal[1], 
				mBaseValueReal[2], mBaseValueReal[3]));
			break;
		case DEGREE:
			setValue(Degree(mBaseValueReal[0]));
			break;
		case RADIAN:		
			setValue(Radian(mBaseValueReal[0]));
			break;
		}
	}
	//--------------------------------------------------------------------------
	void AnimableValue::setAsBaseValue(const Any& val)
	{
		switch(mType)
		{
		case INT:
			setAsBaseValue(any_cast<int>(val));
			break;
		case REAL:
			setAsBaseValue(any_cast<Real>(val));
			break;
		case VECTOR2:
			setAsBaseValue(any_cast<Vector2>(val));
			break;
		case VECTOR3:
			setAsBaseValue(any_cast<Vector3>(val));
			break;
		case VECTOR4:
			setAsBaseValue(any_cast<Vector4>(val));
			break;
		case QUATERNION:
			setAsBaseValue(any_cast<Quaternion>(val));
			break;
		case COLOUR:
			setAsBaseValue(any_cast<ColourValue>(val));
			break;
		case DEGREE:
			setAsBaseValue(any_cast<Degree>(val));
			break;
		case RADIAN:
			setAsBaseValue(any_cast<Radian>(val));
			break;
		}
	}
	//--------------------------------------------------------------------------
	void AnimableValue::setValue(const Any& val)
	{
		switch(mType)
		{
		case INT:
			setValue(any_cast<int>(val));
			break;
		case REAL:
			setValue(any_cast<Real>(val));
			break;
		case VECTOR2:
			setValue(any_cast<Vector2>(val));
			break;
		case VECTOR3:
			setValue(any_cast<Vector3>(val));
			break;
		case VECTOR4:
			setValue(any_cast<Vector4>(val));
			break;
		case QUATERNION:
			setValue(any_cast<Quaternion>(val));
			break;
		case COLOUR:
			setValue(any_cast<ColourValue>(val));
			break;
		case RADIAN:
			setValue(any_cast<Radian>(val));
			break;
		case DEGREE:
			setValue(any_cast<Degree>(val));
			break;
		}
	}
	//--------------------------------------------------------------------------
	void AnimableValue::applyDeltaValue(const Any& val)
	{
		switch(mType)
		{
		case INT:
			applyDeltaValue(any_cast<int>(val));
			break;
		case REAL:
			applyDeltaValue(any_cast<Real>(val));
			break;
		case VECTOR2:
			applyDeltaValue(any_cast<Vector2>(val));
			break;
		case VECTOR3:
			applyDeltaValue(any_cast<Vector3>(val));
			break;
		case VECTOR4:
			applyDeltaValue(any_cast<Vector4>(val));
			break;
		case QUATERNION:
			applyDeltaValue(any_cast<Quaternion>(val));
			break;
		case COLOUR:
			applyDeltaValue(any_cast<ColourValue>(val));
			break;
		case DEGREE:
			applyDeltaValue(any_cast<Degree>(val));
			break;
		case RADIAN:
			applyDeltaValue(any_cast<Radian>(val));
			break;
		}
	}



}
