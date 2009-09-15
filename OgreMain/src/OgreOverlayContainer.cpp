/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgreOverlayContainer.h"
#include "OgreException.h"
#include "OgreOverlayManager.h"

namespace Ogre {

    //---------------------------------------------------------------------
    OverlayContainer::OverlayContainer(const String& name)
        : OverlayElement(name),
		mChildrenProcessEvents(true)
    {
    }
    //---------------------------------------------------------------------
    OverlayContainer::~OverlayContainer()
    {
		// remove from parent overlay if root
		if (mOverlay && !mParent)
		{
			mOverlay->remove2D(this);
		}

        OverlayContainer::ChildIterator ci = getChildIterator();
        while (ci.hasMoreElements())
        {
            OverlayElement* child = ci.getNext();
			child->_notifyParent(0, 0);
        }
    }
    //---------------------------------------------------------------------
    void OverlayContainer::addChild(OverlayElement* elem)
    {
        if (elem->isContainer())
		{
			addChildImpl(static_cast<OverlayContainer*>(elem));
		}
		else
		{
			addChildImpl(elem);
		}

	}
    //---------------------------------------------------------------------
    void OverlayContainer::addChildImpl(OverlayElement* elem)
    {
        String name = elem->getName();
        ChildMap::iterator i = mChildren.find(name);
        if (i != mChildren.end())
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, "Child with name " + name + 
                " already defined.", "OverlayContainer::addChild");
        }

        mChildren.insert(ChildMap::value_type(name, elem));
        // tell child about parent & ZOrder
        elem->_notifyParent(this, mOverlay);
	    elem->_notifyZOrder(mZOrder + 1);
	    elem->_notifyWorldTransforms(mXForm);
	    elem->_notifyViewport();

    }
    //---------------------------------------------------------------------
    void OverlayContainer::addChildImpl(OverlayContainer* cont)
    {
        // Add to main map first 
        // This will pick up duplicates
        OverlayElement* pElem = cont;
        addChildImpl(pElem);

        /*
        cont->_notifyParent(this, mOverlay);
        cont->_notifyZOrder(mZOrder + 1);
	    cont->_notifyWorldTransforms(mXForm);

		// tell children of new container the current overlay
        ChildIterator it = cont->getChildIterator();
        while (it.hasMoreElements())
        {
            // Give children ZOrder 1 higher than this
            GuiElement* pElemChild = it.getNext();
			pElemChild->_notifyParent(cont, mOverlay);
            pElemChild->_notifyZOrder(cont->getZOrder() + 1);
    	    pElemChild->_notifyWorldTransforms(mXForm);
        }
        */

        // Now add to specific map too
        mChildContainers.insert(ChildContainerMap::value_type(cont->getName(), cont));

    }
    //---------------------------------------------------------------------
    void OverlayContainer::removeChild(const String& name)
    {
        ChildMap::iterator i = mChildren.find(name);
        if (i == mChildren.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Child with name " + name + 
                " not found.", "OverlayContainer::removeChild");
        }

        OverlayElement* element = i->second;
        mChildren.erase(i);

            // remove from container list (if found)
        ChildContainerMap::iterator j = mChildContainers.find(name);
        if (j != mChildContainers.end())
            mChildContainers.erase(j);

        element->_setParent(0);
    }
    //---------------------------------------------------------------------
    void OverlayContainer::_addChild(OverlayElement* elem)
    {
        if (elem->isContainer())
		{
			addChildImpl(static_cast<OverlayContainer*>(elem));
		}
		else
		{
			addChildImpl(elem);
		}
	}
    //---------------------------------------------------------------------
    void OverlayContainer::_removeChild(const String& name)
    {
        ChildMap::iterator i = mChildren.find(name);
        if (i == mChildren.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Child with name " + name + 
                " not found.", "OverlayContainer::removeChild");
        }

        OverlayElement* element = i->second;
        mChildren.erase(i);

            // remove from container list (if found)
        ChildContainerMap::iterator j = mChildContainers.find(name);
        if (j != mChildContainers.end())
            mChildContainers.erase(j);

        element->_setParent(0);
    }
    //---------------------------------------------------------------------
    OverlayElement* OverlayContainer::getChild(const String& name)
    {
        ChildMap::iterator i = mChildren.find(name);
        if (i == mChildren.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Child with name " + name + 
                " not found.", "OverlayContainer::getChild");
        }

        return i->second;
    }
    //---------------------------------------------------------------------
    OverlayContainer::ChildIterator OverlayContainer::getChildIterator(void)
    {
        return ChildIterator(mChildren.begin(), mChildren.end());
    }
    //---------------------------------------------------------------------
    OverlayContainer::ChildContainerIterator OverlayContainer::getChildContainerIterator(void)
    {
        return ChildContainerIterator(mChildContainers.begin(), mChildContainers.end());
    }
	//---------------------------------------------------------------------
	void OverlayContainer::initialise(void)
	{
		ChildContainerMap::iterator coni;
		for (coni =  mChildContainers.begin(); coni != mChildContainers.end(); ++coni)
		{
			coni->second->initialise();
		}
		ChildMap::iterator ci;
		for (ci =  mChildren.begin(); ci != mChildren.end(); ++ci)
		{
			ci->second->initialise();
		}
	}
    //---------------------------------------------------------------------
	void OverlayContainer::_positionsOutOfDate(void)
	{
		OverlayElement::_positionsOutOfDate();

        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
			it.getNext()->_positionsOutOfDate();
        }
	}

    //---------------------------------------------------------------------
    void OverlayContainer::_update(void)
    {
        // call superclass
        OverlayElement::_update();

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            it.getNext()->_update();
        }


    }
    //---------------------------------------------------------------------
    ushort OverlayContainer::_notifyZOrder(ushort newZOrder)
    {
        OverlayElement::_notifyZOrder(newZOrder);
		// One for us
		newZOrder++; 

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            // Children "consume" ZOrder values, so keep track of them
            newZOrder = it.getNext()->_notifyZOrder(newZOrder);
        }

		return newZOrder;
    }
    //---------------------------------------------------------------------
    void OverlayContainer::_notifyWorldTransforms(const Matrix4& xform)
    {
        OverlayElement::_notifyWorldTransforms(xform);

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            it.getNext()->_notifyWorldTransforms(xform);
        }
    }
    //---------------------------------------------------------------------
    void OverlayContainer::_notifyViewport()
    {
        OverlayElement::_notifyViewport();

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            it.getNext()->_notifyViewport();
        }
    }
    //---------------------------------------------------------------------
    void OverlayContainer::_notifyParent(OverlayContainer* parent, Overlay* overlay)
    {
        OverlayElement::_notifyParent(parent, overlay);

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            // Notify the children of the overlay 
            it.getNext()->_notifyParent(this, overlay);
        }
    }

    //---------------------------------------------------------------------
    void OverlayContainer::_updateRenderQueue(RenderQueue* queue)
    {
        if (mVisible)
        {

            OverlayElement::_updateRenderQueue(queue);

            // Also add children
            ChildIterator it = getChildIterator();
            while (it.hasMoreElements())
            {
                // Give children ZOrder 1 higher than this
                it.getNext()->_updateRenderQueue(queue);
            }
        }

    }


	OverlayElement* OverlayContainer::findElementAt(Real x, Real y) 		// relative to parent
	{

		OverlayElement* ret = NULL;

		int currZ = -1;

		if (mVisible)
		{
			ret = OverlayElement::findElementAt(x,y);	//default to the current container if no others are found
			if (ret && mChildrenProcessEvents)
			{
				ChildIterator it = getChildIterator();
				while (it.hasMoreElements())
				{
					OverlayElement* currentOverlayElement = it.getNext();
					if (currentOverlayElement->isVisible() && currentOverlayElement->isEnabled())
					{
						int z = currentOverlayElement->getZOrder();
						if (z > currZ)
						{
							OverlayElement* elementFound = currentOverlayElement->findElementAt(x ,y );
							if (elementFound)
							{
								currZ = z;
								ret = elementFound;
							}
						}
					}
				}
			}
		}
		return ret;
	}

    void OverlayContainer::copyFromTemplate(OverlayElement* templateOverlay)
    {
        OverlayElement::copyFromTemplate(templateOverlay);

		    if (templateOverlay->isContainer() && isContainer())
		    {
    	     OverlayContainer::ChildIterator it = static_cast<OverlayContainer*>(templateOverlay)->getChildIterator();
			 while (it.hasMoreElements())
			 {
				 OverlayElement* oldChildElement = it.getNext();
				 if (oldChildElement->isCloneable())
				 {
					 OverlayElement* newChildElement = 
						 OverlayManager::getSingleton().createOverlayElement(
							oldChildElement->getTypeName(), 
							mName+"/"+oldChildElement->getName());
					 newChildElement->copyFromTemplate(oldChildElement);
					 addChild((OverlayContainer*)newChildElement);
				 }
			 }
        }
    }

    OverlayElement* OverlayContainer::clone(const String& instanceName)
    {
        OverlayContainer *newContainer;

        newContainer = static_cast<OverlayContainer*>(OverlayElement::clone(instanceName));

    	  ChildIterator it = getChildIterator();
  		  while (it.hasMoreElements())
			  {
				    OverlayElement* oldChildElement = it.getNext();
				    if (oldChildElement->isCloneable())
				    {
                OverlayElement* newChildElement = oldChildElement->clone(instanceName);
                newContainer->_addChild(newChildElement);
            }
        }

        return newContainer;
    }

}

