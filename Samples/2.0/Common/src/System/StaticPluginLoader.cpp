/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2016 Torus Knot Software Ltd

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

#include "System/StaticPluginLoader.h"

#ifdef OGRE_STATIC_LIB
    #ifdef OGRE_BUILD_RENDERSYSTEM_GL3PLUS
        #include "OgreGL3PlusPlugin.h"
    #endif
    #ifdef OGRE_BUILD_RENDERSYSTEM_D3D11
        #include "OgreD3D11Plugin.h"
    #endif
    #ifdef OGRE_BUILD_RENDERSYSTEM_METAL
        #include "OgreMetalPlugin.h"
    #endif
#endif
#include "OgreRoot.h"

namespace Demo
{
    StaticPluginLoader::StaticPluginLoader()
#if defined( OGRE_STATIC_LIB )
        :
        mDummy( 0 )
    #ifdef OGRE_BUILD_RENDERSYSTEM_GL3PLUS
    ,   mGL3PlusPlugin( 0 )
    #endif
    #ifdef OGRE_BUILD_RENDERSYSTEM_D3D11
    ,   mD3D11PlusPlugin( 0 )
    #endif
    #ifdef OGRE_BUILD_RENDERSYSTEM_METAL
    ,   mMetalPlugin( 0 )
    #endif
#endif
    {
    }
    //-----------------------------------------------------------------------------------
    StaticPluginLoader::~StaticPluginLoader()
    {
#ifdef OGRE_STATIC_LIB
    #ifdef OGRE_BUILD_RENDERSYSTEM_GL3PLUS
        OGRE_DELETE mGL3PlusPlugin;
        mGL3PlusPlugin = 0;
    #endif
    #ifdef OGRE_BUILD_RENDERSYSTEM_D3D11
        OGRE_DELETE mD3D11PlusPlugin;
        mD3D11PlusPlugin = 0;
    #endif
    #ifdef OGRE_BUILD_RENDERSYSTEM_METAL
        OGRE_DELETE mMetalPlugin;
        mMetalPlugin = 0;
    #endif
#endif
    }
    //-----------------------------------------------------------------------------------
    void StaticPluginLoader::install( Ogre::Root *root )
    {
#ifdef OGRE_STATIC_LIB
    #ifdef OGRE_BUILD_RENDERSYSTEM_GL3PLUS
        if( !mGL3PlusPlugin )
            mGL3PlusPlugin = OGRE_NEW Ogre::GL3PlusPlugin();
        root->installPlugin( mGL3PlusPlugin );
    #endif
    #ifdef OGRE_BUILD_RENDERSYSTEM_D3D11
        if( !mD3D11PlusPlugin )
            mD3D11PlusPlugin = OGRE_NEW Ogre::D3D11Plugin();
        root->installPlugin( mD3D11PlusPlugin );
    #endif
    #ifdef OGRE_BUILD_RENDERSYSTEM_METAL
        if( !mMetalPlugin )
            mMetalPlugin = OGRE_NEW Ogre::MetalPlugin();
        root->installPlugin( mMetalPlugin );
    #endif
#endif
    }
}
