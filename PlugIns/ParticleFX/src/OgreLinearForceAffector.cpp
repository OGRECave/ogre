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
#include "OgreLinearForceAffector.h"
#include "OgreParticleSystem.h"
#include "OgreParticle.h"
#include "OgreStringConverter.h"


namespace Ogre {

    // Instantiate statics
    LinearForceAffector::CmdForceVector LinearForceAffector::msForceVectorCmd;
    LinearForceAffector::CmdForceApp LinearForceAffector::msForceAppCmd;


    //-----------------------------------------------------------------------
    LinearForceAffector::LinearForceAffector(ParticleSystem* psys)
        :ParticleAffector(psys)
    {
        mType = "LinearForce";

        // Default to gravity-like
        mForceApplication = FA_ADD;
        mForceVector.x = mForceVector.z = 0;
        mForceVector.y = -100;

        // Set up parameters
        if (createParamDictionary("LinearForceAffector"))
        {
            addBaseParameters();
            // Add extra parameters
            ParamDictionary* dict = getParamDictionary();
            dict->addParameter(ParameterDef("force_vector", 
                "The vector representing the force to apply.",
                PT_VECTOR3),&msForceVectorCmd);
            dict->addParameter(ParameterDef("force_application", 
                "How to apply the force vector to particles.",
                PT_STRING),&msForceAppCmd);

        }

    }
    //-----------------------------------------------------------------------
    void LinearForceAffector::_affectParticles(ParticleSystem* pSystem, Real timeElapsed)
    {
        ParticleIterator pi = pSystem->_getIterator();
        Particle *p;

        Vector3 scaledVector = Vector3::ZERO;

        // Precalc scaled force for optimisation
        if (mForceApplication == FA_ADD)
        {
            // Scale force by time
            scaledVector = mForceVector * timeElapsed;
        }

        while (!pi.end())
        {
            p = pi.getNext();
            if (mForceApplication == FA_ADD)
            {
                p->direction += scaledVector;
            }
            else // FA_AVERAGE
            {
                p->direction = (p->direction + mForceVector) / 2;
            }
        }
        
    }
    //-----------------------------------------------------------------------
    void LinearForceAffector::setForceVector(const Vector3& force)
    {
        mForceVector = force;
    }
    //-----------------------------------------------------------------------
    void LinearForceAffector::setForceApplication(ForceApplication fa)
    {
        mForceApplication = fa;
    }
    //-----------------------------------------------------------------------
    Vector3 LinearForceAffector::getForceVector(void) const
    {
        return mForceVector;
    }
    //-----------------------------------------------------------------------
    LinearForceAffector::ForceApplication LinearForceAffector::getForceApplication(void) const
    {
        return mForceApplication;
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String LinearForceAffector::CmdForceVector::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const LinearForceAffector*>(target)->getForceVector() );
    }
    void LinearForceAffector::CmdForceVector::doSet(void* target, const String& val)
    {
        static_cast<LinearForceAffector*>(target)->setForceVector(
            StringConverter::parseVector3(val));
    }
    //-----------------------------------------------------------------------
    String LinearForceAffector::CmdForceApp::doGet(const void* target) const
    {
        ForceApplication app = static_cast<const LinearForceAffector*>(target)->getForceApplication();
        switch(app)
        {
        case LinearForceAffector::FA_AVERAGE:
            return "average";
            break;
        case LinearForceAffector::FA_ADD:
            return "add";
            break;
        }
        // Compiler nicety
        return "";
    }
    void LinearForceAffector::CmdForceApp::doSet(void* target, const String& val)
    {
        if (val == "average")
        {
            static_cast<LinearForceAffector*>(target)->setForceApplication(FA_AVERAGE);
        }
        else if (val == "add")
        {
            static_cast<LinearForceAffector*>(target)->setForceApplication(FA_ADD);
        }
    }


}

