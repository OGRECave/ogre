/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

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

#ifndef __Ogre_TerrainAutoUpdateLod_H__
#define __Ogre_TerrainAutoUpdateLod_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreTerrain.h"

namespace Ogre
{
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Terrain
	*  Some details on the terrain auto load
	*  @{
	*/

	/** Terrain automatic LOD loading
	@par
		This set of classes is used for automatic change of terrain LOD level. Base is TerrainAutoUpdateLod interface with just
		one public method autoUpdateLod. This method gets called by terrain whenever user thinks something has
		changed(typically in application's main loop) what could affect terrain's LOD level. It is designed in such a way
		so user can use whatever algorithm he likes to change terrin's LOD level. For example see TerrainAutoUpdateLod
		implemetation TerrainAutoUpdateLodByDistance.
	*/

	class _OgreTerrainExport TerrainAutoUpdateLod : public Singleton<TerrainAutoUpdateLod>
	{
	public:
		/** Method to be called to change terrain's LOD level.
			@param terrain Instance of Terrain which LOD level is going to be changed
			@param synchronous Run this as part of main thread or in background
			@param data Any user specific data.
		*/
		virtual void autoUpdateLod(Terrain *terrain, bool synchronous, const Any &data) = 0;
	};

	/** Class implementing TerrainAutoUpdateLod interface. It does LOD level increase/decrease according to camera's
		distance to Terrain.
	*/
	class _OgreTerrainExport TerrainAutoUpdateLodByDistance : public TerrainAutoUpdateLod
	{
	public:
		void autoUpdateLod(Terrain *terrain, bool synchronous, const Any &data);

	protected:
		/** Modifies Terrain's LOD level according to it's distance from camera.
			@param holdDistance How far ahead of terrain's LOD level change this LOD level should be loaded.
		*/
		void autoUpdateLodByDistance(Terrain *terrain, bool synchronous, const Real holdDistance);
		/// Traverse Terrain's QuadTree and calculate what LOD level is needed.
		int traverseTreeByDistance(TerrainQuadTreeNode *node, const Camera *cam, Real cFactor, const Real holdDistance);
	private:
		// the only instance
		static TerrainAutoUpdateLodByDistance me;
	};

	// other Strategy's id start from 2
	enum DefaultTerrainAutoUpdateLodStrategy
	{
		NONE = 0,
		BY_DISTANCE = 1,
		DEFAULT = 1
	};
	class _OgreTerrainExport TerrainAutoUpdateLodFactory
	{
	public:
		static void registerAutoUpdateLodStrategy( uint32 strategy, TerrainAutoUpdateLod* updater )
		{
			assert(strategy>1 && "User defined strategy must start from 2");
			mAutoUpdateLodStrategyMap[strategy] = updater;
		}
		static TerrainAutoUpdateLod* getAutoUpdateLod( uint32 strategy )
		{
			switch(strategy)
			{
			case NONE: return 0;
			case BY_DISTANCE: return TerrainAutoUpdateLodByDistance::getSingletonPtr();
			default:
				std::map<int,TerrainAutoUpdateLod*>::iterator iter = mAutoUpdateLodStrategyMap.find(strategy);
				assert(iter!=mAutoUpdateLodStrategyMap.end() && "Unregistered AutoUpdateLodStrategy");
				return iter->second;
			}
			return 0;
		}
	private:
		static std::map<int,TerrainAutoUpdateLod*> mAutoUpdateLodStrategyMap;
	};
	/** @} */
	/** @} */
}

#endif 
