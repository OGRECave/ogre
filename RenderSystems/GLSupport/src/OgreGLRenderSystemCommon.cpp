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
#include "OgreGLRenderSystemCommon.h"
#include "OgreGLContext.h"
#include "OgreFrustum.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
#include <OgreEGLWindow.h>
#endif

namespace Ogre {
    void GLRenderSystemCommon::_makeProjectionMatrix(const Radian& fovy, Real aspect,
                                                    Real nearPlane, Real farPlane,
                                                    Matrix4& dest, bool forGpuProgram)
    {
        Radian thetaY(fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);

        // Calc matrix elements
        Real w = (1.0f / tanThetaY) / aspect;
        Real h = 1.0f / tanThetaY;
        Real q, qn;
        if (farPlane == 0)
        {
            // Infinite far plane
            q = Frustum::INFINITE_FAR_PLANE_ADJUST - 1;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 2);
        }
        else
        {
            q = -(farPlane + nearPlane) / (farPlane - nearPlane);
            qn = -2 * (farPlane * nearPlane) / (farPlane - nearPlane);
        }

        // NB This creates Z in range [-1,1]
        //
        // [ w   0   0   0  ]
        // [ 0   h   0   0  ]
        // [ 0   0   q   qn ]
        // [ 0   0   -1  0  ]

        dest = Matrix4::ZERO;
        dest[0][0] = w;
        dest[1][1] = h;
        dest[2][2] = q;
        dest[2][3] = qn;
        dest[3][2] = -1;
    }

    void GLRenderSystemCommon::_makeProjectionMatrix(Real left, Real right,
                                                    Real bottom, Real top,
                                                    Real nearPlane, Real farPlane,
                                                    Matrix4& dest, bool forGpuProgram)
    {
        Real width = right - left;
        Real height = top - bottom;
        Real q, qn;
        if (farPlane == 0)
        {
            // Infinite far plane
            q = Frustum::INFINITE_FAR_PLANE_ADJUST - 1;
            qn = nearPlane * (Frustum::INFINITE_FAR_PLANE_ADJUST - 2);
        }
        else
        {
            q = -(farPlane + nearPlane) / (farPlane - nearPlane);
            qn = -2 * (farPlane * nearPlane) / (farPlane - nearPlane);
        }

        dest = Matrix4::ZERO;
        dest[0][0] = 2 * nearPlane / width;
        dest[0][2] = (right+left) / width;
        dest[1][1] = 2 * nearPlane / height;
        dest[1][2] = (top+bottom) / height;
        dest[2][2] = q;
        dest[2][3] = qn;
        dest[3][2] = -1;
    }

    void GLRenderSystemCommon::_makeOrthoMatrix(const Radian& fovy, Real aspect,
                                               Real nearPlane, Real farPlane,
                                               Matrix4& dest, bool forGpuProgram)
    {
        Radian thetaY(fovy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);

        // Real thetaX = thetaY * aspect;
        Real tanThetaX = tanThetaY * aspect; // Math::Tan(thetaX);
        Real half_w = tanThetaX * nearPlane;
        Real half_h = tanThetaY * nearPlane;
        Real iw = 1.0f / half_w;
        Real ih = 1.0f / half_h;
        Real q;
        if (farPlane == 0)
        {
            q = 0;
        }
        else
        {
            q = 2.0f / (farPlane - nearPlane);
        }
        dest = Matrix4::ZERO;
        dest[0][0] = iw;
        dest[1][1] = ih;
        dest[2][2] = -q;
        dest[2][3] = -(farPlane + nearPlane) / (farPlane - nearPlane);
        dest[3][3] = 1;
    }

    void GLRenderSystemCommon::_applyObliqueDepthProjection(Matrix4& matrix,
                                                           const Plane& plane,
                                                           bool forGpuProgram)
    {
        // Thanks to Eric Lenyel for posting this calculation at www.terathon.com

        // Calculate the clip-space corner point opposite the clipping plane
        // as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
        // transform it into camera space by multiplying it
        // by the inverse of the projection matrix

        Vector4 q;
        q.x = (Math::Sign(plane.normal.x) + matrix[0][2]) / matrix[0][0];
        q.y = (Math::Sign(plane.normal.y) + matrix[1][2]) / matrix[1][1];
        q.z = -1.0F;
        q.w = (1.0F + matrix[2][2]) / matrix[2][3];

        // Calculate the scaled plane vector
        Vector4 clipPlane4d(plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
        Vector4 c = clipPlane4d * (2.0F / (clipPlane4d.dotProduct(q)));

        // Replace the third row of the projection matrix
        matrix[2][0] = c.x;
        matrix[2][1] = c.y;
        matrix[2][2] = c.z + 1.0F;
        matrix[2][3] = c.w;
    }

    void GLRenderSystemCommon::_completeDeferredVaoFboDestruction()
    {
        if(GLContext* ctx = mCurrentContext)
        {
            vector<uint32>::type& vaos = ctx->_getVaoDeferredForDestruction();
            while(!vaos.empty())
            {
                _destroyVao(ctx, vaos.back());
                vaos.pop_back();
            }
            
            vector<uint32>::type& fbos = ctx->_getFboDeferredForDestruction();
            while(!fbos.empty())
            {
                _destroyFbo(ctx, fbos.back());
                fbos.pop_back();
            }

        }
    }

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
    void GLRenderSystemCommon::_destroyInternalResources(RenderWindow* pRenderWnd)
    {
        static_cast<EGLWindow*>(pRenderWnd)->_notifySurfaceDestroyed();
    }

    void GLRenderSystemCommon::_createInternalResources(RenderWindow* pRenderWnd, void* window,
                                                        void* config)
    {
#if OGRE_PLATFORM != OGRE_PLATFORM_EMSCRIPTEN
        static_cast<EGLWindow*>(pRenderWnd)
            ->_notifySurfaceCreated(reinterpret_cast<EGLNativeWindowType>(window), config);
#endif
    }
#endif
}
