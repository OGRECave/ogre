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
#include "OgreGLNativeSupport.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_ANDROID || OGRE_PLATFORM == OGRE_PLATFORM_EMSCRIPTEN
#include <OgreEGLWindow.h>
#endif

namespace Ogre {
    static void removeDuplicates(std::vector<String>& c)
    {
        std::sort(c.begin(), c.end());
        auto p = std::unique(c.begin(), c.end());
        c.erase(p, c.end());
    }

    String GLRenderSystemCommon::VideoMode::getDescription() const
    {
        return StringUtil::format("%4d x %4d", width, height);
    }

    void GLRenderSystemCommon::initConfigOptions()
    {
        mOptions = mGLSupport->getConfigOptions();

        RenderSystem::initConfigOptions();

        ConfigOption optDisplayFrequency;
        optDisplayFrequency.name = "Display Frequency";
        optDisplayFrequency.immutable = false;
        mOptions[optDisplayFrequency.name] = optDisplayFrequency;

        ConfigOption optVideoMode;
        optVideoMode.name = "Video Mode";
        optVideoMode.immutable = false;
        for (const auto& mode : mGLSupport->getVideoModes())
        {
            optVideoMode.possibleValues.push_back(mode.getDescription());
        }
        removeDuplicates(optVideoMode.possibleValues); // also sorts
        optVideoMode.currentValue = optVideoMode.possibleValues[0];
        mOptions[optVideoMode.name] = optVideoMode;

        ConfigOption optFSAA;
        optFSAA.name = "FSAA";
        optFSAA.immutable = false;
        for (int sampleLevel : mGLSupport->getFSAALevels())
        {
            optFSAA.possibleValues.push_back(StringConverter::toString(sampleLevel));
        }
        if (!optFSAA.possibleValues.empty())
        {
            removeDuplicates(optFSAA.possibleValues);
            optFSAA.currentValue = optFSAA.possibleValues[0];
        }
        mOptions[optFSAA.name] = optFSAA;

        // TODO remove this on next release
        ConfigOption optRTTMode;
        optRTTMode.name = "RTT Preferred Mode";
        optRTTMode.possibleValues.push_back("FBO");
        optRTTMode.currentValue = optRTTMode.possibleValues[0];
        optRTTMode.immutable = true;
        mOptions[optRTTMode.name] = optRTTMode;

        refreshConfig();
    }

    void GLRenderSystemCommon::refreshConfig()
    {
        // set bpp and refresh rate as appropriate
        ConfigOptionMap::iterator optVideoMode = mOptions.find("Video Mode");
        ConfigOptionMap::iterator optDisplayFrequency = mOptions.find("Display Frequency");
        ConfigOptionMap::iterator optFullScreen = mOptions.find("Full Screen");
        ConfigOptionMap::iterator optColourDepth = mOptions.find("Colour Depth");

        // coulour depth is optional
        if (optColourDepth != mOptions.end())
        {
            for (const auto& mode : mGLSupport->getVideoModes())
            {
                if (mode.getDescription() == optVideoMode->second.currentValue)
                {
                    optColourDepth->second.possibleValues.push_back(
                        StringConverter::toString((unsigned int)mode.bpp));
                }
            }

            removeDuplicates(optColourDepth->second.possibleValues);
        }

        // we can only set refresh rate in full screen mode
        bool isFullscreen = false;
        if( optFullScreen != mOptions.end())
            isFullscreen = optFullScreen->second.currentValue == "Yes";

        if (optVideoMode == mOptions.end() || optDisplayFrequency == mOptions.end())
            return;

        optDisplayFrequency->second.possibleValues.clear();
        if( !isFullscreen )
        {
            optDisplayFrequency->second.possibleValues.push_back( "N/A" );
        }
        else
        {
            for (const auto& mode : mGLSupport->getVideoModes())
            {
                if (mode.getDescription() == optVideoMode->second.currentValue)
                {
                    String frequency = StringConverter::toString(mode.refreshRate) + " Hz";
                    optDisplayFrequency->second.possibleValues.push_back(frequency);

                    if(optColourDepth != mOptions.end())
                        optColourDepth->second.possibleValues.push_back(
                            StringConverter::toString((unsigned int)mode.bpp));
                }
            }

            removeDuplicates(optDisplayFrequency->second.possibleValues);
        }

        if (!optDisplayFrequency->second.possibleValues.empty())
        {
            optDisplayFrequency->second.currentValue = optDisplayFrequency->second.possibleValues[0];
        }
        else
        {
            optVideoMode->second.currentValue = mGLSupport->getVideoModes()[0].getDescription();
            optDisplayFrequency->second.currentValue = StringConverter::toString(mGLSupport->getVideoModes()[0].refreshRate) + " Hz";
        }
    }

    //-------------------------------------------------------------------------------------------------//
    NameValuePairList GLRenderSystemCommon::parseOptions(uint& w, uint& h, bool& fullscreen)
    {
        ConfigOptionMap::iterator opt;
        ConfigOptionMap::iterator end = mOptions.end();
        NameValuePairList miscParams;

        opt = mOptions.find("Full Screen");
        if (opt == mOptions.end())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find full screen options!", "parseOptions");
        fullscreen = opt->second.currentValue == "Yes";

        opt = mOptions.find("Video Mode");
        if (opt == mOptions.end())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find video mode options!", "parseOptions");
        String val = opt->second.currentValue;
        String::size_type pos = val.find('x');
        if (pos == String::npos)
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid Video Mode provided", "parseOptions");

        w = StringConverter::parseUnsignedInt(val.substr(0, pos));
        h = StringConverter::parseUnsignedInt(val.substr(pos + 1));

        if((opt = mOptions.find("Display Frequency")) != end)
            miscParams["displayFrequency"] = opt->second.currentValue;

        if((opt = mOptions.find("FSAA")) != end)
            miscParams["FSAA"] = opt->second.currentValue;

        if((opt = mOptions.find("VSync")) != end)
            miscParams["vsync"] = opt->second.currentValue;

        if((opt = mOptions.find("sRGB Gamma Conversion")) != end)
            miscParams["gamma"] = opt->second.currentValue;

        // backend specific options. Presence determined by getConfigOptions
        if((opt = mOptions.find("Colour Depth")) != end)
            miscParams["colourDepth"] = opt->second.currentValue;

        if((opt = mOptions.find("VSync Interval")) != end)
            miscParams["vsyncInterval"] = opt->second.currentValue;

        if((opt = mOptions.find("hidden")) != end)
            miscParams[ "hidden" ] = opt->second.currentValue;

        if((opt = mOptions.find("Content Scaling Factor")) != end)
            miscParams["contentScalingFactor"] = opt->second.currentValue;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        opt = mOptions.find("Stereo Mode");
        if (opt == mOptions.end())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find stereo enabled options!", "parseOptions");
        miscParams["stereoMode"] = opt->second.currentValue;
#endif

        return miscParams;
    }

    void GLRenderSystemCommon::setConfigOption(const String &name, const String &value)
    {
        ConfigOptionMap::iterator option = mOptions.find(name);
        if (option == mOptions.end()) {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Option named " + name + " does not exist.",
                        "GLNativeSupport::setConfigOption");
        }
        option->second.currentValue = value;

        if( name == "Video Mode" || name == "Full Screen" )
            refreshConfig();
    }

    bool GLRenderSystemCommon::checkExtension(const String& ext) const
    {
        return mExtensionList.find(ext) != mExtensionList.end() || mGLSupport->checkExtension(ext);
    }

    bool GLRenderSystemCommon::hasMinGLVersion(int major, int minor) const
    {
        if (mDriverVersion.major == major) {
            return mDriverVersion.minor >= minor;
        }
        return mDriverVersion.major > major;
    }

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
            std::vector<uint32>& vaos = ctx->_getVaoDeferredForDestruction();
            while(!vaos.empty())
            {
                _destroyVao(ctx, vaos.back());
                vaos.pop_back();
            }
            
            std::vector<uint32>& fbos = ctx->_getFboDeferredForDestruction();
            while(!fbos.empty())
            {
                _destroyFbo(ctx, fbos.back());
                fbos.pop_back();
            }

        }
    }
}
