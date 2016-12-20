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

#include "OgreStableHeaders.h"

#include "CommandBuffer/OgreCommandBuffer.h"
#include "CommandBuffer/OgreCbLowLevelMaterial.h"

#include "OgreHlmsLowLevel.h"

namespace Ogre
{
    CbLowLevelMaterial::CbLowLevelMaterial( bool _casterPass, HlmsLowLevel *_hlmsLowLevel,
                                            const MovableObject *_movableObject,
                                            Renderable *_renderable ) :
        CbBase( CB_LOW_LEVEL_MATERIAL ),
        casterPass( _casterPass ),
        hlmsLowLevel( _hlmsLowLevel ),
        movableObject( _movableObject ),
        renderable( _renderable )
    {
    }

    void CommandBuffer::execute_lowLevelMaterial( CommandBuffer *_this, const CbBase * RESTRICT_ALIAS _cmd )
    {
        const CbLowLevelMaterial *cmd = static_cast<const CbLowLevelMaterial*>( _cmd );
        cmd->hlmsLowLevel->executeCommand( cmd->movableObject, cmd->renderable, cmd->casterPass );
    }
}
