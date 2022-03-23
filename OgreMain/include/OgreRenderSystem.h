/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef __RenderSystem_H_
#define __RenderSystem_H_

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreTextureUnitState.h"
#include "OgreCommon.h"

#include "OgreRenderSystemCapabilities.h"
#include "OgreConfigOptionMap.h"
#include "OgreGpuProgram.h"
#include "OgrePlane.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHeaderPrefix.h"

namespace Ogre
{
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */

    typedef std::vector<DepthBuffer*> DepthBufferVec;
    typedef std::map< uint16, DepthBufferVec > DepthBufferMap;
    typedef std::map< String, RenderTarget * > RenderTargetMap;
    typedef std::multimap<uchar, RenderTarget * > RenderTargetPriorityMap;

    class TextureManager;
    /// Enum describing the ways to generate texture coordinates
    enum TexCoordCalcMethod
    {
        /// No calculated texture coordinates
        TEXCALC_NONE,
        /// Environment map based on vertex normals
        TEXCALC_ENVIRONMENT_MAP,
        /// Environment map based on vertex positions
        TEXCALC_ENVIRONMENT_MAP_PLANAR,
        TEXCALC_ENVIRONMENT_MAP_REFLECTION,
        TEXCALC_ENVIRONMENT_MAP_NORMAL,
        /// Projective texture
        TEXCALC_PROJECTIVE_TEXTURE
    };
    /// Enum describing the various actions which can be taken on the stencil buffer
    enum StencilOperation
    {
        /// Leave the stencil buffer unchanged
        SOP_KEEP,
        /// Set the stencil value to zero
        SOP_ZERO,
        /// Set the stencil value to the reference value
        SOP_REPLACE,
        /// Increase the stencil value by 1, clamping at the maximum value
        SOP_INCREMENT,
        /// Decrease the stencil value by 1, clamping at 0
        SOP_DECREMENT,
        /// Increase the stencil value by 1, wrapping back to 0 when incrementing the maximum value
        SOP_INCREMENT_WRAP,
        /// Decrease the stencil value by 1, wrapping when decrementing 0
        SOP_DECREMENT_WRAP,
        /// Invert the bits of the stencil buffer
        SOP_INVERT
    };

    /** Describes the stencil buffer operation

    The stencil buffer is used to mask out pixels in the render target, allowing
    you to do effects like mirrors, cut-outs, stencil shadows and more. Each of
    your batches of rendering is likely to ignore the stencil buffer,
    update it with new values, or apply it to mask the output of the render.

    The stencil test is:
    $$(referenceValue\\,\\&\\,compareMask)\\;compareOp\\;(stencilBuffer\\,\\&\\,compareMask)$$

    The result of this will cause one of 3 actions depending on whether
    1. the stencil test fails
    2. the stencil test succeeds but the depth buffer check fails
    3. both depth buffer check and stencil test pass
    */
    struct _OgreExport StencilState
    {
        /// Comparison operator for the stencil test
        CompareFunction compareOp;
        /// The action to perform when the stencil check fails
        StencilOperation stencilFailOp;
        /// The action to perform when the stencil check passes, but the depth buffer check fails
        StencilOperation depthFailOp;
        /// The action to take when both the stencil and depth check pass
        StencilOperation depthStencilPassOp;

        /// The reference value used in the stencil comparison
        uint32 referenceValue;
        ///  The bitmask applied to both the stencil value and the reference value before comparison
        uint32 compareMask;
        /** The bitmask the controls which bits from stencilRefValue will be written to stencil buffer
        (valid for operations such as #SOP_REPLACE) */
        uint32 writeMask;

        /// Turns stencil buffer checking on or off
        bool enabled : 1;
        /// Toggles two-sided stencil operation, which swaps increment and decrement for back-facing polygons.
        bool twoSidedOperation : 1;

        StencilState()
            : compareOp(CMPF_LESS_EQUAL), stencilFailOp(SOP_KEEP), depthFailOp(SOP_KEEP),
              depthStencilPassOp(SOP_KEEP), referenceValue(0), compareMask(0xFFFFFFFF),
              writeMask(0xFFFFFFFF), enabled(false), twoSidedOperation(false)
        {
        }
    };

    /** Defines the functionality of a 3D API
    @remarks
    The RenderSystem class provides a base interface
    which abstracts the general functionality of the 3D API
    e.g. Direct3D or OpenGL. Whilst a few of the general
    methods have implementations, most of this class is
    abstract, requiring a subclass based on a specific API
    to be constructed to provide the full functionality.
    Note there are 2 levels to the interface - one which
    will be used often by the caller of the Ogre library,
    and one which is at a lower level and will be used by the
    other classes provided by Ogre. These lower level
    methods are prefixed with '_' to differentiate them.
    The advanced user of the library may use these lower
    level methods to access the 3D API at a more fundamental
    level (dealing direct with render states and rendering
    primitives), but still benefiting from Ogre's abstraction
    of exactly which 3D API is in use.
    @author
    Steven Streeting
    @version
    1.0
    */
    class _OgreExport RenderSystem : public RenderSysAlloc
    {
    public:
        /** Default Constructor.
        */
        RenderSystem();

        /** Destructor.
        */
        virtual ~RenderSystem();

        /** Returns the name of the rendering system.
        */
        virtual const String& getName(void) const = 0;

        /** Returns the details of this API's configuration options
        @remarks
        Each render system must be able to inform the world
        of what options must/can be specified for it's
        operation.
        @par
        These are passed as strings for portability, but
        grouped into a structure (ConfigOption) which includes
        both options and current value.
        @par
        Note that the settings returned from this call are
        affected by the options that have been set so far,
        since some options are interdependent.
        @par
        This routine is called automatically by the default
        configuration dialogue produced by Root::showConfigDialog
        or may be used by the caller for custom settings dialogs
        @return
        A 'map' of options, i.e. a list of options which is also
        indexed by option name.
        */
        const ConfigOptionMap& getConfigOptions() const { return mOptions; }

        /** Sets an option for this API

        Used to confirm the settings (normally chosen by the user) in
        order to make the renderer able to initialise with the settings as required.
        This may initialise the @ref RenderWindowDescription or set some RenderSystem
        specific parameters.
        Called automatically by the default configuration
        dialog, and by the restoration of saved settings.
        These settings are stored and only activated when
        @ref RenderSystem::_initialise or @ref RenderSystem::reinitialise
        are called.

        If using a custom configuration dialog, it is advised that the
        caller calls RenderSystem::getConfigOptions
        again, since some options can alter resulting from a selection.

        Common options:

        | Key |  Default | Description |
        |-----|---------------|---------|
        | Full Screen | false | Window full-screen flag |
        | VSync | true | "vsync" in  @ref _createRenderWindow |
        | VSync Interval | 1 | "vsyncInterval" in  @ref _createRenderWindow |
        | sRGB Gamma Conversion | false | "gamma" in  @ref _createRenderWindow  |
        | FSAA | 0 | concatenation of "FSAA" and "FSAAHint" as in  @ref _createRenderWindow  |
        | Video Mode | - | Window resolution |
        | Display Frequency | - | "displayFrequency" in  @ref _createRenderWindow |
        | Content Scaling Factor | 1.0 | "contentScalingFactor" in  @ref _createRenderWindow |
        @param
        name The name of the option to alter.
        @param
        value The value to set the option to.
        */
        virtual void setConfigOption(const String &name, const String &value) = 0;

        /** get a RenderWindowDescription from the current ConfigOptionMap
         */
        RenderWindowDescription getRenderWindowDescription() const;

        /** Create an object for performing hardware occlusion queries. 
        */
        virtual HardwareOcclusionQuery* createHardwareOcclusionQuery(void) = 0;

        /** Destroy a hardware occlusion query object. 
        */
        virtual void destroyHardwareOcclusionQuery(HardwareOcclusionQuery *hq);

        /** Validates the options set for the rendering system, returning a message if there are problems.
        @note
        If the returned string is empty, there are no problems.
        */
        virtual String validateConfigOptions(void) { return BLANKSTRING; }

        /** Start up the renderer using the settings selected (Or the defaults if none have been selected).

        Called by Root::setRenderSystem. Shouldn't really be called
        directly, although  this can be done if the app wants to.
        */
        virtual void _initialise();

        /** Query the real capabilities of the GPU and driver in the RenderSystem*/
        virtual RenderSystemCapabilities* createRenderSystemCapabilities() const = 0;
 
        /** Get a pointer to the current capabilities being used by the RenderSystem.
        @remarks
        The capabilities may be modified using this pointer, this will only have an effect
        before the RenderSystem has been initialised. It's intended use is to allow a
        listener of the RenderSystemCapabilitiesCreated event to customise the capabilities
        on the fly before the RenderSystem is initialised.
        */
        RenderSystemCapabilities* getMutableCapabilities(){ return mCurrentCapabilities; }

        /** Force the render system to use the special capabilities. Can only be called
        *    before the render system has been fully initializer (before createWindow is called) 
        *   @param
        *        capabilities has to be a subset of the real capabilities and the caller is 
        *        responsible for deallocating capabilities.
        */
        void useCustomRenderSystemCapabilities(RenderSystemCapabilities* capabilities);

        /** Restart the renderer (normally following a change in settings).
        */
        void reinitialise(void);

        /** Shutdown the renderer and cleanup resources.
        */
        virtual void shutdown(void);

        virtual const GpuProgramParametersPtr& getFixedFunctionParams(TrackVertexColourType tracking,
                                                                      FogMode fog)
        {
            return mFixedFunctionParams;
        }

        /// @deprecated migrate to getFixedFunctionParams ASAP. this is very slow now.
        OGRE_DEPRECATED void _setProjectionMatrix(Matrix4 m);

        /// @deprecated migrate to getFixedFunctionParams ASAP. this is very slow now.
        OGRE_DEPRECATED void _setViewMatrix(const Matrix4& m)
        {
            if (!mFixedFunctionParams) return;
            mFixedFunctionParams->setConstant(4, m);
            applyFixedFunctionParams(mFixedFunctionParams, GPV_GLOBAL);
        }

        /// @deprecated migrate to getFixedFunctionParams ASAP. this is very slow now.
        OGRE_DEPRECATED void _setWorldMatrix(const Matrix4& m)
        {
            if (!mFixedFunctionParams) return;
            mFixedFunctionParams->setConstant(0, m);
            applyFixedFunctionParams(mFixedFunctionParams, GPV_PER_OBJECT);
        }

        /// @deprecated migrate to getFixedFunctionParams ASAP. this is very slow now.
        OGRE_DEPRECATED void _setFog(FogMode f)
        {
            if (mFixedFunctionParams)
                getFixedFunctionParams(TVC_NONE, f);
        }

        /// @deprecated use setColourBlendState
        OGRE_DEPRECATED void _setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor,
                                               SceneBlendOperation op = SBO_ADD)
        {
            mCurrentBlend.sourceFactor = sourceFactor;
            mCurrentBlend.destFactor = destFactor;
            mCurrentBlend.sourceFactorAlpha = sourceFactor;
            mCurrentBlend.destFactorAlpha = destFactor;
            mCurrentBlend.operation = op;
            mCurrentBlend.alphaOperation = op;
            setColourBlendState(mCurrentBlend);
        }

        virtual void applyFixedFunctionParams(const GpuProgramParametersPtr& params, uint16 variabilityMask) {}

        /** Sets the type of light shading required (default = Gouraud).
        @deprecated only needed for fixed function APIs
        */
        virtual void setShadingType(ShadeOptions so) {}

        /** Sets whether or not dynamic lighting is enabled.
        @param
        enabled If true, dynamic lighting is performed on geometry with normals supplied, geometry without
        normals will not be displayed. If false, no lighting is applied and all geometry will be full brightness.
        @deprecated only needed for fixed function APIs
        */
        virtual void setLightingEnabled(bool enabled) {}

        /** Creates a new rendering window.
        @remarks
        This method creates a new rendering window as specified
        by the paramteters. The rendering system could be
        responible for only a single window (e.g. in the case
        of a game), or could be in charge of multiple ones (in the
        case of a level editor). The option to create the window
        as a child of another is therefore given.
        This method will create an appropriate subclass of
        RenderWindow depending on the API and platform implementation.
        @par
        After creation, this window can be retrieved using getRenderTarget().
        @param
        name The name of the window. Used in other methods
        later like setRenderTarget and getRenderTarget.
        @param
        width The width of the new window.
        @param
        height The height of the new window.
        @param
        fullScreen Specify true to make the window full screen
        without borders, title bar or menu bar.
        @param
        miscParams A NameValuePairList describing the other parameters for the new rendering window. 
        Options are case sensitive. Unrecognised parameters will be ignored silently.
        These values might be platform dependent, but these are present for all platforms unless
        indicated otherwise:

        | Key | Type / Values | Default | Description | Platform |
        |-----|---------------|---------|-------------|-------|
        | title | String | RenderTarget name | The title of the window that will appear in the title bar |  |
        | left | Positive integers | Centred | Screen x coordinate from left |  |
        | top | Positive integers | Centred | Screen y coordinate from left |  |
        | hidden | true, false | false | hide the created window | |
        | FSAA | Positive integer (usually 0, 2, 4, 8, 16) | 0 | Full screen antialiasing factor |  |
        | gamma | true, false | false | Enable hardware conversion from linear colour space to gamma colour space on rendering to the window. |  |
        | vsync | true, false | false | Synchronize buffer swaps to monitor vsync, eliminating tearing at the expense of a fixed frame rate |  |
        | vsyncInterval | 1, 2, 3, 4 | 1 | If vsync is enabled, the minimum number of vertical blanks that should occur between renders. For example if vsync is enabled, the refresh rate is 60 and this is set to 2, then the frame rate will be locked at 30. |  |
        | Full Screen | true, false | false | Specify whether to create the window in full screen mode | |
        | border | none, fixed, resize | resize | The type of window border (in windowed mode) | Windows, OSX |
        | displayFrequency | Refresh rate in Hertz (e.g. 60, 75, 100) | Desktop vsync rate | Display frequency rate, for fullscreen mode |  |
        | externalWindowHandle | <ul><li>Win32: HWND as int<li>Linux: X11 Window as ulong<li>OSX: OgreGLView address as an integer. You can pass NSView or NSWindow too, but should perform OgreGLView callbacks into the Ogre manually<li>iOS: UIWindow address as an integer<li>Emscripten: canvas selector String ("#canvas")</ul> | 0 (none) | External window handle, for embedding the OGRE render in an existing window |  |
        | externalGLControl | true, false | false | Let the external window control OpenGL i.e. don't select a pixel format for the window, do not change v-sync and do not swap buffer. When set to true, the calling application is responsible of OpenGL initialization and buffer swapping. It should also create an OpenGL context for its own rendering, Ogre will create one for its use. Then the calling application must also enable Ogre OpenGL context before calling any Ogre function and restore its OpenGL context after these calls. | OpenGL |
        | currentGLContext | true, false | false | Use an externally created GL context. (Must be current) | OpenGL |
        | minColourBufferSize | Positive integer (usually 16, 32) | 16 | Min total colour buffer size. See EGL_BUFFER_SIZE | OpenGL |
        | windowProc | WNDPROC | DefWindowProc | function that processes window messages | Win 32 |
        | colourDepth | 16, 32 | Desktop depth | Colour depth of the resulting rendering window; only applies if fullScreen | Win32 |
        | FSAAHint | %RenderSystem specific. Currently enables EQAA/ CSAA mode on D3D: if you want 4f8x (8x CSAA), set FSAA=4 and this to "f8" | Blank | FSAA mode hint | D3D |
        | outerDimensions | true, false | false | Whether the width/height is expressed as the size of the outer window, rather than the content area | Win32  |
        | monitorIndex | | -1 | | Win 32 |
        | monitorHandle | | 0 (none) | | Win 32 (OpenGL) |
        | enableDoubleClick | true, false | false | Enable the window to keep track and transmit double click messages. | Win32 |
        | useNVPerfHUD | true, false | false | Enable the use of nVidia NVPerfHUD | D3D |
        | depthBuffer | true, false | true | Use depth buffer | D3D |
        | NSOpenGLCPSurfaceOrder | -1 or 1 | 1 | [NSOpenGLCPSurfaceOrder](https://developer.apple.com/documentation/appkit/nsopenglcpsurfaceorder) | OSX |
        | contentScalingFactor | Positive Float | The default content scaling factor of the screen | On IOS specifies the CAEAGLLayer content scaling factor. This can be useful to limit the resolution of the OpenGL ES backing store. For example, the iPhone 4's native resolution is 960 x 640\. Windows are always 320 x 480, if you would like to limit the display to 720 x 480, specify 1.5 as the scaling factor. | OSX, iOS, Android |
        | externalViewHandle | UIView pointer as an integer | 0 | External view handle, for rendering OGRE render in an existing view | iOS |
        | externalViewControllerHandle | UIViewController pointer as an integer | 0 | External view controller handle, for embedding OGRE in an existing view controller | iOS |
        | externalSharegroup | EAGLSharegroup pointer as an integer | 0 | External sharegroup, used to shared GL resources between contexts | iOS |
        | CSAA | Positive integer (usually 0, 2, 4, 8, 16) | 0 | [Coverage sampling factor](https://www.khronos.org/registry/egl/extensions/NV/EGL_NV_coverage_sample.txt) | Android |
        | maxColourBufferSize | Positive integer (usually 16, 32) | 32 | Max EGL_BUFFER_SIZE | Android |
        | maxStencilBufferSize | Positive integer (usually 0, 8) | 0 | EGL_STENCIL_SIZE | Android |
        | maxDepthBufferSize | Positive integer (usually 0, 16, 24) | 16 | EGL_DEPTH_SIZE | Android |
        */
        virtual RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
            bool fullScreen, const NameValuePairList *miscParams = 0);
        
        /** Create a MultiRenderTarget, which is a render target that renders to multiple RenderTextures
        at once. Surfaces can be bound and unbound at will.
        This fails if mCapabilities->getNumMultiRenderTargets() is smaller than 2.
        */
        virtual MultiRenderTarget * createMultiRenderTarget(const String & name) = 0; 

        /** Destroys a render window */
        virtual void destroyRenderWindow(const String& name);
        /** Destroys a render texture */
        virtual void destroyRenderTexture(const String& name);
        /** Destroys a render target of any sort */
        virtual void destroyRenderTarget(const String& name);

        /** Attaches the passed render target to the render system.
        */
        void attachRenderTarget( RenderTarget &target );
        /** Returns a pointer to the render target with the passed name, or NULL if that
        render target cannot be found.
        */
        RenderTarget * getRenderTarget( const String &name );
        /** Detaches the render target with the passed name from the render system and
        returns a pointer to it.
        @note
        If the render target cannot be found, NULL is returned.
        */
        virtual RenderTarget * detachRenderTarget( const String &name );

        /** Returns the global instance vertex buffer.
        */
        HardwareVertexBufferSharedPtr getGlobalInstanceVertexBuffer() const;
        /** Sets the global instance vertex buffer.
        */
        void setGlobalInstanceVertexBuffer(const HardwareVertexBufferSharedPtr &val);
        /** Gets vertex declaration for the global vertex buffer for the global instancing
        */
        VertexDeclaration* getGlobalInstanceVertexBufferVertexDeclaration() const;
        /** Sets vertex declaration for the global vertex buffer for the global instancing
        */
        void setGlobalInstanceVertexBufferVertexDeclaration( VertexDeclaration* val);
        /** Gets the global number of instances.
        */
        size_t getGlobalNumberOfInstances() const;
        /** Sets the global number of instances.
        */
        void setGlobalNumberOfInstances(const size_t val);

        /** Retrieves an existing DepthBuffer or creates a new one suited for the given RenderTarget
            and sets it.
            @remarks
                RenderTarget's pool ID is respected. @see RenderTarget::setDepthBufferPool()
        */
        void setDepthBufferFor( RenderTarget *renderTarget );

        /**
         Returns if reverse Z-buffer is enabled.

         If you have large scenes and need big far clip distance but still want
         to draw objects closer (for example cockpit of a plane) you can enable
         reverse depth buffer so that the depth buffer precision is greater further away.
         This enables the OGRE_REVERSED_Z preprocessor define for shaders.

         @retval true If reverse Z-buffer is enabled.
         @retval false If reverse Z-buffer is disabled (default).

         @see setReverseDepthBuffer
         */
        bool isReverseDepthBufferEnabled() const;

        // ------------------------------------------------------------------------
        //                     Internal Rendering Access
        // All methods below here are normally only called by other OGRE classes
        // They can be called by library user if required
        // ------------------------------------------------------------------------

        /** Tells the rendersystem to use the attached set of lights (and no others) 
        up to the number specified (this allows the same list to be used with different
        count limits)
        @deprecated only needed for fixed function APIs
        */
        virtual void _useLights(unsigned short limit) {}
        /** Utility function for setting all the properties of a texture unit at once.
        This method is also worth using over the individual texture unit settings because it
        only sets those settings which are different from the current settings for this
        unit, thus minimising render state changes.
        */
        virtual void _setTextureUnitSettings(size_t texUnit, TextureUnitState& tl);
        /// set the sampler settings for the given texture unit
        virtual void _setSampler(size_t texUnit, Sampler& s) = 0;
        /** Turns off a texture unit. */
        virtual void _disableTextureUnit(size_t texUnit);
        /** Disables all texture units from the given unit upwards */
        virtual void _disableTextureUnitsFrom(size_t texUnit);

        /** Sets whether or not rendering points using OT_POINT_LIST will 
        render point sprites (textured quads) or plain points.
        @param enabled True enables point sprites, false returns to normal
        point rendering.
        @deprecated only needed for fixed function APIs
        */  
        virtual void _setPointSpritesEnabled(bool enabled) {};

        /**
        @deprecated only needed for fixed function APIs
        */
        virtual void _setPointParameters(bool attenuationEnabled, Real minSize, Real maxSize) {}

        /**
         * Set the line width when drawing as RenderOperation::OT_LINE_LIST/ RenderOperation::OT_LINE_STRIP
         * @param width only value of 1.0 might be supported. Check for RSC_WIDE_LINES.
         */
        virtual void _setLineWidth(float width) {};

        /**
        Sets the texture to bind to a given texture unit.

        User processes would not normally call this direct unless rendering
        primitives themselves.

        @param unit The index of the texture unit to modify. Multitexturing 
        hardware can support multiple units (see 
        RenderSystemCapabilites::getNumTextureUnits)
        @param enabled Boolean to turn the unit on/off
        @param texPtr Pointer to the texture to use.
        */
        virtual void _setTexture(size_t unit, bool enabled, 
            const TexturePtr &texPtr) = 0;

        /// @deprecated obsolete
        OGRE_DEPRECATED virtual void _setVertexTexture(size_t unit, const TexturePtr& tex);

        /**
        Sets the texture coordinate set to use for a texture unit.

        Meant for use internally - not generally used directly by apps - the Material and TextureUnitState
        classes let you manage textures far more easily.

        @param unit Texture unit as above
        @param index The index of the texture coordinate set to use.
        @deprecated only needed for fixed function APIs
        */
        virtual void _setTextureCoordSet(size_t unit, size_t index) {}

        /**
        Sets a method for automatically calculating texture coordinates for a stage.
        Should not be used by apps - for use by Ogre only.
        @param unit Texture unit as above
        @param m Calculation method to use
        @param frustum Optional Frustum param, only used for projective effects
        @deprecated only needed for fixed function APIs
        */
        virtual void _setTextureCoordCalculation(size_t unit, TexCoordCalcMethod m, 
            const Frustum* frustum = 0) {}

        /** Sets the texture blend modes from a TextureUnitState record.
        Meant for use internally only - apps should use the Material
        and TextureUnitState classes.
        @param unit Texture unit as above
        @param bm Details of the blending mode
        @deprecated only needed for fixed function APIs
        */
        virtual void _setTextureBlendMode(size_t unit, const LayerBlendModeEx& bm) {}

        /// @deprecated use _setSampler
        OGRE_DEPRECATED virtual void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter) {}

        /// @deprecated use _setSampler
        OGRE_DEPRECATED virtual void _setTextureUnitFiltering(size_t unit, FilterOptions minFilter,
            FilterOptions magFilter, FilterOptions mipFilter);

        /// @deprecated use _setSampler
        OGRE_DEPRECATED virtual void _setTextureAddressingMode(size_t unit, const Sampler::UVWAddressingMode& uvw) {}

        /** Sets the texture coordinate transformation matrix for a texture unit.
        @param unit Texture unit to affect
        @param xform The 4x4 matrix
        @deprecated only needed for fixed function APIs
        */
        virtual void _setTextureMatrix(size_t unit, const Matrix4& xform) {}

        /// Sets the global blending factors for combining subsequent renders with the existing frame contents.
        virtual void setColourBlendState(const ColourBlendState& state) = 0;

        /// @deprecated use setColourBlendState
        OGRE_DEPRECATED void
        _setSeparateSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor,
                                  SceneBlendFactor sourceFactorAlpha, SceneBlendFactor destFactorAlpha,
                                  SceneBlendOperation op = SBO_ADD, SceneBlendOperation alphaOp = SBO_ADD)
        {
            mCurrentBlend.sourceFactor = sourceFactor;
            mCurrentBlend.destFactor = destFactor;
            mCurrentBlend.sourceFactorAlpha = sourceFactorAlpha;
            mCurrentBlend.destFactorAlpha = destFactorAlpha;
            mCurrentBlend.operation = op;
            mCurrentBlend.alphaOperation = alphaOp;
            setColourBlendState(mCurrentBlend);
        }

        /** Sets the global alpha rejection approach for future renders.
        By default images are rendered regardless of texture alpha. This method lets you change that.
        @param func The comparison function which must pass for a pixel to be written.
        @param value The value to compare each pixels alpha value to (0-255)
        @param alphaToCoverage Whether to enable alpha to coverage, if supported
        */
        virtual void _setAlphaRejectSettings(CompareFunction func, unsigned char value, bool alphaToCoverage) = 0;

        /** Notify the rendersystem that it should adjust texture projection to be 
            relative to a different origin.
        */
        virtual void _setTextureProjectionRelativeTo(bool enabled, const Vector3& pos);

        /** Creates a DepthBuffer that can be attached to the specified RenderTarget
            @remarks
                It doesn't attach anything, it just returns a pointer to a new DepthBuffer
                Caller is responsible for putting this buffer into the right pool, for
                attaching, and deleting it. Here's where API-specific magic happens.
                Don't call this directly unless you know what you're doing.
        */
        virtual DepthBuffer* _createDepthBufferFor( RenderTarget *renderTarget ) = 0;

        /** Removes all depth buffers. Should be called on device lost and shutdown
            @remarks
                Advanced users can call this directly with bCleanManualBuffers=false to
                remove all depth buffers created for RTTs; when they think the pool has
                grown too big or they've used lots of depth buffers they don't need anymore,
                freeing GPU RAM.
        */
        void _cleanupDepthBuffers( bool bCleanManualBuffers=true );

        /**
        * Signifies the beginning of a frame, i.e. the start of rendering on a single viewport. Will occur
        * several times per complete frame if multiple viewports exist.
        */
        virtual void _beginFrame();
        
        /// Dummy structure for render system contexts - implementing RenderSystems can extend
        /// as needed
        struct RenderSystemContext { };
        /**
        * Pause rendering for a frame. This has to be called after _beginFrame and before _endFrame.
        * Will usually be called by the SceneManager, don't use this manually unless you know what
        * you are doing.
        */
        virtual RenderSystemContext* _pauseFrame(void);
        /**
        * Resume rendering for a frame. This has to be called after a _pauseFrame call
        * Will usually be called by the SceneManager, don't use this manually unless you know what
        * you are doing.
        * @param context the render system context, as returned by _pauseFrame
        */
        virtual void _resumeFrame(RenderSystemContext* context);

        /**
        * Ends rendering of a frame to the current viewport.
        */
        virtual void _endFrame(void) = 0;
        /**
        Sets the provided viewport as the active one for future
        rendering operations. This viewport is aware of it's own
        camera and render target. Must be implemented by subclass.

        @param vp Pointer to the appropriate viewport.
        */
        virtual void _setViewport(Viewport *vp) = 0;
        /** Get the current active viewport for rendering. */
        virtual Viewport* _getViewport(void);

        /** Sets the culling mode for the render system based on the 'vertex winding'.
        @copydetails Pass::setCullingMode
        */
        virtual void _setCullingMode(CullingMode mode) = 0;

        virtual CullingMode _getCullingMode(void) const;

        /** Sets the mode of operation for depth buffer tests from this point onwards.
        Sometimes you may wish to alter the behaviour of the depth buffer to achieve
        special effects. Because it's unlikely that you'll set these options for an entire frame,
        but rather use them to tweak settings between rendering objects, this is an internal
        method (indicated by the '_' prefix) which will be used by a SceneManager implementation
        rather than directly from the client application.
        If this method is never called the settings are automatically the same as the default parameters.
        @param depthTest If true, the depth buffer is tested for each pixel and the frame buffer is only updated
        if the depth function test succeeds. If false, no test is performed and pixels are always written.
        @param depthWrite If true, the depth buffer is updated with the depth of the new pixel if the depth test succeeds.
        If false, the depth buffer is left unchanged even if a new pixel is written.
        @param depthFunction Sets the function required for the depth test.
        */
        virtual void _setDepthBufferParams(bool depthTest = true, bool depthWrite = true, CompareFunction depthFunction = CMPF_LESS_EQUAL) = 0;

        /// @deprecated use _setDepthBufferParams
        OGRE_DEPRECATED virtual void _setDepthBufferCheckEnabled(bool enabled = true) {}
        /// @deprecated use _setDepthBufferParams
        OGRE_DEPRECATED virtual void _setDepthBufferWriteEnabled(bool enabled = true) {}
        /// @deprecated use _setDepthBufferParams
        OGRE_DEPRECATED virtual void _setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL) {}
        /// @deprecated use setColourBlendState
        OGRE_DEPRECATED void _setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha)
        {
            mCurrentBlend.writeR = red;
            mCurrentBlend.writeG = green;
            mCurrentBlend.writeB = blue;
            mCurrentBlend.writeA = alpha;
            setColourBlendState(mCurrentBlend);
        }
        /** Sets the depth bias, NB you should use the Material version of this. 
        @remarks
        When polygons are coplanar, you can get problems with 'depth fighting' where
        the pixels from the two polys compete for the same screen pixel. This is particularly
        a problem for decals (polys attached to another surface to represent details such as
        bulletholes etc.).
        @par
        A way to combat this problem is to use a depth bias to adjust the depth buffer value
        used for the decal such that it is slightly higher than the true value, ensuring that
        the decal appears on top.
        @note
        The final bias value is a combination of a constant bias and a bias proportional
        to the maximum depth slope of the polygon being rendered. The final bias
        is constantBias + slopeScaleBias * maxslope. Slope scale biasing is
        generally preferable but is not available on older hardware.
        @param constantBias The constant bias value, expressed as a value in 
        homogeneous depth coordinates.
        @param slopeScaleBias The bias value which is factored by the maximum slope
        of the polygon, see the description above. This is not supported by all
        cards.

        */
        virtual void _setDepthBias(float constantBias, float slopeScaleBias = 0.0f) = 0;

        /**
         * Clamp depth values to near and far plane rather than discarding
         *
         * Useful for "shadow caster pancaking" or with shadow volumes
         */
        virtual void _setDepthClamp(bool enable) {}

        /** The RenderSystem will keep a count of tris rendered, this resets the count. */
        virtual void _beginGeometryCount(void);
        /** Reports the number of tris rendered since the last _beginGeometryCount call. */
        virtual unsigned int _getFaceCount(void) const;
        /** Reports the number of batches rendered since the last _beginGeometryCount call. */
        virtual unsigned int _getBatchCount(void) const;
        /** Reports the number of vertices passed to the renderer since the last _beginGeometryCount call. */
        virtual unsigned int _getVertexCount(void) const;

        /// @deprecated use ColourValue::getAsBYTE()
        OGRE_DEPRECATED static void convertColourValue(const ColourValue& colour, uint32* pDest)
        {
            *pDest = colour.getAsBYTE();
        }
        /// @deprecated assume VET_UBYTE4_NORM
        OGRE_DEPRECATED static VertexElementType getColourVertexElementType(void) { return VET_UBYTE4_NORM; }

        /** Converts a uniform projection matrix to suitable for this render system.
        @remarks
        Because different APIs have different requirements (some incompatible) for the
        projection matrix, this method allows each to implement their own correctly and pass
        back a generic OGRE matrix for storage in the engine.
        */
        virtual void _convertProjectionMatrix(const Matrix4& matrix,
            Matrix4& dest, bool forGpuProgram = false) = 0;

        /** Sets how to rasterise triangles, as points, wireframe or solid polys. */
        virtual void _setPolygonMode(PolygonMode level) = 0;

        /** This method allows you to set all the stencil buffer parameters in one call.

        Unlike other render states, stencilling is left for the application to turn
        on and off when it requires. This is because you are likely to want to change
        parameters between batches of arbitrary objects and control the ordering yourself.
        In order to batch things this way, you'll want to use OGRE's Compositor stencil pass
        or separate render queue groups and register a RenderQueueListener to get notifications
        between batches.

        @see RenderQueue
        */
        virtual void setStencilState(const StencilState& state) = 0;

        /// @deprecated use setStencilState
        OGRE_DEPRECATED void setStencilCheckEnabled(bool enabled);
        /// @deprecated use setStencilState
        OGRE_DEPRECATED void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, uint32 refValue = 0,
                                    uint32 compareMask = 0xFFFFFFFF, uint32 writeMask = 0xFFFFFFFF,
                                    StencilOperation stencilFailOp = SOP_KEEP,
                                    StencilOperation depthFailOp = SOP_KEEP,
                                    StencilOperation passOp = SOP_KEEP, bool twoSidedOperation = false);

        /** Sets whether or not normals are to be automatically normalised.
        @remarks
        This is useful when, for example, you are scaling SceneNodes such that
        normals may not be unit-length anymore. Note though that this has an
        overhead so should not be turn on unless you really need it.
        @par
        You should not normally call this direct unless you are rendering
        world geometry; set it on the Renderable because otherwise it will be
        overridden by material settings.
        @deprecated only needed for fixed function APIs
        */
        virtual void setNormaliseNormals(bool normalise) {}

        /**
        Render something to the active viewport.

        Low-level rendering interface to perform rendering
        operations. Unlikely to be used directly by client
        applications, since the SceneManager and various support
        classes will be responsible for calling this method.
        Can only be called between _beginScene and _endScene

        @param op A rendering operation instance, which contains
        details of the operation to be performed.
        */
        virtual void _render(const RenderOperation& op);

        virtual void _dispatchCompute(const Vector3i& workgroupDim) {}

        /** Gets the capabilities of the render system. */
        const RenderSystemCapabilities* getCapabilities(void) const { return mCurrentCapabilities; }


        /** Returns the driver version.
        */
        const DriverVersion& getDriverVersion(void) const { return mDriverVersion; }

        /** Returns the default material scheme used by the render system.
            Systems that use the RTSS to emulate a fixed function pipeline 
            (e.g. OpenGL ES 2, GL3+, DX11) need to return
            the default material scheme of the RTSS ShaderGenerator.
         
            This is currently only used to set the default material scheme for
            viewports.  It is a necessary step on these render systems for
            render textures to be rendered into properly.
        */
        const String& _getDefaultViewportMaterialScheme(void) const;

        /** Binds a given GpuProgram (but not the parameters). 
        @remarks Only one GpuProgram of each type can be bound at once, binding another
        one will simply replace the existing one.
        */
        virtual void bindGpuProgram(GpuProgram* prg);

        /** Bind Gpu program parameters.
        @param gptype The type of program to bind the parameters to
        @param params The parameters to bind
        @param variabilityMask A mask of GpuParamVariability identifying which params need binding
        */
        virtual void bindGpuProgramParameters(GpuProgramType gptype, 
            const GpuProgramParametersPtr& params, uint16 variabilityMask) = 0;

        /** Unbinds GpuPrograms of a given GpuProgramType.
        @remarks
        This returns the pipeline to fixed-function processing for this type.
        */
        virtual void unbindGpuProgram(GpuProgramType gptype);

        /** Returns whether or not a Gpu program of the given type is currently bound. */
        bool isGpuProgramBound(GpuProgramType gptype);

        /**
         * Gets the native shading language version for this render system.
         * Formatted so that it can be used within a shading program. 
         * For example, OpenGL 3.2 would return 150, while 3.3 would return 330
         */
        uint16 getNativeShadingLanguageVersion() const { return mNativeShadingLanguageVersion; }

        /** Sets the user clipping region.
        @deprecated only needed for fixed function APIs
        */
        virtual void setClipPlanes(const PlaneList& clipPlanes);

        /** Utility method for initialising all render targets attached to this rendering system. */
        void _initRenderTargets(void);

        /** Utility method to notify all render targets that a camera has been removed, 
        in case they were referring to it as their viewer. 
        */
        void _notifyCameraRemoved(const Camera* cam);

        /** Internal method for updating all render targets attached to this rendering system. */
        virtual void _updateAllRenderTargets(bool swapBuffers = true);
        /** Internal method for swapping all the buffers on all render targets,
        if _updateAllRenderTargets was called with a 'false' parameter. */
        virtual void _swapAllRenderTargetBuffers();

        /** Sets whether or not vertex windings set should be inverted; this can be important
        for rendering reflections. */
        void setInvertVertexWinding(bool invert);

        /** Indicates whether or not the vertex windings set will be inverted for the current render (e.g. reflections)
        @see RenderSystem::setInvertVertexWinding
        */
        bool getInvertVertexWinding(void) const;

        /** Sets the 'scissor region' i.e. the region of the target in which rendering can take place.
        @remarks
        This method allows you to 'mask off' rendering in all but a given rectangular area
        as identified by the parameters to this method.
        @param enabled True to enable the scissor test, false to disable it.
        @param rect The location of the corners of the rectangle, expressed in
        <i>pixels</i>.
        */
        virtual void setScissorTest(bool enabled, const Rect& rect = Rect()) = 0;
        /// @deprecated
        OGRE_DEPRECATED void setScissorTest(bool enabled, uint32 left, uint32 top = 0,
                                            uint32 right = 800, uint32 bottom = 600)
        {
            setScissorTest(enabled, Rect(left, top, right, bottom));
        }

        /** Clears one or more frame buffers on the active render target. 
        @param buffers Combination of one or more elements of FrameBufferType
        denoting which buffers are to be cleared
        @param colour The colour to clear the colour buffer with, if enabled
        @param depth The value to initialise the depth buffer with, if enabled
        @param stencil The value to initialise the stencil buffer with, if enabled.
        */
        virtual void clearFrameBuffer(uint32 buffers, const ColourValue& colour = ColourValue::Black,
                                      float depth = 1.0f, uint16 stencil = 0) = 0;
        /** Returns the horizontal texel offset value required for mapping 
        texel origins to pixel origins in this rendersystem.
        @remarks
        Since rendersystems sometimes disagree on the origin of a texel, 
        mapping from texels to pixels can sometimes be problematic to 
        implement generically. This method allows you to retrieve the offset
        required to map the origin of a texel to the origin of a pixel in
        the horizontal direction.
        @note only non-zero with D3D9
        */
        virtual Real getHorizontalTexelOffset(void) { return 0.0f; }
        /** Returns the vertical texel offset value required for mapping 
        texel origins to pixel origins in this rendersystem.
        @remarks
        Since rendersystems sometimes disagree on the origin of a texel, 
        mapping from texels to pixels can sometimes be problematic to 
        implement generically. This method allows you to retrieve the offset
        required to map the origin of a texel to the origin of a pixel in
        the vertical direction.
        @note only non-zero with D3D9
        */
        virtual Real getVerticalTexelOffset(void) { return 0.0f; }

        /** Gets the minimum (closest) depth value to be used when rendering
        using identity transforms.
        @remarks
        When using identity transforms you can manually set the depth
        of a vertex; however the input values required differ per
        rendersystem. This method lets you retrieve the correct value.
        @see Renderable::getUseIdentityView, Renderable::getUseIdentityProjection
        */
        virtual Real getMinimumDepthInputValue(void) = 0;
        /** Gets the maximum (farthest) depth value to be used when rendering
        using identity transforms.
        @remarks
        When using identity transforms you can manually set the depth
        of a vertex; however the input values required differ per
        rendersystem. This method lets you retrieve the correct value.
        @see Renderable::getUseIdentityView, Renderable::getUseIdentityProjection
        */
        virtual Real getMaximumDepthInputValue(void) = 0;
        /** set the current multi pass count value.  This must be set prior to 
        calling _render() if multiple renderings of the same pass state are 
        required.
        @param count Number of times to render the current state.
        */
        virtual void setCurrentPassIterationCount(const size_t count) { mCurrentPassIterationCount = count; }

        /** Tell the render system whether to derive a depth bias on its own based on 
        the values passed to it in setCurrentPassIterationCount.
        The depth bias set will be baseValue + iteration * multiplier
        @param derive True to tell the RS to derive this automatically
        @param baseValue The base value to which the multiplier should be
        added
        @param multiplier The amount of depth bias to apply per iteration
        @param slopeScale The constant slope scale bias for completeness
        */
        void setDeriveDepthBias(bool derive, float baseValue = 0.0f,
            float multiplier = 0.0f, float slopeScale = 0.0f)
        {
            mDerivedDepthBias = derive;
            mDerivedDepthBiasBase = baseValue;
            mDerivedDepthBiasMultiplier = multiplier;
            mDerivedDepthBiasSlopeScale = slopeScale;
        }

        /**
         * Set current render target to target, enabling its device context if needed
         */
        virtual void _setRenderTarget(RenderTarget *target) = 0;

        /** Defines a listener on the custom events that this render system 
        can raise.
        @see RenderSystem::addListener
        */
        class _OgreExport Listener
        {
        public:
            Listener() {}
            virtual ~Listener() {}

            /** A rendersystem-specific event occurred.
            @param eventName The name of the event which has occurred
            @param parameters A list of parameters that may belong to this event,
            may be null if there are no parameters
            */
            virtual void eventOccurred(const String& eventName, 
                const NameValuePairList* parameters = 0) = 0;
        };

        /** Sets shared listener.
        @remarks
        Shared listener could be set even if no render system is selected yet.
        This listener will receive "RenderSystemChanged" event on each Root::setRenderSystem call.
        */
        static void setSharedListener(Listener* listener);
        /** Retrieve a pointer to the current shared render system listener. */
        static Listener* getSharedListener(void);

        /** Adds a listener to the custom events that this render system can raise.
        @remarks
        Some render systems have quite specific, internally generated events 
        that the application may wish to be notified of. Many applications
        don't have to worry about these events, and can just trust OGRE to 
        handle them, but if you want to know, you can add a listener here.
        @par
        Events are raised very generically by string name. Perhaps the most 
        common example of a render system specific event is the loss and 
        restoration of a device in DirectX; which OGRE deals with, but you 
        may wish to know when it happens. 
        @see RenderSystem::getRenderSystemEvents
        */
        void addListener(Listener* l);
        /** Remove a listener to the custom events that this render system can raise.
        */
        void removeListener(Listener* l);

        /** Gets a list of the rendersystem specific events that this rendersystem
        can raise.
        @see RenderSystem::addListener
        */
        const StringVector& getRenderSystemEvents(void) const { return mEventNames; }

        /** Tell the rendersystem to perform any prep tasks it needs to directly
        before other threads which might access the rendering API are registered.
        @remarks
        Call this from your main thread before starting your other threads.
        @note
        If you start your own threads, there is a specific startup sequence which
        must be respected and requires synchronisation between the threads:

        @note
        1. [Main thread] Call preExtraThreadsStarted()
        2. [Main thread] Start other thread, wait
        3. [Other thread] Call registerThread(), notify main thread & continue
        4. [Main thread] Wake up & call postExtraThreadsStarted()

        @note
        Once this init sequence is completed the threads are independent but
        this startup sequence must be respected.
        */
        virtual void preExtraThreadsStarted() {}

        /** Tell the rendersystem to perform any tasks it needs to directly
        after other threads which might access the rendering API are registered.
        @see RenderSystem::preExtraThreadsStarted
        */
        virtual void postExtraThreadsStarted() {}

        /** Register the an additional thread which may make calls to rendersystem-related 
        objects.
        @remarks
        This method should only be called by additional threads during their
        initialisation. If they intend to use hardware rendering system resources 
        they should call this method before doing anything related to the render system.
        Some rendering APIs require a per-thread setup and this method will sort that
        out. It is also necessary to call unregisterThread before the thread shuts down.
        @note
        This method takes no parameters - it must be called from the thread being
        registered and that context is enough.
        */
        virtual void registerThread() {}

        /** Unregister an additional thread which may make calls to rendersystem-related objects.
        @see RenderSystem::registerThread
        */
        virtual void unregisterThread() {}

        /// @deprecated do not use
        OGRE_DEPRECATED virtual unsigned int getDisplayMonitorCount() const { return 1; }

        /**
        * This marks the beginning of an event for GPU profiling.
        */
        virtual void beginProfileEvent( const String &eventName ) = 0;

        /**
        * Ends the currently active GPU profiling event.
        */
        virtual void endProfileEvent( void ) = 0;

        /**
        * Marks an instantaneous event for graphics profilers.  
        * This is equivalent to calling @see beginProfileEvent and @see endProfileEvent back to back.
        */
        virtual void markProfileEvent( const String &event ) = 0;

        /** Gets a custom (maybe platform-specific) attribute.
        @remarks This is a nasty way of satisfying any API's need to see platform-specific details.
        @param name The name of the attribute.
        @param pData Pointer to memory of the right kind of structure to receive the info.
        */
        virtual void getCustomAttribute(const String& name, void* pData);

		/**
		* Sets the colour buffer that the render system will to draw. If the render system
		* implementation or configuration does not support a particular value, then false will be
		* returned and the current colour buffer value will not be modified.
		*
		* @param
		*     colourBuffer Specifies the colour buffer that will be drawn into.
		*/
		virtual bool setDrawBuffer(ColourBufferType colourBuffer) { return false; };

    protected:

        /** DepthBuffers to be attached to render targets */
        DepthBufferMap  mDepthBufferPool;

        /** The render targets. */
        RenderTargetMap mRenderTargets;
        /** The render targets, ordered by priority. */
        RenderTargetPriorityMap mPrioritisedRenderTargets;
        /** The Active render target. */
        RenderTarget * mActiveRenderTarget;

        /** The Active GPU programs and gpu program parameters*/
        GpuProgramParametersSharedPtr mActiveVertexGpuProgramParameters;
        GpuProgramParametersSharedPtr mActiveGeometryGpuProgramParameters;
        GpuProgramParametersSharedPtr mActiveFragmentGpuProgramParameters;
        GpuProgramParametersSharedPtr mActiveTessellationHullGpuProgramParameters;
        GpuProgramParametersSharedPtr mActiveTessellationDomainGpuProgramParameters;
        GpuProgramParametersSharedPtr mActiveComputeGpuProgramParameters;

        // Texture manager
        // A concrete class of this will be created and
        // made available under the TextureManager singleton,
        // managed by the RenderSystem
        TextureManager* mTextureManager;

        // Active viewport (dest for future rendering operations)
        Viewport* mActiveViewport;

        CullingMode mCullingMode;

        size_t mBatchCount;
        size_t mFaceCount;
        size_t mVertexCount;

        /// Saved manual colour blends
        ColourValue mManualBlendColours[OGRE_MAX_TEXTURE_LAYERS][2];

        bool mInvertVertexWinding;
        bool mIsReverseDepthBufferEnabled;

        /// Texture units from this upwards are disabled
        size_t mDisabledTexUnitsFrom;

        /// number of times to render the current state
        size_t mCurrentPassIterationCount;
        size_t mCurrentPassIterationNum;
        /// Whether to update the depth bias per render call
        bool mDerivedDepthBias;
        float mDerivedDepthBiasBase;
        float mDerivedDepthBiasMultiplier;
        float mDerivedDepthBiasSlopeScale;

        /// a global vertex buffer for global instancing
        HardwareVertexBufferSharedPtr mGlobalInstanceVertexBuffer;
        /// a vertex declaration for the global vertex buffer for the global instancing
        VertexDeclaration* mGlobalInstanceVertexBufferVertexDeclaration;
        /// the number of global instances (this number will be multiply by the render op instance number) 
        size_t mGlobalNumberOfInstances;

        /** updates pass iteration rendering state including bound gpu program parameter
        pass iteration auto constant entry
        @return True if more iterations are required
        */
        bool updatePassIterationRenderState(void);

        /// List of names of events this rendersystem may raise
        StringVector mEventNames;

        /// Internal method for firing a rendersystem event
        void fireEvent(const String& name, const NameValuePairList* params = 0);

        typedef std::list<Listener*> ListenerList;
        ListenerList mEventListeners;
        static Listener* msSharedEventListener;

        typedef std::list<HardwareOcclusionQuery*> HardwareOcclusionQueryList;
        HardwareOcclusionQueryList mHwOcclusionQueries;

        bool mVertexProgramBound;
        bool mGeometryProgramBound;
        bool mFragmentProgramBound;
        bool mTessellationHullProgramBound;
        bool mTessellationDomainProgramBound;
        bool mComputeProgramBound;

        // Recording user clip planes
        PlaneList mClipPlanes;
        // Indicator that we need to re-set the clip planes on next render call
        bool mClipPlanesDirty;

        /// Used to store the capabilities of the graphics card
        RenderSystemCapabilities* mRealCapabilities;
        RenderSystemCapabilities* mCurrentCapabilities;
        bool mUseCustomCapabilities;

        /// @deprecated only needed for fixed function APIs
        virtual void setClipPlanesImpl(const PlaneList& clipPlanes) {}

        /** Initialize the render system from the capabilities*/
        virtual void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary) = 0;


        DriverVersion mDriverVersion;
        uint16 mNativeShadingLanguageVersion;

        bool mTexProjRelative;
        Vector3 mTexProjRelativeOrigin;

        // Stored options
        ConfigOptionMap mOptions;

        virtual void initConfigOptions();

        ColourBlendState mCurrentBlend;
        GpuProgramParametersSharedPtr mFixedFunctionParams;

        void initFixedFunctionParams();
        void setFFPLightParams(uint32 index, bool enabled);
        bool flipFrontFace() const;
        static CompareFunction reverseCompareFunction(CompareFunction func);
    private:
        StencilState mStencilState;
    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
