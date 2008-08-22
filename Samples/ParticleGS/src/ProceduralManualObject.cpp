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

#include "ProceduralManualObject.h"

namespace Ogre
{
	const String& ProceduralManualObject::getMovableType(void) const
	{
		return ProceduralManualObjectFactory::FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------------
	void ProceduralManualObject::_updateRenderQueue(RenderQueue* queue)
	{
		mR2vbObject->update(m_pParentSceneManager);
		queue->addRenderable(this);
	}
	//-----------------------------------------------------------------------------
	void ProceduralManualObject::getRenderOperation(RenderOperation& op)
	{
		mR2vbObject->getRenderOperation(op);
	}
	//-----------------------------------------------------------------------------
	void ProceduralManualObject::setManualObject(Ogre::ManualObject *manualObject)
	{
		mManualObject = manualObject;
		m_pParentSceneManager = manualObject->_getManager();
		if (!mR2vbObject.isNull())
		{
			mR2vbObject->setSourceRenderable(manualObject->getSection(0));
		}
	}
	//-----------------------------------------------------------------------------
	String ProceduralManualObjectFactory::FACTORY_TYPE_NAME = "ProceduralManualObject";
	//-----------------------------------------------------------------------------
	const String& ProceduralManualObjectFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------------
	MovableObject* ProceduralManualObjectFactory::createInstanceImpl(
		const String& name, const NameValuePairList* params)
	{
		return new ProceduralManualObject();
	}
	//-----------------------------------------------------------------------------
	void ProceduralManualObjectFactory::destroyInstance( MovableObject* obj)
	{
		delete obj;
	}
}