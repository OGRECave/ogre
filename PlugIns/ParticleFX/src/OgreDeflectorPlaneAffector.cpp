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
#include "OgreDeflectorPlaneAffector.h"
#include "OgreParticleSystem.h"
#include "OgreParticle.h"
#include "OgreStringConverter.h"


namespace Ogre {

    // Instantiate statics
    DeflectorPlaneAffector::CmdPlanePoint DeflectorPlaneAffector::msPlanePointCmd;
    DeflectorPlaneAffector::CmdPlaneNormal DeflectorPlaneAffector::msPlaneNormalCmd;
    DeflectorPlaneAffector::CmdBounce DeflectorPlaneAffector::msBounceCmd;

    //-----------------------------------------------------------------------
    DeflectorPlaneAffector::DeflectorPlaneAffector(ParticleSystem* psys)
        : ParticleAffector(psys)
    {
        mType = "DeflectorPlane";

        // defaults
        mPlanePoint = Vector3::ZERO;
        mPlaneNormal = Vector3::UNIT_Y;
        mBounce = 1.0;

        // Set up parameters
        if (createParamDictionary("DeflectorPlaneAffector"))
        {
            addBaseParameters();
            // Add extra paramaters
            ParamDictionary* dict = getParamDictionary();
            dict->addParameter(ParameterDef("plane_point",
                "A point on the deflector plane. Together with the normal vector it defines the plane.",
                PT_VECTOR3), &msPlanePointCmd);
            dict->addParameter(ParameterDef("plane_normal",
                "The normal vector of the deflector plane. Together with the point it defines the plane.",
                PT_VECTOR3), &msPlaneNormalCmd);
            dict->addParameter(ParameterDef("bounce",
                "The amount of bouncing when a particle is deflected. 0 means no deflection and 1 stands for 100 percent reflection.",
                PT_REAL), &msBounceCmd);
        }
    }
    //-----------------------------------------------------------------------
    void DeflectorPlaneAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        // precalculate distance of plane from origin
        Real planeDistance = - mPlaneNormal.dotProduct(mPlanePoint) / Math::Sqrt(mPlaneNormal.dotProduct(mPlaneNormal));
		Vector3 directionPart;

        ParticleIterator pi = pSystem->_getIterator();
        Particle *p;

        while (!pi.end())
        {
            p = pi.getNext();

            Vector3 direction(p->direction * timeElapsed);
            if (mPlaneNormal.dotProduct(p->position + direction) + planeDistance <= 0.0)
            {
                Real a = mPlaneNormal.dotProduct(p->position) + planeDistance;
                if (a > 0.0)
                {
                    // for intersection point
					directionPart = direction * (- a / direction.dotProduct( mPlaneNormal ));
                    // set new position
					p->position = (p->position + ( directionPart )) + (((directionPart) - direction) * mBounce);

                    // reflect direction vector
                    p->direction = (p->direction - (2.0 * p->direction.dotProduct( mPlaneNormal ) * mPlaneNormal)) * mBounce;
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void DeflectorPlaneAffector::setPlanePoint(const Vector3& pos)
    {
        mPlanePoint = pos;
    }
    //-----------------------------------------------------------------------
    void DeflectorPlaneAffector::setPlaneNormal(const Vector3& normal)
    {
        mPlaneNormal = normal;
    }
    //-----------------------------------------------------------------------
    void DeflectorPlaneAffector::setBounce(Real bounce)
    {
        mBounce = bounce;
    }

    //-----------------------------------------------------------------------
    Vector3 DeflectorPlaneAffector::getPlanePoint(void) const
    {
        return mPlanePoint;
    }
    //-----------------------------------------------------------------------
    Vector3 DeflectorPlaneAffector::getPlaneNormal(void) const
    {
        return mPlaneNormal;
    }
    //-----------------------------------------------------------------------
    Real DeflectorPlaneAffector::getBounce(void) const
    {
        return mBounce;
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String DeflectorPlaneAffector::CmdPlanePoint::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const DeflectorPlaneAffector*>(target)->getPlanePoint() );
    }
    void DeflectorPlaneAffector::CmdPlanePoint::doSet(void* target, const String& val)
    {
        static_cast<DeflectorPlaneAffector*>(target)->setPlanePoint(
            StringConverter::parseVector3(val));
    }
    //-----------------------------------------------------------------------
    String DeflectorPlaneAffector::CmdPlaneNormal::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const DeflectorPlaneAffector*>(target)->getPlaneNormal() );
    }
    void DeflectorPlaneAffector::CmdPlaneNormal::doSet(void* target, const String& val)
    {
        static_cast<DeflectorPlaneAffector*>(target)->setPlaneNormal(
            StringConverter::parseVector3(val));
    }
    //-----------------------------------------------------------------------
    String DeflectorPlaneAffector::CmdBounce::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const DeflectorPlaneAffector*>(target)->getBounce() );

    }
    void DeflectorPlaneAffector::CmdBounce::doSet(void* target, const String& val)
    {
        static_cast<DeflectorPlaneAffector*>(target)->setBounce(
            StringConverter::parseReal(val));
    }

}
