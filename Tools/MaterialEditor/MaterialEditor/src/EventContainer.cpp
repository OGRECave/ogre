/*
-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------
*/
#include "EventContainer.h"

#include "EventArgs.h"

EventContainer::EventContainer()
{
}

EventContainer::~EventContainer()
{
}

void EventContainer::subscribe(int eventId, EventHandler handler)
{
	DelegateMap::iterator it = mDelegates.find(eventId);
	it->second->connect(handler);
}

// TODO: Implement unsubscribe
void EventContainer::unsubscribe(int eventId,EventHandler handler)
{
	DelegateMap::iterator it = mDelegates.find(eventId);
	//it->second->disconnect(handler);
}

// Should we throw an exception on duplicate event IDs?
void EventContainer::registerEvent(int eventId)
{
	mDelegates[eventId] = new Delegate();
}

void EventContainer::fireEvent(int eventId, EventArgs& args)
{
	DelegateMap::iterator it = mDelegates.find(eventId);
	(*(it->second))(args);
}