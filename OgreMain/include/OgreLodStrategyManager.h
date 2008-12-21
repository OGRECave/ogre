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
#ifndef __Lod_Strategy_Manager_H__
#define __Lod_Strategy_Manager_H__

#include "OgrePrerequisites.h"

#include "OgreLodStrategy.h"
#include "OgreSingleton.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

    /** Manager for lod strategies. */
    class _OgreExport LodStrategyManager : public Singleton<LodStrategyManager>, public LodAlloc
    {
        /** Map of strategies. */
        typedef std::map<String, LodStrategy *> StrategyMap;

        /** Internal map of strategies. */
        StrategyMap mStrategies;

        /** Default strategy. */
        LodStrategy *mDefaultStrategy;

    public:
        /** Default constructor. */
        LodStrategyManager();

        /** Destructor. */
        ~LodStrategyManager();

        /** Add a strategy to the manager. */
        void addStrategy(LodStrategy *strategy);

        /** Remove a strategy from the manager with a specified name.
        @remarks
            The removed strategy is returned so the user can control
            how it is destroyed.
        */
        LodStrategy *removeStrategy(const String& name);

        /** Remove and delete all strategies from the manager.
        @remarks
            All strategies are deleted.  If finer control is required
            over strategy destruction, use removeStrategy.
        */
        void removeAllStrategies();

        /** Get the strategy with the specified name. */
        LodStrategy *getStrategy(const String& name);

        /** Set the default strategy. */
        void setDefaultStrategy(LodStrategy *strategy);

        /** Set the default strategy by name. */
        void setDefaultStrategy(const String& name);

        /** Get the current default strategy. */
        LodStrategy *getDefaultStrategy();

        /** Get an iterator for all contained strategies. */
        MapIterator<StrategyMap> getIterator();

        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static LodStrategyManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static LodStrategyManager* getSingletonPtr(void);
    };
}

#endif
