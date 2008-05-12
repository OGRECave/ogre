/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __BLENDMODE_H__
#define __BLENDMODE_H__

#include "OgrePrerequisites.h"
#include "OgreColourValue.h"

namespace Ogre {

    /** Type of texture blend mode.
    */
    enum LayerBlendType
    {
        LBT_COLOUR,
        LBT_ALPHA
    };

    /** List of valid texture blending operations, for use with TextureUnitState::setColourOperation.
        @remarks
            This list is a more limited list than LayerBlendOperationEx because it only
            includes operations that are supportable in both multipass and multitexture
            rendering and thus provides automatic fallback if multitexture hardware
            is lacking or insufficient.
    */
    enum LayerBlendOperation {
        /// Replace all colour with texture with no adjustment
        LBO_REPLACE,
        /// Add colour components together.
        LBO_ADD,
        /// Multiply colour components together.
        LBO_MODULATE,
        /// Blend based on texture alpha
        LBO_ALPHA_BLEND

    };

    /** Expert list of valid texture blending operations, for use with TextureUnitState::setColourOperationEx
        and TextureUnitState::setAlphaOperation, and internally in the LayerBlendModeEx class. It's worth
        noting that these operations are for blending <i>between texture layers</i> and not between rendered objects
        and the existing scene. Because all of these modes are only supported in multitexture hardware it may be
        required to set up a fallback operation where this hardware is not available.
    */
    enum LayerBlendOperationEx {
        /// use source1 without modification
        LBX_SOURCE1,
        /// use source2 without modification
        LBX_SOURCE2,
        /// multiply source1 and source2 together
        LBX_MODULATE,
        /// as LBX_MODULATE but brighten afterwards (x2)
        LBX_MODULATE_X2,
        /// as LBX_MODULATE but brighten more afterwards (x4)
        LBX_MODULATE_X4,
        /// add source1 and source2 together
        LBX_ADD,
        /// as LBX_ADD, but subtract 0.5 from the result
        LBX_ADD_SIGNED,
        /// as LBX_ADD, but subtract product from the sum
        LBX_ADD_SMOOTH,
        /// subtract source2 from source1
        LBX_SUBTRACT,
        /// use interpolated alpha value from vertices to scale source1, then add source2 scaled by (1-alpha)
        LBX_BLEND_DIFFUSE_ALPHA,
        /// as LBX_BLEND_DIFFUSE_ALPHA, but use alpha from texture
        LBX_BLEND_TEXTURE_ALPHA,
        /// as LBX_BLEND_DIFFUSE_ALPHA, but use current alpha from previous stages
        LBX_BLEND_CURRENT_ALPHA,
        /// as LBX_BLEND_DIFFUSE_ALPHA but use a constant manual blend value (0.0-1.0)
        LBX_BLEND_MANUAL,
        /// dot product of color1 and color2 
        LBX_DOTPRODUCT,
        /// use interpolated color values from vertices to scale source1, then add source2 scaled by (1-color)
        LBX_BLEND_DIFFUSE_COLOUR
    };

    /** List of valid sources of values for blending operations used
        in TextureUnitState::setColourOperation and TextureUnitState::setAlphaOperation,
        and internally in the LayerBlendModeEx class.
    */
    enum LayerBlendSource
    {
        /// the colour as built up from previous stages
        LBS_CURRENT,
        /// the colour derived from the texture assigned to this layer
        LBS_TEXTURE,
        /// the interpolated diffuse colour from the vertices
        LBS_DIFFUSE,
        /// the interpolated specular colour from the vertices
        LBS_SPECULAR,
        /// a colour supplied manually as a separate argument
        LBS_MANUAL
    };
    /** Class which manages blending of both colour and alpha components.
        @remarks
            This class is a utility class used by both TextureUnitState and
            RenderSystem to wrap up the details of a blending operation. This blending
            operation could be used for blending colour or alpha in a texture layer.
            This class is really only for use by OGRE, since apps can deal with
            blending modes through the TextureUnitState class methods
            setColourOperation and setAlphaOperation.
        @par
            It's worth noting that these operations are for blending <i>between texture
            layers</i> and not between rendered objects and the existing scene. If
            you wish to make an object blend with others in the scene, e.g. to make
            transparent objects etc, use the Material::setSceneBlending method.
    */
    class _OgreExport LayerBlendModeEx
    {
    public:
        /// The type of blending (colour or alpha)
        LayerBlendType blendType;
        /// The operation to be applied
        LayerBlendOperationEx operation;
        /// The first source of colour/alpha
        LayerBlendSource source1;
        /// The second source of colour/alpha
        LayerBlendSource source2;

        /// Manual colour value for manual source1
        ColourValue colourArg1;
        /// Manual colour value for manual source2
        ColourValue colourArg2;
        /// Manual alpha value for manual source1
        Real alphaArg1;
        /// Manual alpha value for manual source2
        Real alphaArg2;
        /// Manual blending factor
        Real factor;

        bool operator==(const LayerBlendModeEx& rhs) const
        {
            if (blendType != rhs.blendType) return false;

            if (blendType == LBT_COLOUR)
            {

                if (operation == rhs.operation &&
                    source1 == rhs.source1 &&
                    source2 == rhs.source2 &&
                    colourArg1 == rhs.colourArg1 &&
                    colourArg2 == rhs.colourArg2 &&
                    factor == rhs.factor)
                {
                    return true;
                }
            }
            else // if (blendType == LBT_ALPHA)
            {
                if (operation == rhs.operation &&
                    source1 == rhs.source1 &&
                    source2 == rhs.source2 &&
                    alphaArg1 == rhs.alphaArg1 &&
                    alphaArg2 == rhs.alphaArg2 &&
                    factor == rhs.factor)
                {
                    return true;
                }
            }
            return false;
        }

        bool operator!=(const LayerBlendModeEx& rhs) const
        {
            return !(*this == rhs);
        }



    };

    /** Types of blending that you can specify between an object and the existing contents of the scene.
        @remarks
            As opposed to the LayerBlendType, which classifies blends between texture layers, these blending
            types blend between the output of the texture units and the pixels already in the viewport,
            allowing for object transparency, glows, etc.
        @par
            These types are provided to give quick and easy access to common effects. You can also use
            the more manual method of supplying source and destination blending factors.
            See Material::setSceneBlending for more details.
        @see
            Material::setSceneBlending
    */
    enum SceneBlendType
    {
        /// Make the object transparent based on the final alpha values in the texture
        SBT_TRANSPARENT_ALPHA,
        /// Make the object transparent based on the colour values in the texture (brighter = more opaque)
        SBT_TRANSPARENT_COLOUR,
        /// Add the texture values to the existing scene content
        SBT_ADD,
		/// Multiply the 2 colours together
		SBT_MODULATE,
        /// The default blend mode where source replaces destination
        SBT_REPLACE
        // TODO : more
    };

    /** Blending factors for manually blending objects with the scene. If there isn't a predefined
        SceneBlendType that you like, then you can specify the blending factors directly to affect the
        combination of object and the existing scene. See Material::setSceneBlending for more details.
    */
    enum SceneBlendFactor
    {
        SBF_ONE,
        SBF_ZERO,
        SBF_DEST_COLOUR,
        SBF_SOURCE_COLOUR,
        SBF_ONE_MINUS_DEST_COLOUR,
        SBF_ONE_MINUS_SOURCE_COLOUR,
        SBF_DEST_ALPHA,
        SBF_SOURCE_ALPHA,
        SBF_ONE_MINUS_DEST_ALPHA,
        SBF_ONE_MINUS_SOURCE_ALPHA

    };
}

#endif
