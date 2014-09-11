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

#include "ParticleTest.h"

ParticleTest::ParticleTest()
{
    mInfo["Title"] = "VTests_Particles";
    mInfo["Description"] = "Tests basic particle system functionality.";
    // take screenshot early, when emitters are just beginning
    addScreenshotFrame(35);
    // and another after particles have died, extra emitters emitted, etc
    addScreenshotFrame(500);
}
//---------------------------------------------------------------------------

void ParticleTest::setupContent()
{
    // create a bunch of random particle systems
    Ogre::ParticleSystem* ps = mSceneMgr->createParticleSystem("Fireworks", "Examples/Fireworks");
    mSceneMgr->getRootSceneNode()->attachObject(ps);
    
    // Multiple systems didn't seem to stay deterministic... (in gcc at least...)
    //ps = mSceneMgr->createParticleSystem("Fountain", "Examples/PurpleFountain");
    //mSceneMgr->getRootSceneNode()->attachObject(ps);
    //ps = mSceneMgr->createParticleSystem("Nimbus", "Examples/GreenyNimbus");
    //mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(150, 0, 0))->attachObject(ps);
    //ps = mSceneMgr->createParticleSystem("Nimbus2", "Examples/GreenyNimbus");
    //mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-150, 0, 0))->attachObject(ps);
    mCamera->setPosition(0,150,500);
}
//-----------------------------------------------------------------------


