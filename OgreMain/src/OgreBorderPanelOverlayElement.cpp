/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright  2000-2005 The OGRE Team

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

#include "OgreBorderPanelOverlayElement.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreStringConverter.h"
#include "OgreOverlayManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreException.h"
#include "OgreRenderQueue.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
    //---------------------------------------------------------------------
    String BorderPanelOverlayElement::msTypeName = "BorderPanel";
    BorderPanelOverlayElement::CmdBorderSize BorderPanelOverlayElement::msCmdBorderSize;
    BorderPanelOverlayElement::CmdBorderMaterial BorderPanelOverlayElement::msCmdBorderMaterial;
    BorderPanelOverlayElement::CmdBorderLeftUV BorderPanelOverlayElement::msCmdBorderLeftUV;
    BorderPanelOverlayElement::CmdBorderTopUV BorderPanelOverlayElement::msCmdBorderTopUV;
    BorderPanelOverlayElement::CmdBorderBottomUV BorderPanelOverlayElement::msCmdBorderBottomUV;
    BorderPanelOverlayElement::CmdBorderRightUV BorderPanelOverlayElement::msCmdBorderRightUV;
    BorderPanelOverlayElement::CmdBorderTopLeftUV BorderPanelOverlayElement::msCmdBorderTopLeftUV;
    BorderPanelOverlayElement::CmdBorderBottomLeftUV BorderPanelOverlayElement::msCmdBorderBottomLeftUV;
    BorderPanelOverlayElement::CmdBorderTopRightUV BorderPanelOverlayElement::msCmdBorderTopRightUV;
    BorderPanelOverlayElement::CmdBorderBottomRightUV BorderPanelOverlayElement::msCmdBorderBottomRightUV;

    #define BCELL_UV(x) (x * 4 * 2)
    #define POSITION_BINDING 0
    #define TEXCOORD_BINDING 1
    //---------------------------------------------------------------------
    BorderPanelOverlayElement::BorderPanelOverlayElement(const String& name)
      : PanelOverlayElement(name), 
        mLeftBorderSize(0),
        mRightBorderSize(0),
        mTopBorderSize(0),
        mBottomBorderSize(0),
        mPixelLeftBorderSize(0),
        mPixelRightBorderSize(0),
        mPixelTopBorderSize(0),
        mPixelBottomBorderSize(0),
        mpBorderMaterial(0),
        mBorderRenderable(0)
    {
        if (createParamDictionary("BorderPanelOverlayElement"))
        {
            addBaseParameters();
        }
    }
    //---------------------------------------------------------------------
    BorderPanelOverlayElement::~BorderPanelOverlayElement()
    {
        OGRE_DELETE mRenderOp2.vertexData;
        OGRE_DELETE mRenderOp2.indexData;
        OGRE_DELETE mBorderRenderable;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::initialise(void)
    {
		bool init = !mInitialised;

        PanelOverlayElement::initialise();

        // superclass will handle the interior panel area 

        if (init)
		{
			// Setup render op in advance
			mRenderOp2.vertexData = OGRE_NEW VertexData();
			mRenderOp2.vertexData->vertexCount = 4 * 8; // 8 cells, can't necessarily share vertices cos
														// texcoords may differ
			mRenderOp2.vertexData->vertexStart = 0;

			// Vertex declaration
			VertexDeclaration* decl = mRenderOp2.vertexData->vertexDeclaration;
			// Position and texture coords each have their own buffers to allow
			// each to be edited separately with the discard flag
			decl->addElement(POSITION_BINDING, 0, VET_FLOAT3, VES_POSITION);
			decl->addElement(TEXCOORD_BINDING, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);

			// Vertex buffer #1, position
			HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton()
				.createVertexBuffer(
					decl->getVertexSize(POSITION_BINDING), 
					mRenderOp2.vertexData->vertexCount,
					HardwareBuffer::HBU_STATIC_WRITE_ONLY);
			// bind position
			VertexBufferBinding* binding = mRenderOp2.vertexData->vertexBufferBinding;
			binding->setBinding(POSITION_BINDING, vbuf);

			// Vertex buffer #2, texcoords
			vbuf = HardwareBufferManager::getSingleton()
				.createVertexBuffer(
					decl->getVertexSize(TEXCOORD_BINDING), 
					mRenderOp2.vertexData->vertexCount,
					HardwareBuffer::HBU_STATIC_WRITE_ONLY, true);
			// bind texcoord
			binding->setBinding(TEXCOORD_BINDING, vbuf);

			mRenderOp2.operationType = RenderOperation::OT_TRIANGLE_LIST;
			mRenderOp2.useIndexes = true;
			// Index data
			mRenderOp2.indexData = OGRE_NEW IndexData();
			mRenderOp2.indexData->indexCount = 8 * 6;
			mRenderOp2.indexData->indexStart = 0;

			/* Each cell is
				0-----2
				|    /|
				|  /  |
				|/    |
				1-----3
			*/
			mRenderOp2.indexData->indexBuffer = HardwareBufferManager::getSingleton().
				createIndexBuffer(
					HardwareIndexBuffer::IT_16BIT, 
					mRenderOp2.indexData->indexCount, 
					HardwareBuffer::HBU_STATIC_WRITE_ONLY);

			ushort* pIdx = static_cast<ushort*>(
				mRenderOp2.indexData->indexBuffer->lock(
					0, 
					mRenderOp2.indexData->indexBuffer->getSizeInBytes(), 
					HardwareBuffer::HBL_DISCARD) );

			for (ushort cell = 0; cell < 8; ++cell)
			{
				ushort base = cell * 4;
				*pIdx++ = base;
				*pIdx++ = base + 1;
				*pIdx++ = base + 2;

				*pIdx++ = base + 2;
				*pIdx++ = base + 1;
				*pIdx++ = base + 3;
			}

			mRenderOp2.indexData->indexBuffer->unlock();

			// Create sub-object for rendering border
			mBorderRenderable = OGRE_NEW BorderRenderable(this);

			mInitialised = true;
		}
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::addBaseParameters(void)
    {
        PanelOverlayElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("border_size", 
            "The sizes of the borders relative to the screen size, in the order "
            "left, right, top, bottom."
            , PT_STRING),
            &msCmdBorderSize);
        dict->addParameter(ParameterDef("border_material", 
            "The material to use for the border."
            , PT_STRING),
            &msCmdBorderMaterial);
        dict->addParameter(ParameterDef("border_topleft_uv", 
            "The texture coordinates for the top-left corner border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderTopLeftUV);
        dict->addParameter(ParameterDef("border_topright_uv", 
            "The texture coordinates for the top-right corner border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderTopRightUV);
        dict->addParameter(ParameterDef("border_bottomright_uv", 
            "The texture coordinates for the bottom-right corner border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderBottomRightUV);
        dict->addParameter(ParameterDef("border_bottomleft_uv", 
            "The texture coordinates for the bottom-left corner border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderBottomLeftUV);
        dict->addParameter(ParameterDef("border_left_uv", 
            "The texture coordinates for the left edge border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderLeftUV);
        dict->addParameter(ParameterDef("border_top_uv", 
            "The texture coordinates for the top edge border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderTopUV);
        dict->addParameter(ParameterDef("border_right_uv", 
            "The texture coordinates for the right edge border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderRightUV);
        dict->addParameter(ParameterDef("border_bottom_uv", 
            "The texture coordinates for the bottom edge border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderBottomUV);

    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setBorderSize(Real size)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelLeftBorderSize = mPixelRightBorderSize = 
                mPixelTopBorderSize = mPixelBottomBorderSize = static_cast<unsigned short>(size);
        }
        else
        {
            mLeftBorderSize = mRightBorderSize = 
                mTopBorderSize = mBottomBorderSize = size;
        }
        mGeomPositionsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setBorderSize(Real sides, Real topAndBottom)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelLeftBorderSize = mPixelRightBorderSize = static_cast<unsigned short>(sides);
            mPixelTopBorderSize = mPixelBottomBorderSize = static_cast<unsigned short>(topAndBottom);
        }
        else
        {
            mLeftBorderSize = mRightBorderSize = sides;
            mTopBorderSize = mBottomBorderSize = topAndBottom;
        }
        mGeomPositionsOutOfDate = true;


    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setBorderSize(Real left, Real right, Real top, Real bottom)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelLeftBorderSize = static_cast<unsigned short>(left);
            mPixelRightBorderSize = static_cast<unsigned short>(right);
            mPixelTopBorderSize = static_cast<unsigned short>(top);
            mPixelBottomBorderSize = static_cast<unsigned short>(bottom);
        }
        else
        {
            mLeftBorderSize = left;
            mRightBorderSize = right;
            mTopBorderSize = top;
            mBottomBorderSize = bottom;
        }
        mGeomPositionsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    Real BorderPanelOverlayElement::getLeftBorderSize(void) const
    {
        if (mMetricsMode == GMM_PIXELS)
        {
			return mPixelLeftBorderSize;
		}
		else
		{
			return mLeftBorderSize;
		}
    }
    //---------------------------------------------------------------------
    Real BorderPanelOverlayElement::getRightBorderSize(void) const
    {
        if (mMetricsMode == GMM_PIXELS)
        {
			return mPixelRightBorderSize;
		}
		else
		{
			return mRightBorderSize;
		}
    }
    //---------------------------------------------------------------------
    Real BorderPanelOverlayElement::getTopBorderSize(void) const
    {
        if (mMetricsMode == GMM_PIXELS)
        {
			return mPixelTopBorderSize;
		}
		else
		{
			return mTopBorderSize;
		}
    }
    //---------------------------------------------------------------------
    Real BorderPanelOverlayElement::getBottomBorderSize(void) const
    {
        if (mMetricsMode == GMM_PIXELS)
        {
			return mPixelBottomBorderSize;
		}
		else
		{
			return mBottomBorderSize;
		}
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::updateTextureGeometry()
    {
		PanelOverlayElement::updateTextureGeometry();
		/* Each cell is
			0-----2
			|    /|
			|  /  |
			|/    |
			1-----3
		*/
		// No choice but to lock / unlock each time here, but lock only small sections
	    
		HardwareVertexBufferSharedPtr vbuf = 
			mRenderOp2.vertexData->vertexBufferBinding->getBuffer(TEXCOORD_BINDING);
		// Can't use discard since this discards whole buffer
		float* pUV = static_cast<float*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));
		
		for (uint i = 0; i < 8; ++i)
		{
			*pUV++ = mBorderUV[i].u1; *pUV++ = mBorderUV[i].v1;
			*pUV++ = mBorderUV[i].u1; *pUV++ = mBorderUV[i].v2;
			*pUV++ = mBorderUV[i].u2; *pUV++ = mBorderUV[i].v1;
			*pUV++ = mBorderUV[i].u2; *pUV++ = mBorderUV[i].v2;
		}

		vbuf->unlock();
    }
    //---------------------------------------------------------------------
    String BorderPanelOverlayElement::getCellUVString(BorderCellIndex idx) const
    {
        String ret = StringConverter::toString(mBorderUV[idx].u1) + " " +
		            StringConverter::toString(mBorderUV[idx].v1) + " " +
		            StringConverter::toString(mBorderUV[idx].u2) + " " +
		            StringConverter::toString(mBorderUV[idx].v2);
        return ret;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setLeftBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
		mBorderUV[BCELL_LEFT].u1 = u1; 
		mBorderUV[BCELL_LEFT].u2 = u2; 
		mBorderUV[BCELL_LEFT].v1 = v1; 
		mBorderUV[BCELL_LEFT].v2 = v2; 
		mGeomUVsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setRightBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
		mBorderUV[BCELL_RIGHT].u1 = u1; 
		mBorderUV[BCELL_RIGHT].u2 = u2; 
		mBorderUV[BCELL_RIGHT].v1 = v1; 
		mBorderUV[BCELL_RIGHT].v2 = v2; 
		mGeomUVsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setTopBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
		mBorderUV[BCELL_TOP].u1 = u1; 
		mBorderUV[BCELL_TOP].u2 = u2; 
		mBorderUV[BCELL_TOP].v1 = v1; 
		mBorderUV[BCELL_TOP].v2 = v2; 
		mGeomUVsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setBottomBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
		mBorderUV[BCELL_BOTTOM].u1 = u1; 
		mBorderUV[BCELL_BOTTOM].u2 = u2; 
		mBorderUV[BCELL_BOTTOM].v1 = v1; 
		mBorderUV[BCELL_BOTTOM].v2 = v2; 
		mGeomUVsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setTopLeftBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
		mBorderUV[BCELL_TOP_LEFT].u1 = u1; 
		mBorderUV[BCELL_TOP_LEFT].u2 = u2; 
		mBorderUV[BCELL_TOP_LEFT].v1 = v1; 
		mBorderUV[BCELL_TOP_LEFT].v2 = v2; 
		mGeomUVsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setTopRightBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
		mBorderUV[BCELL_TOP_RIGHT].u1 = u1; 
		mBorderUV[BCELL_TOP_RIGHT].u2 = u2; 
		mBorderUV[BCELL_TOP_RIGHT].v1 = v1; 
		mBorderUV[BCELL_TOP_RIGHT].v2 = v2; 
		mGeomUVsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setBottomLeftBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
		mBorderUV[BCELL_BOTTOM_LEFT].u1 = u1; 
		mBorderUV[BCELL_BOTTOM_LEFT].u2 = u2; 
		mBorderUV[BCELL_BOTTOM_LEFT].v1 = v1; 
		mBorderUV[BCELL_BOTTOM_LEFT].v2 = v2; 
		mGeomUVsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setBottomRightBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
		mBorderUV[BCELL_BOTTOM_RIGHT].u1 = u1; 
		mBorderUV[BCELL_BOTTOM_RIGHT].u2 = u2; 
		mBorderUV[BCELL_BOTTOM_RIGHT].v1 = v1; 
		mBorderUV[BCELL_BOTTOM_RIGHT].v2 = v2; 
		mGeomUVsOutOfDate = true;
    }

    //---------------------------------------------------------------------
    String BorderPanelOverlayElement::getLeftBorderUVString() const
    {
        return getCellUVString(BCELL_LEFT);
    }
    //---------------------------------------------------------------------
    String BorderPanelOverlayElement::getRightBorderUVString() const
    {
        return getCellUVString(BCELL_RIGHT);
    }
    //---------------------------------------------------------------------
    String BorderPanelOverlayElement::getTopBorderUVString() const
    {
        return getCellUVString(BCELL_TOP);
    }
    //---------------------------------------------------------------------
    String BorderPanelOverlayElement::getBottomBorderUVString() const
    {
        return getCellUVString(BCELL_BOTTOM);
    }
    //---------------------------------------------------------------------
    String BorderPanelOverlayElement::getTopLeftBorderUVString() const
    {
        return getCellUVString(BCELL_TOP_LEFT);
    }
    //---------------------------------------------------------------------
    String BorderPanelOverlayElement::getTopRightBorderUVString() const
    {
        return getCellUVString(BCELL_TOP_RIGHT);
    }
    //---------------------------------------------------------------------
    String BorderPanelOverlayElement::getBottomLeftBorderUVString() const
    {
        return getCellUVString(BCELL_BOTTOM_LEFT);
    }
    //---------------------------------------------------------------------
    String BorderPanelOverlayElement::getBottomRightBorderUVString() const
    {
        return getCellUVString(BCELL_BOTTOM_RIGHT);
    }





    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::setBorderMaterialName(const String& name)
    {
        mBorderMaterialName = name;
        mpBorderMaterial = MaterialManager::getSingleton().getByName(name);
        if (mpBorderMaterial.isNull())
			OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Could not find material " + name,
				"BorderPanelOverlayElement::setBorderMaterialName" );
        mpBorderMaterial->load();
        // Set some prerequisites to be sure
        mpBorderMaterial->setLightingEnabled(false);
        mpBorderMaterial->setDepthCheckEnabled(false);

    }
    //---------------------------------------------------------------------
    const String& BorderPanelOverlayElement::getBorderMaterialName(void) const
    {
        return mBorderMaterialName;
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::updatePositionGeometry(void)
    {
		/*
		Grid is like this:
		+--+---------------+--+
		|0 |       1       |2 |
		+--+---------------+--+
		|  |               |  |
		|  |               |  |
		|3 |    center     |4 |
		|  |               |  |
		+--+---------------+--+
		|5 |       6       |7 |
		+--+---------------+--+
		*/
		// Convert positions into -1, 1 coordinate space (homogenous clip space)
		// Top / bottom also need inverting since y is upside down
		Real left[8], right[8], top[8], bottom[8];
		// Horizontal
		left[0] = left[3] = left[5] = _getDerivedLeft() * 2 - 1;
		left[1] = left[6] = right[0] = right[3] = right[5] = left[0] + (mLeftBorderSize * 2);
		right[2] = right[4] = right[7] = left[0] + (mWidth * 2);
		left[2] = left[4] = left[7] = right[1] = right[6] = right[2] - (mRightBorderSize * 2);
		// Vertical
		top[0] = top[1] = top[2] = -((_getDerivedTop() * 2) - 1);
		top[3] = top[4] = bottom[0] = bottom[1] = bottom[2] = top[0] - (mTopBorderSize * 2);
		bottom[5] = bottom[6] = bottom[7] = top[0] -  (mHeight * 2);
		top[5] = top[6] = top[7] = bottom[3] = bottom[4] = bottom[5] + (mBottomBorderSize * 2);

		// Lock the whole position buffer in discard mode
		HardwareVertexBufferSharedPtr vbuf = 
			mRenderOp2.vertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);
		float* pPos = static_cast<float*>(
			vbuf->lock(HardwareBuffer::HBL_DISCARD) );
		// Use the furthest away depth value, since materials should have depth-check off
		// This initialised the depth buffer for any 3D objects in front
		Real zValue = Root::getSingleton().getRenderSystem()->getMaximumDepthInputValue();
		for (ushort cell = 0; cell < 8; ++cell)
		{
			/*
				0-----2
				|    /|
				|  /  |
				|/    |
				1-----3
			*/
			*pPos++ = left[cell];
			*pPos++ = top[cell];
			*pPos++ = zValue;

			*pPos++ = left[cell];
			*pPos++ = bottom[cell];
			*pPos++ = zValue;

			*pPos++ = right[cell];
			*pPos++ = top[cell];
			*pPos++ = zValue;

			*pPos++ = right[cell];
			*pPos++ = bottom[cell];
			*pPos++ = zValue;

		}
		vbuf->unlock();

		// Also update center geometry
		// NB don't use superclass because we need to make it smaller because of border
		vbuf = mRenderOp.vertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);
		pPos = static_cast<float*>(
			vbuf->lock(HardwareBuffer::HBL_DISCARD) );
		// Use cell 1 and 3 to determine positions
		*pPos++ = left[1];
		*pPos++ = top[3];
		*pPos++ = zValue;

		*pPos++ = left[1];
		*pPos++ = bottom[3];
		*pPos++ = zValue;

		*pPos++ = right[1];
		*pPos++ = top[3];
		*pPos++ = zValue;

		*pPos++ = right[1];
		*pPos++ = bottom[3];
		*pPos++ = zValue;

		vbuf->unlock();
    }
    //---------------------------------------------------------------------
    void BorderPanelOverlayElement::_updateRenderQueue(RenderQueue* queue)
    {
        // Add self twice to the queue
        // Have to do this to allow 2 materials
        if (mVisible)
        {

            // Add outer
            queue->addRenderable(mBorderRenderable, RENDER_QUEUE_OVERLAY, mZOrder);

			// do inner last so the border artifacts don't overwrite the children
            // Add inner
            PanelOverlayElement::_updateRenderQueue(queue);
        }
    }
	//---------------------------------------------------------------------
	void BorderPanelOverlayElement::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		visitor->visit(mBorderRenderable, 0, false);
		PanelOverlayElement::visitRenderables(visitor, debugRenderables);
	}
    //-----------------------------------------------------------------------
    void BorderPanelOverlayElement::setMetricsMode(GuiMetricsMode gmm)
    {
        PanelOverlayElement::setMetricsMode(gmm);
        if (gmm != GMM_RELATIVE)
        {
            mPixelBottomBorderSize = static_cast<unsigned short>(mBottomBorderSize);
            mPixelLeftBorderSize = static_cast<unsigned short>(mLeftBorderSize);
            mPixelRightBorderSize = static_cast<unsigned short>(mRightBorderSize);
            mPixelTopBorderSize = static_cast<unsigned short>(mTopBorderSize);
        }
    }
    //-----------------------------------------------------------------------
    void BorderPanelOverlayElement::_update(void)
    {
        if (mMetricsMode != GMM_RELATIVE && 
            (OverlayManager::getSingleton().hasViewportChanged() || mGeomPositionsOutOfDate))
        {
            mLeftBorderSize = mPixelLeftBorderSize * mPixelScaleX;
            mRightBorderSize = mPixelRightBorderSize * mPixelScaleX;
            mTopBorderSize = mPixelTopBorderSize * mPixelScaleY;
            mBottomBorderSize = mPixelBottomBorderSize * mPixelScaleY;
            mGeomPositionsOutOfDate = true;
        }
		PanelOverlayElement::_update();
    }
    //-----------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // Command objects
    //---------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String BorderPanelOverlayElement::CmdBorderSize::doGet(const void* target) const
    {
		const BorderPanelOverlayElement* t = static_cast<const BorderPanelOverlayElement*>(target);
        return String(
			StringConverter::toString(t->getLeftBorderSize()) + " " +
			StringConverter::toString(t->getRightBorderSize()) + " " +
			StringConverter::toString(t->getTopBorderSize()) + " " +
			StringConverter::toString(t->getBottomBorderSize())	);
    }
    void BorderPanelOverlayElement::CmdBorderSize::doSet(void* target, const String& val)
    {
        vector<String>::type vec = StringUtil::split(val);

        static_cast<BorderPanelOverlayElement*>(target)->setBorderSize(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelOverlayElement::CmdBorderMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const BorderPanelOverlayElement*>(target)->getBorderMaterialName();
    }
    void BorderPanelOverlayElement::CmdBorderMaterial::doSet(void* target, const String& val)
    {
        vector<String>::type vec = StringUtil::split(val);

        static_cast<BorderPanelOverlayElement*>(target)->setBorderMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String BorderPanelOverlayElement::CmdBorderBottomLeftUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelOverlayElement*>(target)->getBottomLeftBorderUVString();
    }
    void BorderPanelOverlayElement::CmdBorderBottomLeftUV::doSet(void* target, const String& val)
    {
        vector<String>::type vec = StringUtil::split(val);

        static_cast<BorderPanelOverlayElement*>(target)->setBottomLeftBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelOverlayElement::CmdBorderBottomRightUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelOverlayElement*>(target)->getBottomRightBorderUVString();
    }
    void BorderPanelOverlayElement::CmdBorderBottomRightUV::doSet(void* target, const String& val)
    {
        vector<String>::type vec = StringUtil::split(val);

        static_cast<BorderPanelOverlayElement*>(target)->setBottomRightBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelOverlayElement::CmdBorderTopLeftUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelOverlayElement*>(target)->getTopLeftBorderUVString();
    }
    void BorderPanelOverlayElement::CmdBorderTopLeftUV::doSet(void* target, const String& val)
    {
        vector<String>::type vec = StringUtil::split(val);

        static_cast<BorderPanelOverlayElement*>(target)->setTopLeftBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelOverlayElement::CmdBorderTopRightUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelOverlayElement*>(target)->getTopRightBorderUVString();
    }
    void BorderPanelOverlayElement::CmdBorderTopRightUV::doSet(void* target, const String& val)
    {
        vector<String>::type vec = StringUtil::split(val);

        static_cast<BorderPanelOverlayElement*>(target)->setTopRightBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelOverlayElement::CmdBorderLeftUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelOverlayElement*>(target)->getLeftBorderUVString();
    }
    void BorderPanelOverlayElement::CmdBorderLeftUV::doSet(void* target, const String& val)
    {
        vector<String>::type vec = StringUtil::split(val);

        static_cast<BorderPanelOverlayElement*>(target)->setLeftBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelOverlayElement::CmdBorderRightUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelOverlayElement*>(target)->getRightBorderUVString();
    }
    void BorderPanelOverlayElement::CmdBorderRightUV::doSet(void* target, const String& val)
    {
        vector<String>::type vec = StringUtil::split(val);

        static_cast<BorderPanelOverlayElement*>(target)->setRightBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelOverlayElement::CmdBorderTopUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelOverlayElement*>(target)->getTopBorderUVString();
    }
    void BorderPanelOverlayElement::CmdBorderTopUV::doSet(void* target, const String& val)
    {
        vector<String>::type vec = StringUtil::split(val);

        static_cast<BorderPanelOverlayElement*>(target)->setTopBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelOverlayElement::CmdBorderBottomUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelOverlayElement*>(target)->getBottomBorderUVString();
    }
    void BorderPanelOverlayElement::CmdBorderBottomUV::doSet(void* target, const String& val)
    {
        vector<String>::type vec = StringUtil::split(val);

        static_cast<BorderPanelOverlayElement*>(target)->setBottomBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //---------------------------------------------------------------------
    const String& BorderPanelOverlayElement::getTypeName(void) const
    {
        return msTypeName;
    }



}

