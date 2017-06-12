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
#ifndef _OgreGL3PlusHlmsPso_H_
#define _OgreGL3PlusHlmsPso_H_

#include "OgreGL3PlusPrerequisites.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Resources
    *  @{
    */

    /// @See HlmsPso. We cache some conversion values here
    /// to avoid doing it every time we set the Pso
    struct _OgreGL3PlusExport GL3PlusHlmsPso
    {
        //Macroblock data
        GLboolean   depthWrite;
        GLenum      depthFunc;
        GLenum      cullMode;
        GLenum      polygonMode;

        //Blendblock data
        bool    enableAlphaBlend;
        GLenum sourceBlend;
        GLenum destBlend;
        GLenum sourceBlendAlpha;
        GLenum destBlendAlpha;
        GLenum blendFunc;
        GLenum blendFuncAlpha;

        //Shader
        GLSLShader *vertexShader;
        GLSLShader *geometryShader;
        GLSLShader *hullShader;
        GLSLShader *domainShader;
        GLSLShader *pixelShader;
    };

    /** @} */
    /** @} */

}

#include "OgreHeaderSuffix.h"

#endif
