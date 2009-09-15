/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

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
#include "OgreLodStrategyManager.h"

#include "OgreException.h"
#include "OgreDistanceLodStrategy.h"
#include "OgrePixelCountLodStrategy.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> LodStrategyManager* Singleton<LodStrategyManager>::ms_Singleton = 0;
    LodStrategyManager* LodStrategyManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    LodStrategyManager& LodStrategyManager::getSingleton(void)
    {
        assert( ms_Singleton );  return ( *ms_Singleton );
    }
    //-----------------------------------------------------------------------
    LodStrategyManager::LodStrategyManager()
    {
        // Add default (distance) strategy
        LodStrategy *distanceStrategy = OGRE_NEW DistanceLodStrategy();
        addStrategy(distanceStrategy);

        // Add new pixel-count strategy
        LodStrategy *pixelCountStrategy = OGRE_NEW PixelCountLodStrategy();
        addStrategy(pixelCountStrategy);

        // Set the default strategy
        setDefaultStrategy(distanceStrategy);
    }
    //-----------------------------------------------------------------------
    LodStrategyManager::~LodStrategyManager()
    {
        // Destroy all strategies and clear the map
        removeAllStrategies();
    }
    //-----------------------------------------------------------------------
    void LodStrategyManager::addStrategy(LodStrategy *strategy)
    {
        // Check for invalid strategy name
        if (strategy->getName() == "default")
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Lod strategy name must not be \"default\".", "LodStrategyManager::addStrategy");

        // Insert the strategy into the map with its name as the key
        mStrategies.insert(std::make_pair(strategy->getName(), strategy));
    }
    //-----------------------------------------------------------------------
    LodStrategy *LodStrategyManager::removeStrategy(const String& name)
    {
        // Find strategy with specified name
        StrategyMap::iterator it = mStrategies.find(name);

        // If not found, return null
        if (it == mStrategies.end())
            return 0;

        // Otherwise, erase the strategy from the map
        mStrategies.erase(it);

        // Return the strategy that was removed
        return it->second;
    }
    //-----------------------------------------------------------------------
    void LodStrategyManager::removeAllStrategies()
    {
        // Get beginning iterator
        for (StrategyMap::iterator it = mStrategies.begin(); it != mStrategies.end(); ++it)
		{
			OGRE_DELETE it->second;
		}
		mStrategies.clear();

    }
    //-----------------------------------------------------------------------
    LodStrategy *LodStrategyManager::getStrategy(const String& name)
    {
        // If name is "default", return the default strategy instead of performing a lookup
        if (name == "default")
            return getDefaultStrategy();

        // Find strategy with specified name
        StrategyMap::iterator it = mStrategies.find(name);

        // If not found, return null
        if (it == mStrategies.end())
            return 0;

        // Otherwise, return the strategy
        return it->second;
    }
    //-----------------------------------------------------------------------
    void LodStrategyManager::setDefaultStrategy(LodStrategy *strategy)
    {
        mDefaultStrategy = strategy;
    }
    //-----------------------------------------------------------------------
    void LodStrategyManager::setDefaultStrategy(const String& name)
    {
        // Lookup by name and set default strategy
        setDefaultStrategy(getStrategy(name));
    }
    //-----------------------------------------------------------------------
    LodStrategy *LodStrategyManager::getDefaultStrategy()
    {
        return mDefaultStrategy;
    }
    //-----------------------------------------------------------------------
    MapIterator<LodStrategyManager::StrategyMap> LodStrategyManager::getIterator()
    {
        // Construct map iterator from strategy map and return
        return MapIterator<StrategyMap>(mStrategies);
    }
    //-----------------------------------------------------------------------

}
