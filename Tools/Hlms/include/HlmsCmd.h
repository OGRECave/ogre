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

#ifndef _OgreHlmsCmd_H_
#define _OgreHlmsCmd_H_

#include <OgrePrerequisites.h>
#include <OgreCommon.h>

class HlmsCmd
{
    struct Options
    {
        Ogre::String language;
        Ogre::String generator;
    };

    Ogre::Root          *mRoot;
    Ogre::Camera        *mCamera;
    Ogre::SceneManager  *mSceneMgr;
    Ogre::RenderWindow  *mWindow;
    Ogre::CompositorWorkspace *mWorkspace;
    Ogre::String        mResourcesCfg;
    Ogre::String        mPluginsCfg;

    Options             mOpts;

protected:
     bool setup();
     bool configure(void);
     void chooseSceneManager(void);
     void createCamera(void);
     void createScene(void);
     void destroyScene(void);
     void createCompositor(void);
     void setupResources(void);
     void createResourceListener(void);
     void loadResources(void);

    void parseOpts( Ogre::UnaryOptionList &unOpts, Ogre::BinaryOptionList &binOpts );

public:
    HlmsCmd( int numargs, char** args );
    ~HlmsCmd(void);

    static void help(void);

    void go(void);
};

#endif
