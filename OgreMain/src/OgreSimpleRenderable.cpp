/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#include "OgreSimpleRenderable.h"
#include "OgreException.h"
#include "OgreSceneNode.h"

#include "OgreMaterialManager.h"

namespace Ogre {

    uint SimpleRenderable::ms_uGenNameCount = 0;

    SimpleRenderable::SimpleRenderable()
    {
        m_matWorldTransform = Matrix4::IDENTITY;

        m_strMatName = "BaseWhite"; 
        m_pMaterial = MaterialManager::getSingleton().getByName("BaseWhite");

        m_pParentSceneManager = NULL;

        mParentNode = NULL;
        m_pCamera = NULL;

        // Generate name
		StringUtil::StrStreamType name;
		name << "SimpleRenderable" << ms_uGenNameCount++;
		mName = name.str();
    }

    void SimpleRenderable::setMaterial( const String& matName )
    {
        m_strMatName = matName;
        m_pMaterial = MaterialManager::getSingleton().getByName(m_strMatName);
		if (m_pMaterial.isNull())
			OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Could not find material " + m_strMatName,
				"SimpleRenderable::setMaterial" );
    
        // Won't load twice anyway
        m_pMaterial->load();
    }

    const MaterialPtr& SimpleRenderable::getMaterial(void) const
    {
        return m_pMaterial;
    }

    void SimpleRenderable::getRenderOperation(RenderOperation& op)
    {
        op = mRenderOp;
    }

    void SimpleRenderable::setRenderOperation( const RenderOperation& rend )
    {
        mRenderOp = rend;
    }

    void SimpleRenderable::setWorldTransform( const Matrix4& xform )
    {
        m_matWorldTransform = xform;
    }

    void SimpleRenderable::getWorldTransforms( Matrix4* xform ) const
    {
        *xform = m_matWorldTransform * mParentNode->_getFullTransform();
    }

    void SimpleRenderable::_notifyCurrentCamera(Camera* cam)
    {
		MovableObject::_notifyCurrentCamera(cam);

        m_pCamera = cam;
    }

    void SimpleRenderable::setBoundingBox( const AxisAlignedBox& box )
    {
        mBox = box;
    }

    const AxisAlignedBox& SimpleRenderable::getBoundingBox(void) const
    {
        return mBox;
    }

    void SimpleRenderable::_updateRenderQueue(RenderQueue* queue)
    {
        queue->addRenderable( this, mRenderQueueID, OGRE_RENDERABLE_DEFAULT_PRIORITY); 
    }

	void SimpleRenderable::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		visitor->visit(this, 0, false);
	}

    SimpleRenderable::~SimpleRenderable()
    {
    }
    //-----------------------------------------------------------------------
    const String& SimpleRenderable::getMovableType(void) const
    {
        static String movType = "SimpleRenderable";
        return movType;
    }
    //-----------------------------------------------------------------------
    const LightList& SimpleRenderable::getLights(void) const
    {
        // Use movable query lights
        return queryLights();
    }

}
