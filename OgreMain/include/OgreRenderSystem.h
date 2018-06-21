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
        grouped into a structure (_ConfigOption) which includes
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
        virtual ConfigOptionMap& getConfigOptions(void) = 0;

        /** Sets an option for this API
        @remarks
        Used to confirm the settings (normally chosen by the user) in
        order to make the renderer able to initialise with the settings as required.
        This may be video mode, D3D driver, full screen / windowed etc.
        Called automatically by the default configuration
        dialog, and by the restoration of saved settings.
        These settings are stored and only activated when
        RenderSystem::initialise or RenderSystem::reinitialise
        are called.
        @par
        If using a custom configuration dialog, it is advised that the
        caller calls RenderSystem::getConfigOptions
        again, since some options can alter resulting from a selection.
        @param
        name The name of the option to alter.
        @param
        value The value to set the option to.
        */
        virtual void setConfigOption(const String &name, const String &value) = 0;

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
        virtual String validateConfigOptions(void) = 0;

        /** Start up the renderer using the settings selected (Or the defaults if none have been selected).
        @remarks
        Called by Root::setRenderSystem. Shouldn't really be called
        directly, although  this can be done if the app wants to.
        @param
        autoCreateWindow If true, creates a render window
        automatically, based on settings chosen so far. This saves
        an extra call to _createRenderWindow
        for the main render window.
        @param
        windowTitle Sets the app window title
        @return
        A pointer to the automatically created window, if requested, otherwise null.
        */
        virtual RenderWindow* _initialise(bool autoCreateWindow, const String& windowTitle = "OGRE Render Window");

        /**
        Returns whether under the current render system buffers marked as TU_STATIC can be locked for update
        @remarks
        Needed in the implementation of DirectX9 with DirectX9Ex driver
        */
        virtual bool isStaticBufferLockable() const { return true; }

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
        virtual void reinitialise(void) = 0;

        /** Shutdown the renderer and cleanup resources.
        */
        virtual void shutdown(void);


        /** Sets the colour & strength of the ambient (global directionless) light in the world.
        @deprecated only needed for fixed function APIs
        */
        virtual void setAmbientLight(float r, float g, float b) {}

        /// @overload
        void setAmbientLight(const ColourValue& c) { setAmbientLight(c.r, c.g, c.b); }

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

        /** Sets whether or not W-buffers are enabled if they are available for this renderer.
        @param
        enabled If true and the renderer supports them W-buffers will be used.  If false 
        W-buffers will not be used even if available.  W-buffers are enabled by default 
        for 16bit depth buffers and disabled for all other depths.
        */
        void setWBufferEnabled(bool enabled);

        /** Returns true if the renderer will try to use W-buffers when available.
        */
        bool getWBufferEnabled(void) const;

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
        | Key | Type / Values | Default | Description | Notes |
        |-----|---------------|---------|-------------|-------|
        | title | Any string | RenderTarget name | The title of the window that will appear in the title bar |  |
        | left | Positive integers | Centred | Screen x coordinate from left |  |
        | top | Positive integers | Centred | Screen y coordinate from left |  |
        | border | none, fixed, resize | resize | The type of window border (in windowed mode) |  |
        | hidden | true, false | false | hide the created window | |
        | FSAA | Positive integer (usually 0, 2, 4, 8, 16) | 0 | Full screen antialiasing factor |  |
        | gamma | true, false | false | Enable hardware conversion from linear colour space to gamma colour space on rendering to the window. |  |
        | vsync | true, false | false | Synchronize buffer swaps to monitor vsync, eliminating tearing at the expense of a fixed frame rate |  |
        | vsyncInterval | 1, 2, 3, 4 | 1 | If vsync is enabled, the minimum number of vertical blanks that should occur between renders. For example if vsync is enabled, the refresh rate is 60 and this is set to 2, then the frame rate will be locked at 30. |  |
        | Full Screen | true, false | false | Specify whether to create the window in full screen mode | |
        | displayFrequency | Refresh rate in Hertz (e.g. 60, 75, 100) | Desktop vsync rate | Display frequency rate, for fullscreen mode |  |
        | parentWindowHandle | <ul><li>Win32: HWND as integer<li>GLX: poslong:posint:poslong (display*:screen:windowHandle) or poslong:posint:poslong:poslong (display*:screen:windowHandle:XVisualInfo*) | 0 (none) | Parent window handle, for embedding the OGRE in a child of an external window |  |
        | externalWindowHandle | <ul><li>Win32: HWND as integer<li>GLX: poslong:posint:poslong (display*:screen:windowHandle) or poslong:posint:poslong:poslong (display*:screen:windowHandle:XVisualInfo*)<li>OS X Cocoa: OgreGLView address as an integer. You can pass NSView or NSWindow too, but should perform OgreGLView callbacks into the Ogre manually.<li>OS X Carbon: WindowRef as an integer<li>iOS: UIWindow address as an integer</ul> | 0 (none) | External window handle, for embedding the OGRE render in an existing window |  |
        | externalGLControl | true, false | false | Let the external window control OpenGL i.e. don't select a pixel format for the window, do not change v-sync and do not swap buffer. When set to true, the calling application is responsible of OpenGL initialization and buffer swapping. It should also create an OpenGL context for its own rendering, Ogre will create one for its use. Then the calling application must also enable Ogre OpenGL context before calling any Ogre function and restore its OpenGL context after these calls. | OpenGL Specific |
        | currentGLContext | true, false | false | Use an externally created GL context. (Must be current) | OpenGL Specific |
        | minColourBufferSize | Positive integer (usually 16, 32) | 16 | Min total colour buffer size. See EGL_BUFFER_SIZE | OpenGL Specific |
        | colourDepth | 16, 32 | Desktop depth | Colour depth of the resulting rendering window; only applies if fullScreen | Win32 Specific |
        | FSAAHint | Depends on RenderSystem and hardware. Currently supports:"Quality": on systems that have an option to prefer higher AA quality over speed, use it | Blank | Full screen antialiasing hint | Win32 Specific |
        | outerDimensions | true, false | false | Whether the width/height is expressed as the size of the outer window, rather than the content area | Win32 Specific  |
        | monitorIndex | | -1 | | Win 32 Specific |
        | monitorHandle | | 0 (none) | | Win 32 Specific (OpenGL) |
        | enableDoubleClick | true, false | false | Enable the window to keep track and transmit double click messages. | Win32 Specific |
        | useNVPerfHUD | true, false | false | Enable the use of nVidia NVPerfHUD | DirectX Specific |
        | depthBuffer | true, false | true | Use depth buffer | DirectX9 Specific |
        | NSOpenGLCPSurfaceOrder | -1 or 1 | 1 | [NSOpenGLCPSurfaceOrder](https://developer.apple.com/documentation/appkit/nsopenglcpsurfaceorder) | Mac OS X Specific |
        | contentScalingFactor | Positive Float greater than 1.0 | The default content scaling factor of the screen | Specifies the CAEAGLLayer content scaling factor. Only supported on iOS 4 or greater. This can be useful to limit the resolution of the OpenGL ES backing store. For example, the iPhone 4's native resolution is 960 x 640\. Windows are always 320 x 480, if you would like to limit the display to 720 x 480, specify 1.5 as the scaling factor. | iOS Specific |
        | externalViewHandle | UIView pointer as an integer | 0 | External view handle, for rendering OGRE render in an existing view | iOS Specific |
        | externalViewControllerHandle | UIViewController pointer as an integer | 0 | External view controller handle, for embedding OGRE in an existing view controller | iOS Specific |
        | externalSharegroup | EAGLSharegroup pointer as an integer | 0 | External sharegroup, used to shared GL resources between contexts | iOS Specific |
        | MSAA | Positive integer (usually 0, 2, 4, 8, 16) | 0 | Full screen antialiasing factor | Android Specific |
        | CSAA | Positive integer (usually 0, 2, 4, 8, 16) | 0 | [Coverage sampling factor](https://www.khronos.org/registry/egl/extensions/NV/EGL_NV_coverage_sample.txt) | Android Specific |
        | maxColourBufferSize | Positive integer (usually 16, 32) | 32 | Max EGL_BUFFER_SIZE | Android Specific |
        | maxStencilBufferSize | Positive integer (usually 0, 8) | 0 | EGL_STENCIL_SIZE | Android Specific |
        | maxDepthBufferSize | Positive integer (usually 0, 16, 24) | 16 | EGL_DEPTH_SIZE | Android Specific |
        */
        virtual RenderWindow* _createRenderWindow(const String &name, unsigned int width, unsigned int height, 
            bool fullScreen, const NameValuePairList *miscParams = 0) = 0;

        /** Creates multiple rendering windows.     
        @param
        renderWindowDescriptions Array of structures containing the descriptions of each render window.
        The structure's members are the same as the parameters of _createRenderWindow:
        * name
        * width
        * height
        * fullScreen
        * miscParams
        See _createRenderWindow for details about each member.      
        @param
        createdWindows This array will hold the created render windows.
        @return
        true on success.        
        */
        virtual bool _createRenderWindows(const RenderWindowDescriptionList& renderWindowDescriptions, 
            RenderWindowList& createdWindows);

        
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

        /// Iterator over RenderTargets
        typedef MapIterator<Ogre::RenderTargetMap> RenderTargetIterator;

        /** Returns a specialised MapIterator over all render targets attached to the RenderSystem. */
        RenderTargetIterator getRenderTargetIterator(void) {
            return RenderTargetIterator( mRenderTargets.begin(), mRenderTargets.end() );
        }
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
        virtual void _useLights(const LightList& lights, unsigned short limit) {}
        /** Are fixed-function lights provided in view space? Affects optimisation. 
        */
        virtual bool areFixedFunctionLightsInViewSpace() const { return false; }
        /** Sets the world transform matrix.
         * @deprecated only needed for fixed function APIs */
        virtual void _setWorldMatrix(const Matrix4 &m) {}
        /** Sets the view transform matrix
         * @deprecated only needed for fixed function APIs */
        virtual void _setViewMatrix(const Matrix4 &m) {}
        /** Sets the projection transform matrix
         * @deprecated only needed for fixed function APIs*/
        virtual void _setProjectionMatrix(const Matrix4 &m) {}
        /** Utility function for setting all the properties of a texture unit at once.
        This method is also worth using over the individual texture unit settings because it
        only sets those settings which are different from the current settings for this
        unit, thus minimising render state changes.
        */
        virtual void _setTextureUnitSettings(size_t texUnit, TextureUnitState& tl);
        OGRE_DEPRECATED virtual void _setBindingType(TextureUnitState::BindingType bindigType) {}
        /** Turns off a texture unit. */
        virtual void _disableTextureUnit(size_t texUnit);
        /** Disables all texture units from the given unit upwards */
        virtual void _disableTextureUnitsFrom(size_t texUnit);
        /** Sets the surface properties to be used for future rendering.

        This method sets the the properties of the surfaces of objects
        to be rendered after it. In this context these surface properties
        are the amount of each type of light the object reflects (determining
        it's colour under different types of light), whether it emits light
        itself, and how shiny it is. Textures are not dealt with here,
        see the _setTetxure method for details.
        This method is used by _setMaterial so does not need to be called
        direct if that method is being used.

        @param ambient The amount of ambient (sourceless and directionless)
        light an object reflects. Affected by the colour/amount of ambient light in the scene.
        @param diffuse The amount of light from directed sources that is
        reflected (affected by colour/amount of point, directed and spot light sources)
        @param specular The amount of specular light reflected. This is also
        affected by directed light sources but represents the colour at the
        highlights of the object.
        @param emissive The colour of light emitted from the object. Note that
        this will make an object seem brighter and not dependent on lights in
        the scene, but it will not act as a light, so will not illuminate other
        objects. Use a light attached to the same SceneNode as the object for this purpose.
        @param shininess A value which only has an effect on specular highlights (so
        specular must be non-black). The higher this value, the smaller and crisper the
        specular highlights will be, imitating a more highly polished surface.
        This value is not constrained to 0.0-1.0, in fact it is likely to
        be more (10.0 gives a modest sheen to an object).
        @param tracking A bit field that describes which of the ambient, diffuse, specular
        and emissive colours follow the vertex colour of the primitive. When a bit in this field is set
        its ColourValue is ignored. This is a combination of TVC_AMBIENT, TVC_DIFFUSE, TVC_SPECULAR(note that the shininess value is still
        taken from shininess) and TVC_EMISSIVE. TVC_NONE means that there will be no material property
        tracking the vertex colours.
        @deprecated only needed for fixed function APIs
        */
        virtual void _setSurfaceParams(const ColourValue &ambient,
            const ColourValue &diffuse, const ColourValue &specular,
            const ColourValue &emissive, Real shininess,
            TrackVertexColourType tracking = TVC_NONE) {}

        /** Sets whether or not rendering points using OT_POINT_LIST will 
        render point sprites (textured quads) or plain points.
        @param enabled True enables point sprites, false returns to normal
        point rendering.
        @deprecated only needed for fixed function APIs
        */  
        virtual void _setPointSpritesEnabled(bool enabled) {};

        /** Sets the size of points and how they are attenuated with distance.
        @remarks
        When performing point rendering or point sprite rendering,
        point size can be attenuated with distance. The equation for
        doing this is attenuation = 1 / (constant + linear * dist + quadratic * d^2) .
        @par
        For example, to disable distance attenuation (constant screensize) 
        you would set constant to 1, and linear and quadratic to 0. A
        standard perspective attenuation would be 0, 1, 0 respectively.
        @deprecated only needed for fixed function APIs
        */
        virtual void _setPointParameters(Real size, bool attenuationEnabled, 
            Real constant, Real linear, Real quadratic, Real minSize, Real maxSize) {};

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

        /** Binds a texture to a vertex, geometry, compute, tesselation hull
        or tessellation domain sampler.
        @remarks
        Not all rendersystems support separate vertex samplers. For those that
        do, you can set a texture for them, separate to the regular texture
        samplers, using this method. For those that don't, you should use the
        regular texture samplers which are shared between the vertex and
        fragment units; calling this method will throw an exception.
        @deprecated only needed for D3D9
        */
        virtual void _setVertexTexture(size_t unit, const TexturePtr& tex);

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

        /** Sets a single filter for a given texture unit.
        @param unit The texture unit to set the filtering options for
        @param ftype The filter type
        @param filter The filter to be used
        */
        virtual void _setTextureUnitFiltering(size_t unit, FilterType ftype, FilterOptions filter) = 0;

        /** @overload
        @param unit The texture unit to set the filtering options for
        @param minFilter The filter used when a texture is reduced in size
        @param magFilter The filter used when a texture is magnified
        @param mipFilter The filter used between mipmap levels, FO_NONE disables mipmapping
        */
        virtual void _setTextureUnitFiltering(size_t unit, FilterOptions minFilter,
            FilterOptions magFilter, FilterOptions mipFilter);

        /** Sets whether the compare func is enabled or not for this texture unit 
        @param unit The texture unit to set the filtering options for
        @param compare The state (enabled/disabled)
        */
        virtual void _setTextureUnitCompareEnabled(size_t unit, bool compare) = 0;


        /** Sets the compare function to use for a given texture unit
        @param unit The texture unit to set the filtering options for
        @param function The comparison function
        */
        virtual void _setTextureUnitCompareFunction(size_t unit, CompareFunction function) = 0;


        /** Sets the maximal anisotropy for the specified texture unit.*/
        virtual void _setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy) = 0;

        /** Sets the texture addressing mode for a texture unit.*/
        virtual void _setTextureAddressingMode(size_t unit, const TextureUnitState::UVWAddressingMode& uvw) = 0;

        /** Sets the texture border colour for a texture unit.*/
        virtual void _setTextureBorderColour(size_t unit, const ColourValue& colour) = 0;

        /** Sets the mipmap bias value for a given texture unit.
        @remarks
        This allows you to adjust the mipmap calculation up or down for a
        given texture unit. Negative values force a larger mipmap to be used, 
        positive values force a smaller mipmap to be used. Units are in numbers
        of levels, so +1 forces the mipmaps to one smaller level.
        @note Only does something if render system has capability RSC_MIPMAP_LOD_BIAS.
        */
        virtual void _setTextureMipmapBias(size_t unit, float bias) = 0;

        /** Sets the texture coordinate transformation matrix for a texture unit.
        @param unit Texture unit to affect
        @param xform The 4x4 matrix
        @deprecated only needed for fixed function APIs
        */
        virtual void _setTextureMatrix(size_t unit, const Matrix4& xform) {}

        /** Sets the global blending factors for combining subsequent renders with the existing frame contents.
        The result of the blending operation is:
        <p align="center">final = (texture * sourceFactor) + (pixel * destFactor)</p>
        Each of the factors is specified as one of a number of options, as specified in the SceneBlendFactor
        enumerated type.
        By changing the operation you can change addition between the source and destination pixels to a different operator.
        @param sourceFactor The source factor in the above calculation, i.e. multiplied by the texture colour components.
        @param destFactor The destination factor in the above calculation, i.e. multiplied by the pixel colour components.
        @param op The blend operation mode for combining pixels
        */
        virtual void _setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op = SBO_ADD) = 0;

        /** Sets the global blending factors for combining subsequent renders with the existing frame contents.
        The result of the blending operation is:
        <p align="center">final = (texture * sourceFactor) + (pixel * destFactor)</p>
        Each of the factors is specified as one of a number of options, as specified in the SceneBlendFactor
        enumerated type.
        @param sourceFactor The source factor in the above calculation, i.e. multiplied by the texture colour components.
        @param destFactor The destination factor in the above calculation, i.e. multiplied by the pixel colour components.
        @param sourceFactorAlpha The source factor in the above calculation for the alpha channel, i.e. multiplied by the texture alpha components.
        @param destFactorAlpha The destination factor in the above calculation for the alpha channel, i.e. multiplied by the pixel alpha components.
        @param op The blend operation mode for combining pixels
        @param alphaOp The blend operation mode for combining pixel alpha values
        */
        virtual void _setSeparateSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, 
            SceneBlendFactor destFactorAlpha, SceneBlendOperation op = SBO_ADD, SceneBlendOperation alphaOp = SBO_ADD) = 0;

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
        virtual void _beginFrame(void) = 0;
        
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
        A typical way for the rendering engine to cull triangles is based on the
        'vertex winding' of triangles. Vertex winding refers to the direction in
        which the vertices are passed or indexed to in the rendering operation as viewed
        from the camera, and will wither be clockwise or anticlockwise (that's 'counterclockwise' for
        you Americans out there ;) The default is CULL_CLOCKWISE i.e. that only triangles whose vertices
        are passed/indexed in anticlockwise order are rendered - this is a common approach and is used in 3D studio models
        for example. You can alter this culling mode if you wish but it is not advised unless you know what you are doing.
        You may wish to use the CULL_NONE option for mesh data that you cull yourself where the vertex
        winding is uncertain.
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

        /** Sets whether or not the depth buffer check is performed before a pixel write.
        @param enabled If true, the depth buffer is tested for each pixel and the frame buffer is only updated
        if the depth function test succeeds. If false, no test is performed and pixels are always written.
        */
        virtual void _setDepthBufferCheckEnabled(bool enabled = true) = 0;
        /** Sets whether or not the depth buffer is updated after a pixel write.
        @param enabled If true, the depth buffer is updated with the depth of the new pixel if the depth test succeeds.
        If false, the depth buffer is left unchanged even if a new pixel is written.
        */
        virtual void _setDepthBufferWriteEnabled(bool enabled = true) = 0;
        /** Sets the comparison function for the depth buffer check.
        Advanced use only - allows you to choose the function applied to compare the depth values of
        new and existing pixels in the depth buffer. Only an issue if the deoth buffer check is enabled
        (see _setDepthBufferCheckEnabled)
        @param  func The comparison between the new depth and the existing depth which must return true
        for the new pixel to be written.
        */
        virtual void _setDepthBufferFunction(CompareFunction func = CMPF_LESS_EQUAL) = 0;
        /** Sets whether or not colour buffer writing is enabled, and for which channels. 
        @remarks
        For some advanced effects, you may wish to turn off the writing of certain colour
        channels, or even all of the colour channels so that only the depth buffer is updated
        in a rendering pass. However, the chances are that you really want to use this option
        through the Material class.
        @param red, green, blue, alpha Whether writing is enabled for each of the 4 colour channels. */
        virtual void _setColourBufferWriteEnabled(bool red, bool green, bool blue, bool alpha) = 0;
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
        /** Sets the fogging mode for future geometry.
        @param mode Set up the mode of fog as described in the FogMode enum, or set to FOG_NONE to turn off.
        @param colour The colour of the fog. Either set this to the same as your viewport background colour,
        or to blend in with a skydome or skybox.
        @param expDensity The density of the fog in FOG_EXP or FOG_EXP2 mode, as a value between 0 and 1. The default is 1. i.e. completely opaque, lower values can mean
        that fog never completely obscures the scene.
        @param linearStart Distance at which linear fog starts to encroach. The distance must be passed
        as a parametric value between 0 and 1, with 0 being the near clipping plane, and 1 being the far clipping plane. Only applicable if mode is FOG_LINEAR.
        @param linearEnd Distance at which linear fog becomes completely opaque.The distance must be passed
        as a parametric value between 0 and 1, with 0 being the near clipping plane, and 1 being the far clipping plane. Only applicable if mode is FOG_LINEAR.
        @deprecated only needed for fixed function APIs
        */
        virtual void _setFog(FogMode mode = FOG_NONE, const ColourValue& colour = ColourValue::White, Real expDensity = 1.0, Real linearStart = 0.0, Real linearEnd = 1.0) {}


        /** The RenderSystem will keep a count of tris rendered, this resets the count. */
        virtual void _beginGeometryCount(void);
        /** Reports the number of tris rendered since the last _beginGeometryCount call. */
        virtual unsigned int _getFaceCount(void) const;
        /** Reports the number of batches rendered since the last _beginGeometryCount call. */
        virtual unsigned int _getBatchCount(void) const;
        /** Reports the number of vertices passed to the renderer since the last _beginGeometryCount call. */
        virtual unsigned int _getVertexCount(void) const;

        /** Generates a packed data version of the passed in ColourValue suitable for
        use as with this RenderSystem.
        @remarks
        Since different render systems have different colour data formats (eg
        RGBA for GL, ARGB for D3D) this method allows you to use 1 method for all.
        @param colour The colour to convert
        @param pDest Pointer to location to put the result.
        */
        void convertColourValue(const ColourValue& colour, uint32* pDest);
        /** Get the native VertexElementType for a compact 32-bit colour value
        for this rendersystem.
        */
        virtual VertexElementType getColourVertexElementType(void) const = 0;

        /** Converts a uniform projection matrix to suitable for this render system.
        @remarks
        Because different APIs have different requirements (some incompatible) for the
        projection matrix, this method allows each to implement their own correctly and pass
        back a generic OGRE matrix for storage in the engine.
        */
        virtual void _convertProjectionMatrix(const Matrix4& matrix,
            Matrix4& dest, bool forGpuProgram = false) = 0;

        /** Builds a perspective projection matrix suitable for this render system.
        @remarks
        Because different APIs have different requirements (some incompatible) for the
        projection matrix, this method allows each to implement their own correctly and pass
        back a generic OGRE matrix for storage in the engine.
        */
        virtual void _makeProjectionMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane, 
            Matrix4& dest, bool forGpuProgram = false) = 0;

        /** Builds a perspective projection matrix for the case when frustum is
        not centered around camera.
        @remarks
        Viewport coordinates are in camera coordinate frame, i.e. camera is 
        at the origin.
        */
        virtual void _makeProjectionMatrix(Real left, Real right, Real bottom, Real top, 
            Real nearPlane, Real farPlane, Matrix4& dest, bool forGpuProgram = false) = 0;
        /** Builds an orthographic projection matrix suitable for this render system.
        @remarks
        Because different APIs have different requirements (some incompatible) for the
        projection matrix, this method allows each to implement their own correctly and pass
        back a generic OGRE matrix for storage in the engine.
        */
        virtual void _makeOrthoMatrix(const Radian& fovy, Real aspect, Real nearPlane, Real farPlane, 
            Matrix4& dest, bool forGpuProgram = false) = 0;

        /** Update a perspective projection matrix to use 'oblique depth projection'.
        @remarks
        This method can be used to change the nature of a perspective 
        transform in order to make the near plane not perpendicular to the 
        camera view direction, but to be at some different orientation. 
        This can be useful for performing arbitrary clipping (e.g. to a 
        reflection plane) which could otherwise only be done using user
        clip planes, which are more expensive, and not necessarily supported
        on all cards.
        @param matrix The existing projection matrix. Note that this must be a
        perspective transform (not orthographic), and must not have already
        been altered by this method. The matrix will be altered in-place.
        @param plane The plane which is to be used as the clipping plane. This
        plane must be in CAMERA (view) space.
        @param forGpuProgram Is this for use with a Gpu program or fixed-function
        */
        virtual void _applyObliqueDepthProjection(Matrix4& matrix, const Plane& plane, 
            bool forGpuProgram) = 0;

        /** Sets how to rasterise triangles, as points, wireframe or solid polys. */
        virtual void _setPolygonMode(PolygonMode level) = 0;

        /** Turns depth-stencil buffer checking on or off. 
        @remarks
        An inactive depth-stencil buffer can be read by a shader as a texture. An 
        application that reads a depth-stencil buffer as a texture renders in two
        passes, the first pass writes to the depth-stencil buffer and the second
        pass reads from the buffer. This allows a shader to compare depth or
        stencil values previously written to the buffer against the value for
        the pixel currrently being rendered. The result of the comparison can
        be used to create effects such as shadow mapping or soft particles
        in a particle system.
        */
        // virtual void setDepthCheckEnabled(bool enabled) = 0;

        /** Turns stencil buffer checking on or off. 
        @remarks
        Stencilling (masking off areas of the rendering target based on the stencil 
        buffer) can be turned on or off using this method. By default, stencilling is
        disabled.
        */
        virtual void setStencilCheckEnabled(bool enabled) = 0;
        /** Determines if this system supports hardware accelerated stencil buffer. 
        @remarks
        Note that the lack of this function doesn't mean you can't do stencilling, but
        the stencilling operations will be provided in software, which will NOT be
        fast.
        @par
        Generally hardware stencils are only supported in 32-bit colour modes, because
        the stencil buffer shares the memory of the z-buffer, and in most cards the 
        z-buffer has to be the same depth as the colour buffer. This means that in 32-bit
        mode, 24 bits of the z-buffer are depth and 8 bits are stencil. In 16-bit mode there
        is no room for a stencil (although some cards support a 15:1 depth:stencil option,
        this isn't useful for very much) so 8 bits of stencil are provided in software.
        This can mean that if you use stencilling, your applications may be faster in 
        32-but colour than in 16-bit, which may seem odd to some people.
        */
        /*virtual bool hasHardwareStencil(void) = 0;*/

        /** This method allows you to set all the stencil buffer parameters in one call.
        @remarks
        The stencil buffer is used to mask out pixels in the render target, allowing
        you to do effects like mirrors, cut-outs, stencil shadows and more. Each of
        your batches of rendering is likely to ignore the stencil buffer, 
        update it with new values, or apply it to mask the output of the render.
        The stencil test is:<PRE>
        (Reference Value & Mask) CompareFunction (Stencil Buffer Value & Mask)</PRE>
        The result of this will cause one of 3 actions depending on whether the test fails,
        succeeds but with the depth buffer check still failing, or succeeds with the
        depth buffer check passing too.
        @par
        Unlike other render states, stencilling is left for the application to turn
        on and off when it requires. This is because you are likely to want to change
        parameters between batches of arbitrary objects and control the ordering yourself.
        In order to batch things this way, you'll want to use OGRE's separate render queue
        groups (see RenderQueue) and register a RenderQueueListener to get notifications
        between batches.
        @par
        There are individual state change methods for each of the parameters set using 
        this method. 
        Note that the default values in this method represent the defaults at system 
        start up too.
        @param func The comparison function applied.
        @param refValue The reference value used in the comparison
        @param compareMask The bitmask applied to both the stencil value and the reference value 
        before comparison
        @param writeMask The bitmask the controls which bits from refValue will be written to 
        stencil buffer (valid for operations such as SOP_REPLACE).
        the stencil
        @param stencilFailOp The action to perform when the stencil check fails
        @param depthFailOp The action to perform when the stencil check passes, but the
        depth buffer check still fails
        @param passOp The action to take when both the stencil and depth check pass.
        @param twoSidedOperation If set to true, then if you render both back and front faces 
        (you'll have to turn off culling) then these parameters will apply for front faces, 
        and the inverse of them will happen for back faces (keep remains the same).
        @param readBackAsTexture D3D11 specific
        */
        virtual void setStencilBufferParams(CompareFunction func = CMPF_ALWAYS_PASS, 
            uint32 refValue = 0, uint32 compareMask = 0xFFFFFFFF, uint32 writeMask = 0xFFFFFFFF, 
            StencilOperation stencilFailOp = SOP_KEEP, 
            StencilOperation depthFailOp = SOP_KEEP,
            StencilOperation passOp = SOP_KEEP, 
            bool twoSidedOperation = false,
            bool readBackAsTexture = false) = 0;

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
            GpuProgramParametersSharedPtr params, uint16 variabilityMask) = 0;

        /** Only binds Gpu program parameters used for passes that have more than one iteration rendering
        */
        virtual void bindGpuProgramPassIterationParameters(GpuProgramType gptype) = 0;
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
        */
        virtual void setClipPlanes(const PlaneList& clipPlanes);

        /** Add a user clipping plane. */
        void addClipPlane (const Plane &p);

        /** Clears the user clipping region.
        */
        void resetClipPlanes();

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
        @note
        Not all systems support this method. Check the RenderSystemCapabilities for the
        RSC_SCISSOR_TEST capability to see if it is supported.
        @param enabled True to enable the scissor test, false to disable it.
        @param left, top, right, bottom The location of the corners of the rectangle, expressed in
        <i>pixels</i>.
        */
        virtual void setScissorTest(bool enabled, size_t left = 0, size_t top = 0, 
            size_t right = 800, size_t bottom = 600) = 0;

        /** Clears one or more frame buffers on the active render target. 
        @param buffers Combination of one or more elements of FrameBufferType
        denoting which buffers are to be cleared
        @param colour The colour to clear the colour buffer with, if enabled
        @param depth The value to initialise the depth buffer with, if enabled
        @param stencil The value to initialise the stencil buffer with, if enabled.
        */
        virtual void clearFrameBuffer(unsigned int buffers, 
            const ColourValue& colour = ColourValue::Black, 
            Real depth = 1.0f, unsigned short stencil = 0) = 0;
        /** Returns the horizontal texel offset value required for mapping 
        texel origins to pixel origins in this rendersystem.
        @remarks
        Since rendersystems sometimes disagree on the origin of a texel, 
        mapping from texels to pixels can sometimes be problematic to 
        implement generically. This method allows you to retrieve the offset
        required to map the origin of a texel to the origin of a pixel in
        the horizontal direction.
        */
        virtual Real getHorizontalTexelOffset(void) = 0;
        /** Returns the vertical texel offset value required for mapping 
        texel origins to pixel origins in this rendersystem.
        @remarks
        Since rendersystems sometimes disagree on the origin of a texel, 
        mapping from texels to pixels can sometimes be problematic to 
        implement generically. This method allows you to retrieve the offset
        required to map the origin of a texel to the origin of a pixel in
        the vertical direction.
        */
        virtual Real getVerticalTexelOffset(void) = 0;

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
        virtual void preExtraThreadsStarted() = 0;

        /** Tell the rendersystem to perform any tasks it needs to directly
        after other threads which might access the rendering API are registered.
        @see RenderSystem::preExtraThreadsStarted
        */
        virtual void postExtraThreadsStarted() = 0;

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
        virtual void registerThread() = 0;

        /** Unregister an additional thread which may make calls to rendersystem-related objects.
        @see RenderSystem::registerThread
        */
        virtual void unregisterThread() = 0;

        /**
        * Gets the number of display monitors.
        @see Root::getDisplayMonitorCount
        */
        virtual unsigned int getDisplayMonitorCount() const = 0;

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

        /** Determines if the system has anisotropic mip map filter support
        */
        virtual bool hasAnisotropicMipMapFilter() const = 0;

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

        bool mWBuffer;

        size_t mBatchCount;
        size_t mFaceCount;
        size_t mVertexCount;

        /// Saved manual colour blends
        ColourValue mManualBlendColours[OGRE_MAX_TEXTURE_LAYERS][2];

        bool mInvertVertexWinding;

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

        /// Internal method used to set the underlying clip planes when needed
        virtual void setClipPlanesImpl(const PlaneList& clipPlanes) = 0;

        /** Initialize the render system from the capabilities*/
        virtual void initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary) = 0;


        DriverVersion mDriverVersion;
        uint16 mNativeShadingLanguageVersion;

        bool mTexProjRelative;
        Vector3 mTexProjRelativeOrigin;



    };
    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
