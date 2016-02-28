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
        : ParticleAffector(psys), mPlanePoint(Vector3::ZERO), mPlaneNormal(Vector3::UNIT_Y), mBounce(1.0)
    {
        mType = "DeflectorPlane";

        // Set up parameters
        if (createParamDictionary("DeflectorPlaneAffector"))
        {
            addBaseParameters();
            // Add extra parameters
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

        while (!pi.end())
        {
            Particle *p = pi.getNext();

            Vector3 direction(p->mDirection * timeElapsed);
            if (mPlaneNormal.dotProduct(p->mPosition + direction) + planeDistance <= 0.0)
            {
                Real a = mPlaneNormal.dotProduct(p->mPosition) + planeDistance;
                if (a > 0.0)
                {
                    // for intersection point
                    directionPart = direction * (- a / direction.dotProduct( mPlaneNormal ));
                    // set new position
                    p->mPosition = (p->mPosition + ( directionPart )) + (((directionPart) - direction) * mBounce);

                    // reflect direction vector
                    p->mDirection = (p->mDirection - (2.0f * p->mDirection.dotProduct( mPlaneNormal ) * mPlaneNormal)) * mBounce;
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
