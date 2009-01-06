/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
LGPL like the rest of the engine.
-----------------------------------------------------------------------------
*/

#ifndef _ItemSelectorViewManager_H_
#define _ItemSelectorViewManager_H_

#include "OgreString.h"

#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUILogger.h>
#include <CEGUI/CEGUISchemeManager.h>
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/CEGUIWindow.h>
#include <CEGUI/elements/CEGUICheckbox.h>
#include <CEGUI/elements/CEGUICombobox.h>
#include <CEGUI/elements/CEGUIListbox.h>
#include <CEGUI/elements/CEGUIListboxTextItem.h>
#include <CEGUI/elements/CEGUIPushButton.h>
#include <CEGUI/elements/CEGUIScrollbar.h>
#include <CEGUI/elements/CEGUIScrollablePane.h>

    /** Provides interface between a View and a Controller.
    */
    class ItemSelectorInterface
    {
    public:
        virtual ~ItemSelectorInterface(void) {}
        virtual void itemStateChanged(const size_t index, const bool state) = 0;
    };

    /** A generic view for showing checkbox items in a vertical list.  Checkboxes
        are in a Scrollable pane so items can be scrolled into view.
    */
    class ItemSelectorViewManager
    {
    public:
        ItemSelectorViewManager(const Ogre::String& parentWindowName);
        ~ItemSelectorViewManager(void){}

        /** Returns number of items in the container.
        */
        size_t getSelectorCount(void) const { return mItemSelectorContainer.size(); }
        /** iterates through compositors available through CompositorManager and update list
        */
        /** Add a new Item selector to the container
        */
        void addItemSelector(const Ogre::String& displayText);
        Ogre::String getItemSelectorText(const size_t index) const
        {
            return Ogre::String(mItemSelectorContainer.at(index).CheckBoxWidget->getText().c_str());
        }
        /** Inform Manager about who will be the controller that responds to item event messages.
        */
        void setItemSelectorController(ItemSelectorInterface* controller);

    protected:
        struct ItemSelector
        {
            // make use of widget text member to store Compositor name
            // widget toggle state is used for enable/disable compositor
            // widget ID used for selector index
            CEGUI::Checkbox* CheckBoxWidget;

            ItemSelector() : CheckBoxWidget(0) {}
        };
		typedef Ogre::vector<ItemSelector>::type ItemSelectorContainer;
        typedef ItemSelectorContainer::iterator ItemSelectorIterator;

        float mVerticalScrollPosition;
        CEGUI::Window* mParentWindow;
        CEGUI::ScrollablePane* mScrollablePane;
        ItemSelectorInterface* mItemSelectorController;

        ItemSelectorContainer mItemSelectorContainer;

    private:
        bool handleCheckStateChanged(const CEGUI::EventArgs& e);

    };


#endif
