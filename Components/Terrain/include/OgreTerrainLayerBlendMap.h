/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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

#ifndef __Ogre_TerrainLayerBlendMap_H__
#define __Ogre_TerrainLayerBlendMap_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreCommon.h"
#include "OgreVector3.h"
#include "OgreDataStream.h"

namespace Ogre
{
	class Image;
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Terrain
	*  Some details on the terrain component
	*  @{
	*/


	/** Class exposing an interface to a blend map for a given layer. 
	Each layer after the first layer in a terrain has a blend map which 
	expresses how it is alpha blended with the layers beneath. Internally, this
	blend map is packed into one channel of an RGB or RGBA texture in
	order to use the smallest number of samplers, but this class allows
	a caller to manipulate the data more easily without worrying about
	this packing. 
	@par
	You shouldn't construct this class directly, use Terrain::getLayerBlendMap().
	*/
	class _OgreTerrainExport TerrainLayerBlendMap : public TerrainAlloc
	{
	protected:
		Terrain* mParent;
		uint8 mLayerIdx;
		uint8 mChannel; // RGBA
		uint8 mChannelOffset; // in pixel format
		Box mDirtyBox;
		bool mDirty;
		HardwarePixelBuffer* mBuffer;
		uint8* mData;

		void download();
		void upload();

	public:
		/** Constructor
		@param parent The parent terrain
		@param layerIndex The layer index (should be 1 or higher)
		@param box The region of the blend map to address (in image space)
		@param buf The buffer holding the data
		*/
		TerrainLayerBlendMap(Terrain* parent, uint8 layerIndex, HardwarePixelBuffer* buf);
		~TerrainLayerBlendMap();
		/// Get the parent terrain
		Terrain* getParent() const { return mParent; }
		/// Get the index of the layer this is targetting
		uint8 getLayerIndex() const { return mLayerIdx; }

		/** Helper method - convert a point in world space to UV space based on the
			terrain settings.
		@param worldPos World position
		@param outX, outY Pointers to variables which will be filled in with the
			local UV space value. Note they are deliberately signed Real values, because the
			point you supply may be outside of image space and may be between texels.
			The values will range from 0 to 1, top/bottom, left/right.
		*/
		void convertWorldToUVSpace(const Vector3& worldPos, Real *outX, Real* outY);

		/** Helper method - convert a point in local space to worldspace based on the
			terrain settings.
		@param x,y Local position, ranging from 0 to 1, top/bottom, left/right.
		@param outWorldPos Pointer will be filled in with the world space value
		*/
		void convertUVToWorldSpace(Real x, Real y, Vector3* outWorldPos);

		/** Convert local space values (0,1) to image space (0, imageSize).
		*/
		void convertUVToImageSpace(Real x, Real y, size_t* outX, size_t* outY);
		/** Convert image space (0, imageSize) to local space values (0,1).
		*/
		void convertImageToUVSpace(size_t x, size_t y, Real* outX, Real* outY);
		/** Convert image space (0, imageSize) to terrain space values (0,1).
		*/
		void convertImageToTerrainSpace(size_t x, size_t y, Real* outX, Real* outY);
		/** Convert terrain space values (0,1) to image space (0, imageSize).
		*/
		void convertTerrainToImageSpace(Real x, Real y, size_t* outX, size_t* outY);

		/** Get a single value of blend information, in image space.
		@param x,y Coordinates of the point of data to get, in image space (top down)
		@returns The blend data
		*/
		uint8 getBlendValue(size_t x, size_t y);

		/** Set a single value of blend information (0 = transparent, 255 = solid)
		@param x,y Coordinates of the point of data to get, in image space (top down)
		@param val The blend value to set
		*/
		void setBlendValue(size_t x, size_t y, uint8 val);

		/** Get a pointer to the whole blend data. 
		@remarks
			This method allows you to get a raw pointer to all the blend data, to 
			update it as you like. However, you must then call dirtyRect manually 
			if you want those changes to be recognised. 
		*/
		uint8* getBlendPointer();

		/** Indicate that all of the blend data is dirty and needs updating.
		*/
		void dirty();

		/** Indicate that a portion of the blend data is dirty and needs updating.
		@param rect Rectangle in image space
		*/
		void dirtyRect(const Rect& rect);

		/** Blits a set of values into a region on the blend map. 
		   	@param src		PixelBox containing the source pixels and format 
		   	@param dstBox	Image::Box describing the destination region in this map
            @remarks The source and destination regions dimensions don't have to match, in which
            case scaling is done. 
			@note You can call this method in a background thread if you want.
				You still need to call update() to commit the changes to the GPU. 
		*/
		void blit(const PixelBox &src, const Box &dstBox);
		
		/** Blits a set of values into the entire map. The source data is scaled if needed.
			@param src		PixelBox containing the source pixels and format
			@note You can call this method in a background thread if you want.
				You still need to call update() to commit the changes to the GPU. 
				
		*/
		void blit(const PixelBox &src);

		/** Load an image into this blend layer. 
		*/
		void loadImage(const Image& img);

		/** Load an image into this blend layer. 
		@param stream Stream containing the image data
		@param ext Extension identifying the image type, if the stream data doesn't identify
		*/
		void loadImage(DataStreamPtr& stream, const String& ext = StringUtil::BLANK);

		/** Load an image into this blend layer. 
		*/
		void loadImage(const String& filename, const String& groupName);

		/** Publish any changes you made to the blend data back to the blend map. 
		@note
			Can only be called in the main render thread.
		*/
		void update();


	};

	typedef vector<TerrainLayerBlendMap*>::type TerrainLayerBlendMapList;

	/** @} */
	/** @} */

}



#endif
