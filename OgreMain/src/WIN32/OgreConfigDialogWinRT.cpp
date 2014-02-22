/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

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
#include "OgreConfigDialog.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreException.h"
#include "resource.h"

namespace
{
    Ogre::ConfigDialog* dlg;  // This is a pointer to instance, since this is a static member
}


namespace Ogre
{
    ConfigDialog::ConfigDialog()
    {
        mSelectedRenderSystem = 0;
    }

    ConfigDialog::~ConfigDialog()
    {
    }


    bool ConfigDialog::display(void)
    {
        if(Root::getSingleton().getRenderSystem() != NULL)
        {
            return true;
        }

        // just select the first available render system for now.
        const RenderSystemList* lstRend;
        RenderSystemList::const_iterator pRend;

        lstRend = &Root::getSingleton().getAvailableRenderers();
        pRend = lstRend->begin();            

        if (pRend != lstRend->end())
        {
            Root::getSingleton().setRenderSystem((*pRend));

            return true;
        }

        return false;
    }
}