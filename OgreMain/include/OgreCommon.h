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
#ifndef __Common_H__
#define __Common_H__
// Common stuff

#include "OgreVector.h"
#include "OgreHeaderPrefix.h"
#include "OgreMurmurHash3.h"

namespace Ogre {

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup General
    *  @{
    */

    /// Fast general hashing algorithm
    inline uint32 FastHash (const char * data, size_t len, uint32 hashSoFar = 0) {
        uint32 ret;
        MurmurHash3_x86_32(data, len, hashSoFar, &ret);
        return ret;
    }
    /// Combine hashes with same style as boost::hash_combine
    template <typename T>
    uint32 HashCombine (uint32 hashSoFar, const T& data)
    {
        return FastHash((const char*)&data, sizeof(T), hashSoFar);
    }


    /** Comparison functions used for the depth/stencil buffer operations and 
        others. */
    enum CompareFunction : uint8
    {
        CMPF_ALWAYS_FAIL,  //!< Never writes a pixel to the render target
        CMPF_ALWAYS_PASS,  //!< Always writes a pixel to the render target
        CMPF_LESS,         //!< Write if (new_Z < existing_Z)
        CMPF_LESS_EQUAL,   //!< Write if (new_Z <= existing_Z)
        CMPF_EQUAL,        //!< Write if (new_Z == existing_Z)
        CMPF_NOT_EQUAL,    //!< Write if (new_Z != existing_Z)
        CMPF_GREATER_EQUAL,//!< Write if (new_Z >= existing_Z)
        CMPF_GREATER       //!< Write if (new_Z >= existing_Z)
    };

    /** High-level filtering options providing shortcuts to settings the
        minification, magnification and mip filters. */
    enum TextureFilterOptions
    {
        /// No filtering or mipmapping is used. 
        /// Equal to: min=Ogre::FO_POINT, mag=Ogre::FO_POINT, mip=Ogre::FO_NONE
        TFO_NONE,
        /// 2x2 box filtering is performed when magnifying or reducing a texture, and a mipmap is picked from the list but no filtering is done between the levels of the mipmaps. 
        /// Equal to: min=Ogre::FO_LINEAR, mag=Ogre::FO_LINEAR, mip=Ogre::FO_POINT
        TFO_BILINEAR,
        /// 2x2 box filtering is performed when magnifying and reducing a texture, and the closest 2 mipmaps are filtered together. 
        /// Equal to: min=Ogre::FO_LINEAR, mag=Ogre::FO_LINEAR, mip=Ogre::FO_LINEAR
        TFO_TRILINEAR,
        /// This is the same as ’trilinear’, except the filtering algorithm takes account of the slope of the triangle in relation to the camera rather than simply doing a 2x2 pixel filter in all cases.
        /// Equal to: min=Ogre::FO_ANISOTROPIC, max=Ogre::FO_ANISOTROPIC, mip=Ogre::FO_LINEAR
        TFO_ANISOTROPIC
    };

    enum FilterType
    {
        /// The filter used when shrinking a texture
        FT_MIN,
        /// The filter used when magnifying a texture
        FT_MAG,
        /// The filter used when determining the mipmap
        FT_MIP
    };
    /** Filtering options for textures / mipmaps. */
    enum FilterOptions : uint8
    {
        /// No filtering, used for FT_MIP to turn off mipmapping
        FO_NONE,
        /// Use the closest pixel
        FO_POINT,
        /// Average of a 2x2 pixel area, denotes bilinear for MIN and MAG, trilinear for MIP
        FO_LINEAR,
        /// Similar to FO_LINEAR, but compensates for the angle of the texture plane. Note that in
        /// order for this to make any difference, you must also set the
        /// TextureUnitState::setTextureAnisotropy attribute too.
        FO_ANISOTROPIC
    };

    /** Texture addressing modes - default is TAM_WRAP.
    */
    enum TextureAddressingMode : uint8
    {
        /// %Any value beyond 1.0 wraps back to 0.0. %Texture is repeated.
        TAM_WRAP,
        /// %Texture flips every boundary, meaning texture is mirrored every 1.0 u or v
        TAM_MIRROR,
        /// Values beyond 1.0 are clamped to 1.0. %Texture ’streaks’ beyond 1.0 since last line
        /// of pixels is used across the rest of the address space. Useful for textures which
        /// need exact coverage from 0.0 to 1.0 without the ’fuzzy edge’ wrap gives when
        /// combined with filtering.
        TAM_CLAMP,
        /// %Texture coordinates outside the range [0.0, 1.0] are set to the border colour.
        TAM_BORDER
    };

    /** %Light shading modes. */
    enum ShadeOptions : uint8
    {
        SO_FLAT, //!< No interpolation takes place. Each face is shaded with a single colour determined from the first vertex in the face.
        SO_GOURAUD, //!< Colour at each vertex is linearly interpolated across the face.
        SO_PHONG //!< Vertex normals are interpolated across the face, and these are used to determine colour at each pixel. Gives a more natural lighting effect but is more expensive and works better at high levels of tessellation. Not supported on all hardware.
    };

    /** Fog modes. */
    enum FogMode : uint8
    {
        /// No fog. Duh.
        FOG_NONE,
        /// Fog density increases  exponentially from the camera (fog = 1/e^(distance * density))
        FOG_EXP,
        /// Fog density increases at the square of FOG_EXP, i.e. even quicker (fog = 1/e^(distance * density)^2)
        FOG_EXP2,
        /// Fog density increases linearly between the start and end distances
        FOG_LINEAR
    };

    /** Hardware culling modes based on vertex winding.
        This setting applies to how the hardware API culls triangles it is sent. */
    enum CullingMode : uint8
    {
        /// Hardware never culls triangles and renders everything it receives.
        CULL_NONE = 1,
        /// Hardware culls triangles whose vertices are listed clockwise in the view (default).
        CULL_CLOCKWISE = 2,
        /// Hardware culls triangles whose vertices are listed anticlockwise in the view.
        CULL_ANTICLOCKWISE = 3
    };

    /** Manual culling modes based on vertex normals.
        This setting applies to how the software culls triangles before sending them to the 
        hardware API. This culling mode is used by scene managers which choose to implement it -
        normally those which deal with large amounts of fixed world geometry which is often 
        planar (software culling movable variable geometry is expensive). */
    enum ManualCullingMode : uint8
    {
        /// No culling so everything is sent to the hardware.
        MANUAL_CULL_NONE = 1,
        /// Cull triangles whose normal is pointing away from the camera (default).
        MANUAL_CULL_BACK = 2,
        /// Cull triangles whose normal is pointing towards the camera.
        MANUAL_CULL_FRONT = 3
    };

    /** Enumerates the wave types usable with the Ogre engine. */
    enum WaveformType
    {
        /// Standard sine wave which smoothly changes from low to high and back again.
        WFT_SINE,
        /// An angular wave with a constant increase / decrease speed with pointed peaks.
        WFT_TRIANGLE,
        /// Half of the time is spent at the min, half at the max with instant transition between.
        WFT_SQUARE,
        /// Gradual steady increase from min to max over the period with an instant return to min at the end.
        WFT_SAWTOOTH,
        /// Gradual steady decrease from max to min over the period, with an instant return to max at the end.
        WFT_INVERSE_SAWTOOTH,
        /// Pulse Width Modulation. Works like WFT_SQUARE, except the high to low transition is controlled by duty cycle. 
        /// With a duty cycle of 50% (0.5) will give the same output as WFT_SQUARE.
        WFT_PWM
    };

    /** The polygon mode to use when rasterising. */
    enum PolygonMode : uint8
    {
        /// Only the points of each polygon are rendered.
        PM_POINTS = 1,
        /// Polygons are drawn in outline only.
        PM_WIREFRAME = 2,
        /// The normal situation - polygons are filled in.
        PM_SOLID = 3
    };

    /** An enumeration of broad shadow techniques */
    enum ShadowTechnique
    {
        /** No shadows */
        SHADOWTYPE_NONE = 0x00,
        /** Mask for additive shadows (not for direct use, use  SHADOWTYPE_ enum instead)
        */
        SHADOWDETAILTYPE_ADDITIVE = 0x01,
        /** Mask for modulative shadows (not for direct use, use  SHADOWTYPE_ enum instead)
        */
        SHADOWDETAILTYPE_MODULATIVE = 0x02,
        /** Mask for integrated shadows (not for direct use, use SHADOWTYPE_ enum instead)
        */
        SHADOWDETAILTYPE_INTEGRATED = 0x04,
        /** Mask for stencil shadows (not for direct use, use  SHADOWTYPE_ enum instead)
        */
        SHADOWDETAILTYPE_STENCIL = 0x10,
        /** Mask for texture shadows (not for direct use, use  SHADOWTYPE_ enum instead)
        */
        SHADOWDETAILTYPE_TEXTURE = 0x20,
        
        /** Stencil shadow technique which renders all shadow volumes as
            a modulation after all the non-transparent areas have been 
            rendered. This technique is considerably less fillrate intensive 
            than the additive stencil shadow approach when there are multiple
            lights, but is not an accurate model. 
        */
        SHADOWTYPE_STENCIL_MODULATIVE = SHADOWDETAILTYPE_STENCIL | SHADOWDETAILTYPE_MODULATIVE,
        /** Stencil shadow technique which renders each light as a separate
            additive pass to the scene. This technique can be very fillrate
            intensive because it requires at least 2 passes of the entire
            scene, more if there are multiple lights. However, it is a more
            accurate model than the modulative stencil approach and this is
            especially apparent when using coloured lights or bump mapping.
        */
        SHADOWTYPE_STENCIL_ADDITIVE = SHADOWDETAILTYPE_STENCIL | SHADOWDETAILTYPE_ADDITIVE,
        /** Texture-based shadow technique which involves a monochrome render-to-texture
            of the shadow caster and a projection of that texture onto the 
            shadow receivers as a modulative pass. 
        */
        SHADOWTYPE_TEXTURE_MODULATIVE = SHADOWDETAILTYPE_TEXTURE | SHADOWDETAILTYPE_MODULATIVE,
        
        /** Texture-based shadow technique which involves a render-to-texture
            of the shadow caster and a projection of that texture onto the 
            shadow receivers, built up per light as additive passes. 
            This technique can be very fillrate intensive because it requires numLights + 2 
            passes of the entire scene. However, it is a more accurate model than the 
            modulative approach and this is especially apparent when using coloured lights 
            or bump mapping.
        */
        SHADOWTYPE_TEXTURE_ADDITIVE = SHADOWDETAILTYPE_TEXTURE | SHADOWDETAILTYPE_ADDITIVE,

        /** Texture-based shadow technique which involves a render-to-texture
        of the shadow caster and a projection of that texture on to the shadow
        receivers, with the usage of those shadow textures completely controlled
        by the materials of the receivers.
        This technique is easily the most flexible of all techniques because 
        the material author is in complete control over how the shadows are
        combined with regular rendering. It can perform shadows as accurately
        as SHADOWTYPE_TEXTURE_ADDITIVE but more efficiently because it requires
        less passes. However it also requires more expertise to use, and 
        in almost all cases, shader capable hardware to really use to the full.
        @note The 'additive' part of this mode means that the colour of
        the rendered shadow texture is by default plain black. It does
        not mean it does the adding on your receivers automatically though, how you
        use that result is up to you.
        */
        SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED = SHADOWTYPE_TEXTURE_ADDITIVE | SHADOWDETAILTYPE_INTEGRATED,
        /** Texture-based shadow technique which involves a render-to-texture
            of the shadow caster and a projection of that texture on to the shadow
            receivers, with the usage of those shadow textures completely controlled
            by the materials of the receivers.
            This technique is easily the most flexible of all techniques because 
            the material author is in complete control over how the shadows are
            combined with regular rendering. It can perform shadows as accurately
            as SHADOWTYPE_TEXTURE_ADDITIVE but more efficiently because it requires
            less passes. However it also requires more expertise to use, and 
            in almost all cases, shader capable hardware to really use to the full.
            @note The 'modulative' part of this mode means that the colour of
            the rendered shadow texture is by default the 'shadow colour'. It does
            not mean it modulates on your receivers automatically though, how you
            use that result is up to you.
        */
        SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED = SHADOWTYPE_TEXTURE_MODULATIVE | SHADOWDETAILTYPE_INTEGRATED
    };

    /** An enumeration describing which material properties should track the vertex colours */
    typedef int TrackVertexColourType;
    enum TrackVertexColourEnum {
        TVC_NONE        = 0x0,
        TVC_AMBIENT     = 0x1,        
        TVC_DIFFUSE     = 0x2,
        TVC_SPECULAR    = 0x4,
        TVC_EMISSIVE    = 0x8
    };

    /** Function used compute the camera-distance for sorting objects */
    enum SortMode : uint8
    {

        /** Sort by direction of the camera
         *
         * The distance along the camera view as in `cam->getDerivedDirection().dotProduct(diff)`
         * Best for @ref PT_ORTHOGRAPHIC
         */
        SM_DIRECTION,
        /** Sort by distance from the camera
         *
         * The euclidean distance as in `diff.squaredLength()`
         * Best for @ref PT_PERSPECTIVE
         */
        SM_DISTANCE
    };

    /** Defines the frame buffer types. */
    enum FrameBufferType {
        FBT_COLOUR  = 0x1,
        FBT_DEPTH   = 0x2,
        FBT_STENCIL = 0x4
    };
	
	/** Defines the colour buffer types. */
    enum ColourBufferType
    {
      CBT_BACK = 0x0,
      CBT_BACK_LEFT,
      CBT_BACK_RIGHT
    };

    /** Flags for the Instance Manager when calculating ideal number of instances per batch */
    enum InstanceManagerFlags
    {
        /** Forces an amount of instances per batch low enough so that vertices * numInst < 65535
            since usually improves performance. In HW instanced techniques, this flag is ignored
        */
        IM_USE16BIT     = 0x0001,

        /** The num. of instances is adjusted so that as few pixels as possible are wasted
            in the vertex texture */
        IM_VTFBESTFIT   = 0x0002,

        /** Use a limited number of skeleton animations shared among all instances. 
        Update only that limited amount of animations in the vertex texture.*/
        IM_VTFBONEMATRIXLOOKUP = 0x0004,

        IM_USEBONEDUALQUATERNIONS = 0x0008,

        /** Use one weight per vertex when recommended (i.e. VTF). */
        IM_USEONEWEIGHT = 0x0010,

        /** All techniques are forced to one weight per vertex. */
        IM_FORCEONEWEIGHT = 0x0020,

        IM_USEALL       = IM_USE16BIT|IM_VTFBESTFIT|IM_USEONEWEIGHT
    };
    
    class Light;
    typedef std::vector<Light*> LightList;


    /// Constant blank string, useful for returning by ref where local does not exist
    const String BLANKSTRING;

    typedef std::map<String, bool> UnaryOptionList;
    typedef std::map<String, String> BinaryOptionList;

    /// Name / value parameter pair (first = name, second = value)
    typedef std::map<String, String> NameValuePairList;

    /// Alias / Texture name pair (first = alias, second = texture name)
    typedef std::map<String, String> AliasTextureNamePairList;

        template< typename T > struct TRect
        {
          T left, top, right, bottom;
          TRect() : left(0), top(0), right(0), bottom(0) {}
          TRect( T const & l, T const & t, T const & r, T const & b )
            : left( l ), top( t ), right( r ), bottom( b )
          {
          }
          TRect( TRect const & o )
            : left( o.left ), top( o.top ), right( o.right ), bottom( o.bottom )
          {
          }
          TRect & operator=( TRect const & o )
          {
            left = o.left;
            top = o.top;
            right = o.right;
            bottom = o.bottom;
            return *this;
          }
          T width() const
          {
            return right - left;
          }
          T height() const
          {
            return bottom - top;
          }
          bool isNull() const
          {
              return width() == 0 || height() == 0;
          }
          void setNull()
          {
              left = right = top = bottom = 0;
          }
          TRect & merge(const TRect& rhs)
          {
              assert(right >= left && bottom >= top);
              assert(rhs.right >= rhs.left && rhs.bottom >= rhs.top);
              if (isNull())
              {
                  *this = rhs;
              }
              else if (!rhs.isNull())
              {
                  left = std::min(left, rhs.left);
                  right = std::max(right, rhs.right);
                  top = std::min(top, rhs.top);
                  bottom = std::max(bottom, rhs.bottom);
              }

              return *this;

          }

          /**
           * Returns the intersection of the two rectangles.
           *
           * Note that the rectangles extend downwards. I.e. a valid box will
           * have "right > left" and "bottom > top".
           * @param rhs Another rectangle.
           * @return The intersection of the two rectangles. Zero size if they don't intersect.
           */
          TRect intersect(const TRect& rhs) const
          {
              assert(right >= left && bottom >= top);
              assert(rhs.right >= rhs.left && rhs.bottom >= rhs.top);
              TRect ret;
              if (isNull() || rhs.isNull())
              {
                  // empty
                  return ret;
              }
              else
              {
                  ret.left = std::max(left, rhs.left);
                  ret.right = std::min(right, rhs.right);
                  ret.top = std::max(top, rhs.top);
                  ret.bottom = std::min(bottom, rhs.bottom);
              }

              if (ret.left > ret.right || ret.top > ret.bottom)
              {
                  // no intersection, return empty
                  ret.left = ret.top = ret.right = ret.bottom = 0;
              }

              return ret;

          }
          bool operator==(const TRect& rhs) const
          {
              return left == rhs.left && right == rhs.right && top == rhs.top && bottom == rhs.bottom;
          }
          bool operator!=(const TRect& rhs) const { return !(*this == rhs); }
        };
        template<typename T>
        std::ostream& operator<<(std::ostream& o, const TRect<T>& r)
        {
            o << "TRect<>(l:" << r.left << ", t:" << r.top << ", r:" << r.right << ", b:" << r.bottom << ")";
            return o;
        }

        /** Structure used to define a rectangle in a 2-D floating point space.
        */
        typedef TRect<float> FloatRect;

        /** Structure used to define a rectangle in a 2-D floating point space, 
            subject to double / single floating point settings.
        */
        typedef TRect<Real> RealRect;

        /** Structure used to define a rectangle in a 2-D integer space.
        */
        typedef TRect< int32 > Rect;

        /** Structure used to define a box in a 3-D integer space.
            Note that the left, top, and front edges are included but the right,
            bottom and back ones are not.
         */
        struct Box
        {
            uint32 left, top, right, bottom, front, back;
            /// Parameterless constructor for setting the members manually
            Box()
                : left(0), top(0), right(1), bottom(1), front(0), back(1)
            {
            }
            /** Define a box from left, top, right and bottom coordinates
                This box will have depth one (front=0 and back=1).
                @param  l   x value of left edge
                @param  t   y value of top edge
                @param  r   x value of right edge
                @param  b   y value of bottom edge
                @note @copydetails Ogre::Box
            */
            Box( uint32 l, uint32 t, uint32 r, uint32 b ):
                left(l),
                top(t),   
                right(r),
                bottom(b),
                front(0),
                back(1)
            {
                assert(right >= left && bottom >= top && back >= front);
            }

            /// @overload
            template <typename T> explicit Box(const TRect<T>& r) : Box(r.left, r.top, r.right, r.bottom) {}

            /** Define a box from left, top, front, right, bottom and back
                coordinates.
                @param  l   x value of left edge
                @param  t   y value of top edge
                @param  ff  z value of front edge
                @param  r   x value of right edge
                @param  b   y value of bottom edge
                @param  bb  z value of back edge
                @note @copydetails Ogre::Box
            */
            Box( uint32 l, uint32 t, uint32 ff, uint32 r, uint32 b, uint32 bb ):
                left(l),
                top(t),   
                right(r),
                bottom(b),
                front(ff),
                back(bb)
            {
                assert(right >= left && bottom >= top && back >= front);
            }

            /// @overload
            explicit Box(const Vector3i& size)
                : left(0), top(0), right(size[0]), bottom(size[1]), front(0), back(size[2])
            {
            }

            /// Return true if the other box is a part of this one
            bool contains(const Box &def) const
            {
                return (def.left >= left && def.top >= top && def.front >= front &&
                    def.right <= right && def.bottom <= bottom && def.back <= back);
            }
            
            /// Get the width of this box
            uint32 getWidth() const { return right-left; }
            /// Get the height of this box
            uint32 getHeight() const { return bottom-top; }
            /// Get the depth of this box
            uint32 getDepth() const { return back-front; }

            /// origin (top, left, front) of the box
            Vector3i getOrigin() const { return Vector3i(left, top, front); }
            /// size (width, height, depth) of the box
            Vector3i getSize() const { return Vector3i(getWidth(), getHeight(), getDepth()); }
        };

    
    
    /** Locate command-line options of the unary form '-blah' and of the
        binary form '-blah foo', passing back the index of the next non-option.
    @param numargs, argv The standard parameters passed to the main method
    @param unaryOptList Map of unary options (i.e. those that do not require a parameter).
        Should be pre-populated with, for example '-e' in the key and false in the 
        value. Options which are found will be set to true on return.
    @param binOptList Map of binary options (i.e. those that require a parameter
        e.g. '-e afile.txt').
        Should be pre-populated with, for example '-e' and the default setting. 
        Options which are found will have the value updated.
    */
    int _OgreExport findCommandLineOpts(int numargs, char** argv, UnaryOptionList& unaryOptList, 
        BinaryOptionList& binOptList);

    /// Generic result of clipping
    enum ClipResult
    {
        /// Nothing was clipped
        CLIPPED_NONE = 0,
        /// Partially clipped
        CLIPPED_SOME = 1, 
        /// Everything was clipped away
        CLIPPED_ALL = 2
    };

    /// Render window creation parameters.
    struct RenderWindowDescription
    {
        String              name;
        unsigned int        width;
        unsigned int        height;
        bool                useFullScreen;
        NameValuePairList   miscParams;
    };

    /// Render window creation parameters container.
    typedef std::vector<RenderWindowDescription> RenderWindowDescriptionList;

    /// Render window container.
    typedef std::vector<RenderWindow*> RenderWindowList;

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
