/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

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
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreOverlayElement.h"
#include "OgreOverlay.h"
#include "OgreOverlayManager.h"
#include "OgreException.h"
#include "OgreMaterialManager.h"
#include "OgreOverlayContainer.h"
#include "OgreResourceGroupManager.h"
#include "OgreOverlayElementCommands.h"
#include "OgreTechnique.h"
#include "OgreLogManager.h"

namespace Ogre {


    //---------------------------------------------------------------------
    // Define static members
    // Command object for setting / getting parameters
    static OverlayElementCommands::CmdLeft msLeftCmd;
    static OverlayElementCommands::CmdTop msTopCmd;
    static OverlayElementCommands::CmdWidth msWidthCmd;
    static OverlayElementCommands::CmdHeight msHeightCmd;
    static OverlayElementCommands::CmdMaterial msMaterialCmd;
    static OverlayElementCommands::CmdCaption msCaptionCmd;
    static OverlayElementCommands::CmdMetricsMode msMetricsModeCmd;
    static OverlayElementCommands::CmdHorizontalAlign msHorizontalAlignCmd;
    static OverlayElementCommands::CmdVerticalAlign msVerticalAlignCmd;
    static OverlayElementCommands::CmdVisible msVisibleCmd;

    const String& OverlayElement::DEFAULT_RESOURCE_GROUP = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME;
    //---------------------------------------------------------------------
    OverlayElement::OverlayElement(const String& name)
      : mName(name)
      , mVisible(true)
      , mCloneable(true)
      , mLeft(0.0f)
      , mTop(0.0f)
      , mWidth(1.0f)
      , mHeight(1.0f)
      , mMetricsMode(GMM_RELATIVE)
      , mHorzAlign(GHA_LEFT)
      , mVertAlign(GVA_TOP)
      , mPixelTop(0.0)
      , mPixelLeft(0.0)
      , mPixelWidth(1.0)
      , mPixelHeight(1.0)
      , mPixelScaleX(1.0)
      , mPixelScaleY(1.0)
      , mParent(0)
      , mOverlay(0)
      , mDerivedLeft(0)
      , mDerivedTop(0)
      , mDerivedOutOfDate(true)
      , mGeomPositionsOutOfDate(true)
      , mGeomUVsOutOfDate(true)
      , mZOrder(0)
      , mEnabled(true)
      , mInitialised(false)
    {
        // default overlays to preserve their own detail level
        mPolygonModeOverrideable = false;

        // use identity projection and view matrices
        mUseIdentityProjection = true;
        mUseIdentityView = true;
    }
    //---------------------------------------------------------------------
    OverlayElement::~OverlayElement()
    {
        if (mParent)
        {
            mParent->removeChild(mName);
            mParent = 0;
        }
    }
    //---------------------------------------------------------------------
    void OverlayElement::setDimensions(Real width, Real height)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelWidth = width;
            mPixelHeight = height;
        }
        else
        {
            mWidth = width;
            mHeight = height;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void OverlayElement::setPosition(Real left, Real top)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelLeft = left;
            mPixelTop = top;
        }
        else
        {
            mLeft = left;
            mTop = top;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void OverlayElement::setWidth(Real width)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelWidth = width;
        }
        else
        {
            mWidth = width;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    Real OverlayElement::getWidth(void) const
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            return mPixelWidth;
        }
        else
        {
            return mWidth;
        }
    }
    //---------------------------------------------------------------------
    void OverlayElement::setHeight(Real height)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelHeight = height;
        }
        else
        {
            mHeight = height;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    Real OverlayElement::getHeight(void) const
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            return mPixelHeight;
        }
        else
        {
            return mHeight;
        }
    }
    //---------------------------------------------------------------------
    void OverlayElement::setLeft(Real left)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelLeft = left;
        }
        else
        {
            mLeft = left;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    Real OverlayElement::getLeft(void) const
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            return mPixelLeft;
        }
        else
        {
            return mLeft;
        }
    }
    //---------------------------------------------------------------------
    void OverlayElement::setTop(Real top)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelTop = top;
        }
        else
        {
            mTop = top;
        }

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    Real OverlayElement::getTop(void) const
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            return mPixelTop;
        }
        else
        {
            return mTop;
        }
    }
    //---------------------------------------------------------------------
    void OverlayElement::_setLeft(Real left)
    {
        mLeft = left;
        mPixelLeft = left / mPixelScaleX;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void OverlayElement::_setTop(Real top)
    {
        mTop = top;
        mPixelTop = top / mPixelScaleY;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void OverlayElement::_setWidth(Real width)
    {
        mWidth = width;
        mPixelWidth = width / mPixelScaleX;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void OverlayElement::_setHeight(Real height)
    {
        mHeight = height;
        mPixelHeight = height / mPixelScaleY;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void OverlayElement::_setPosition(Real left, Real top)
    {
        mLeft = left;
        mTop  = top;
        mPixelLeft = left / mPixelScaleX;
        mPixelTop  = top / mPixelScaleY;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void OverlayElement::_setDimensions(Real width, Real height)
    {
        mWidth  = width;
        mHeight = height;
        mPixelWidth  = width / mPixelScaleX;
        mPixelHeight = height / mPixelScaleY;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    const String& OverlayElement::getMaterialName(void) const
    {
        return mMaterial ? mMaterial->getName() : BLANKSTRING;
    }
    //---------------------------------------------------------------------
    void OverlayElement::setMaterial(const MaterialPtr& mat)
    {
        mMaterial = mat;

        if(!mMaterial)
            return;

        mMaterial->load();

        auto dstPass = mMaterial->getTechnique(0)->getPass(0); // assume this is representative
        if (dstPass->getLightingEnabled() || dstPass->getDepthCheckEnabled())
        {
            LogManager::getSingleton().logWarning(
                "force-disabling 'lighting' and 'depth_check' of Material " + mat->getName() +
                " for use with OverlayElement " + getName());
        }

        // Set some prerequisites to be sure
        mMaterial->setLightingEnabled(false);
        mMaterial->setReceiveShadows(false);
        mMaterial->setDepthCheckEnabled(false);
    }

    void OverlayElement::setMaterialName(const String& matName, const String& group)
    {
        if (!matName.empty())
        {
            mMaterial = MaterialManager::getSingleton().getByName(matName, group);
            if (!mMaterial)
                OGRE_EXCEPT( Exception::ERR_ITEM_NOT_FOUND, "Could not find material " + matName,
                    "OverlayElement::setMaterialName" );

            setMaterial(mMaterial);
        }
        else
        {
            mMaterial.reset();
        }
    }
    //---------------------------------------------------------------------
    const MaterialPtr& OverlayElement::getMaterial(void) const
    {
        return mMaterial;
    }
    //---------------------------------------------------------------------
    void OverlayElement::getWorldTransforms(Matrix4* xform) const
    {
        mOverlay->_getWorldTransforms(xform);
    }
    //---------------------------------------------------------------------
    void OverlayElement::_positionsOutOfDate(void)
    {
        mGeomPositionsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void OverlayElement::_update(void)
    {
        Real vpWidth, vpHeight;
        OverlayManager& oMgr = OverlayManager::getSingleton();
        vpWidth = (Real) (oMgr.getViewportWidth());
        vpHeight = (Real) (oMgr.getViewportHeight());

        // Check size if pixel-based or relative-aspect-adjusted
        switch (mMetricsMode)
        {
        case GMM_PIXELS :
            if (mGeomPositionsOutOfDate)
            {               
                mPixelScaleX = 1.0f / vpWidth;
                mPixelScaleY = 1.0f / vpHeight; 
            }
            break;

        case GMM_RELATIVE_ASPECT_ADJUSTED :
            if (mGeomPositionsOutOfDate)
            {
                mPixelScaleX = 1.0f / (10000.0f * (vpWidth / vpHeight));
                mPixelScaleY = 1.0f /  10000.0f;
            }
            break;

        default:
        case GMM_RELATIVE :
            mPixelScaleX = 1.0;
            mPixelScaleY = 1.0;
            mPixelLeft = mLeft;
            mPixelTop = mTop;
            mPixelWidth = mWidth;
            mPixelHeight = mHeight;
            break;
        }

        mLeft   = mPixelLeft   * mPixelScaleX;
        mTop    = mPixelTop    * mPixelScaleY;
        mWidth  = mPixelWidth  * mPixelScaleX;
        mHeight = mPixelHeight * mPixelScaleY;

        Real tmpPixelWidth = mPixelWidth;

        _updateFromParent();
        // NB container subclasses will update children too

        // Tell self to update own position geometry
        if (mGeomPositionsOutOfDate && mInitialised)
        {
            updatePositionGeometry();

            // Within updatePositionGeometry() of TextOverlayElements,
            // the needed pixel width is calculated and as a result a new 
            // second update call is needed, so leave the dirty flag on 'true'
            // so that that in a second call in the next frame, the element
            // width can be correctly set and the text gets displayed.
            if(mMetricsMode == GMM_PIXELS && tmpPixelWidth != mPixelWidth)
                mGeomPositionsOutOfDate = true;
            else
                mGeomPositionsOutOfDate = false;
        }

        // Tell self to update own texture geometry
        if (mGeomUVsOutOfDate && mInitialised)
        {
            updateTextureGeometry();
            mGeomUVsOutOfDate = false;
        } 
    }
    //---------------------------------------------------------------------
    void OverlayElement::_updateFromParent(void)
    {
        Real parentLeft = 0, parentTop = 0, parentBottom = 0, parentRight = 0;

        if (mParent)
        {
            parentLeft = mParent->_getDerivedLeft();
            parentTop = mParent->_getDerivedTop();
            if (mHorzAlign == GHA_CENTER || mHorzAlign == GHA_RIGHT)
            {
                parentRight = parentLeft + mParent->_getRelativeWidth();
            }
            if (mVertAlign == GVA_CENTER || mVertAlign == GVA_BOTTOM)
            {
                parentBottom = parentTop + mParent->_getRelativeHeight();
            }
        }
        else
        {
            RenderSystem* rSys = Root::getSingleton().getRenderSystem();
            OverlayManager& oMgr = OverlayManager::getSingleton();

            // Calculate offsets required for mapping texel origins to pixel origins in the
            // current rendersystem
            Real hOffset = rSys->getHorizontalTexelOffset() / oMgr.getViewportWidth();
            Real vOffset = rSys->getVerticalTexelOffset() / oMgr.getViewportHeight();

            parentLeft = 0.0f + hOffset;
            parentTop = 0.0f + vOffset;
            parentRight = 1.0f + hOffset;
            parentBottom = 1.0f + vOffset;
        }

        // Sort out position based on alignment
        // NB all we do is derived the origin, we don't automatically sort out the position
        // This is more flexible than forcing absolute right & middle 
        switch(mHorzAlign)
        {
        case GHA_CENTER:
            mDerivedLeft = ((parentLeft + parentRight) * 0.5f) + mLeft;
            break;
        case GHA_LEFT:
            mDerivedLeft = parentLeft + mLeft;
            break;
        case GHA_RIGHT:
            mDerivedLeft = parentRight + mLeft;
            break;
        };
        switch(mVertAlign)
        {
        case GVA_CENTER:
            mDerivedTop = ((parentTop + parentBottom) * 0.5f) + mTop;
            break;
        case GVA_TOP:
            mDerivedTop = parentTop + mTop;
            break;
        case GVA_BOTTOM:
            mDerivedTop = parentBottom + mTop;
            break;
        };

        mDerivedOutOfDate = false;

        if (mParent != 0)
        {
            RealRect parent;
            RealRect child;

            mParent->_getClippingRegion(parent);

            child.left   = mDerivedLeft;
            child.top    = mDerivedTop;
            child.right  = mDerivedLeft + mWidth;
            child.bottom = mDerivedTop + mHeight;

            mClippingRegion = parent.intersect(child);
        }
        else
        {
            mClippingRegion.left   = mDerivedLeft;
            mClippingRegion.top    = mDerivedTop;
            mClippingRegion.right  = mDerivedLeft + mWidth;
            mClippingRegion.bottom = mDerivedTop + mHeight;
        }
    }
    //---------------------------------------------------------------------
    void OverlayElement::_notifyParent(OverlayContainer* parent, Overlay* overlay)
    {
        mParent = parent;
        mOverlay = overlay;

        if (mOverlay && mOverlay->isInitialised() && !mInitialised)
        {
            initialise();
        }

        mDerivedOutOfDate = true;
    }
    //---------------------------------------------------------------------
    Real OverlayElement::_getDerivedLeft(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedLeft;
    }
    //---------------------------------------------------------------------
    Real OverlayElement::_getDerivedTop(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedTop;
    }
    //---------------------------------------------------------------------
    Real OverlayElement::_getRelativeWidth(void)
    {
        return mWidth;
    }
    //---------------------------------------------------------------------
    Real OverlayElement::_getRelativeHeight(void)
    {
        return mHeight;
    }
    //---------------------------------------------------------------------    
    void OverlayElement::_getClippingRegion(RealRect &clippingRegion)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        clippingRegion = mClippingRegion;
    }
    //---------------------------------------------------------------------
    ushort OverlayElement::_notifyZOrder(ushort newZOrder)
    {
        mZOrder = newZOrder;
        return mZOrder + 1;
    }
    //---------------------------------------------------------------------
    void OverlayElement::_notifyWorldTransforms(const Matrix4& xform)
    {
        mXForm = xform;
    }
    //---------------------------------------------------------------------
    void OverlayElement::_notifyViewport()
    {
        switch (mMetricsMode)
        {
        case GMM_PIXELS :
            {
                Real vpWidth, vpHeight;
                OverlayManager& oMgr = OverlayManager::getSingleton();
                vpWidth = (Real) (oMgr.getViewportWidth());
                vpHeight = (Real) (oMgr.getViewportHeight());

                mPixelScaleX = 1.0f / vpWidth;
                mPixelScaleY = 1.0f / vpHeight;
            }
            break;

        case GMM_RELATIVE_ASPECT_ADJUSTED :
            {
                Real vpWidth, vpHeight;
                OverlayManager& oMgr = OverlayManager::getSingleton();
                vpWidth = (Real) (oMgr.getViewportWidth());
                vpHeight = (Real) (oMgr.getViewportHeight());

                mPixelScaleX = 1.0f / (10000.0f * (vpWidth / vpHeight));
                mPixelScaleY = 1.0f /  10000.0f;
            }
            break;

        default:
        case GMM_RELATIVE :
            mPixelScaleX = 1.0;
            mPixelScaleY = 1.0;
            mPixelLeft = mLeft;
            mPixelTop = mTop;
            mPixelWidth = mWidth;
            mPixelHeight = mHeight;
            break;
        }

        mLeft = mPixelLeft * mPixelScaleX;
        mTop = mPixelTop * mPixelScaleY;
        mWidth = mPixelWidth * mPixelScaleX;
        mHeight = mPixelHeight * mPixelScaleY;

        mGeomPositionsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void OverlayElement::_updateRenderQueue(RenderQueue* queue)
    {
        if (mVisible)
        {
            queue->addRenderable(this, RENDER_QUEUE_OVERLAY, mZOrder);
        }      
    }
    //---------------------------------------------------------------------
    void OverlayElement::visitRenderables(Renderable::Visitor* visitor, 
        bool debugRenderables)
    {
        visitor->visit(this, 0, false);
    }
    //-----------------------------------------------------------------------
    void OverlayElement::addBaseParameters(void)    
    {
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("left", 
            "The position of the left border of the gui element."
            , PT_REAL),
            &msLeftCmd);
        dict->addParameter(ParameterDef("top", 
            "The position of the top border of the gui element."
            , PT_REAL),
            &msTopCmd);
        dict->addParameter(ParameterDef("width", 
            "The width of the element."
            , PT_REAL),
            &msWidthCmd);
        dict->addParameter(ParameterDef("height", 
            "The height of the element."
            , PT_REAL),
            &msHeightCmd);
        dict->addParameter(ParameterDef("material", 
            "The name of the material to use."
            , PT_STRING),
            &msMaterialCmd);
        dict->addParameter(ParameterDef("caption", 
            "The element caption, if supported."
            , PT_STRING),
            &msCaptionCmd);
        dict->addParameter(ParameterDef("metrics_mode", 
            "The type of metrics to use, either 'relative' to the screen, 'pixels' or 'relative_aspect_adjusted'."
            , PT_STRING),
            &msMetricsModeCmd);
        dict->addParameter(ParameterDef("horz_align", 
            "The horizontal alignment, 'left', 'right' or 'center'."
            , PT_STRING),
            &msHorizontalAlignCmd);
        dict->addParameter(ParameterDef("vert_align", 
            "The vertical alignment, 'top', 'bottom' or 'center'."
            , PT_STRING),
            &msVerticalAlignCmd);
        dict->addParameter(ParameterDef("visible", 
            "Initial visibility of element, either 'true' or 'false' (default true)."
            , PT_STRING),
            &msVisibleCmd);
    }
    //-----------------------------------------------------------------------
    void OverlayElement::setCaption( const DisplayString& caption )
    {
        mCaption = caption;
        _positionsOutOfDate();
    }
    //-----------------------------------------------------------------------
    void OverlayElement::setColour(const ColourValue& col)
    {
        mColour = col;
    }
    //-----------------------------------------------------------------------
    const ColourValue& OverlayElement::getColour(void) const
    {
        return mColour;
    }
    //-----------------------------------------------------------------------
    void OverlayElement::setMetricsMode(GuiMetricsMode gmm)
    {
        switch (gmm)
        {
        case GMM_PIXELS :
            {
                Real vpWidth, vpHeight;
                OverlayManager& oMgr = OverlayManager::getSingleton();
                vpWidth = (Real) (oMgr.getViewportWidth());
                vpHeight = (Real) (oMgr.getViewportHeight());

                // cope with temporarily zero dimensions, avoid divide by zero
                vpWidth = vpWidth == 0.0f? 1.0f : vpWidth;
                vpHeight = vpHeight == 0.0f? 1.0f : vpHeight;

                mPixelScaleX = 1.0f / vpWidth;
                mPixelScaleY = 1.0f / vpHeight;

                if (mMetricsMode == GMM_RELATIVE)
                {
                    mPixelLeft = mLeft;
                    mPixelTop = mTop;
                    mPixelWidth = mWidth;
                    mPixelHeight = mHeight;
                }
            }
            break;

        case GMM_RELATIVE_ASPECT_ADJUSTED :
            {
                Real vpWidth, vpHeight;
                OverlayManager& oMgr = OverlayManager::getSingleton();
                vpWidth = (Real) (oMgr.getViewportWidth());
                vpHeight = (Real) (oMgr.getViewportHeight());

                mPixelScaleX = 1.0f / (10000.0f * (vpWidth / vpHeight));
                mPixelScaleY = 1.0f /  10000.0f;

                if (mMetricsMode == GMM_RELATIVE)
                {
                    mPixelLeft = mLeft;
                    mPixelTop = mTop;
                    mPixelWidth = mWidth;
                    mPixelHeight = mHeight;
                }
            }
            break;

        default:
        case GMM_RELATIVE :
            mPixelScaleX = 1.0;
            mPixelScaleY = 1.0;
            mPixelLeft = mLeft;
            mPixelTop = mTop;
            mPixelWidth = mWidth;
            mPixelHeight = mHeight;
            break;
        }

        mLeft = mPixelLeft * mPixelScaleX;
        mTop = mPixelTop * mPixelScaleY;
        mWidth = mPixelWidth * mPixelScaleX;
        mHeight = mPixelHeight * mPixelScaleY;

        mMetricsMode = gmm;
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //-----------------------------------------------------------------------
    void OverlayElement::setHorizontalAlignment(GuiHorizontalAlignment gha)
    {
        mHorzAlign = gha;
        _positionsOutOfDate();
    }
    //-----------------------------------------------------------------------
    void OverlayElement::setVerticalAlignment(GuiVerticalAlignment gva)
    {
        mVertAlign = gva;
        _positionsOutOfDate();
    }
    //-----------------------------------------------------------------------    
    bool OverlayElement::contains(Real x, Real y) const
    {
        return x >= mClippingRegion.left && x <= mClippingRegion.right && y >= mClippingRegion.top && y <= mClippingRegion.bottom;
    }
    //-----------------------------------------------------------------------
    OverlayElement* OverlayElement::findElementAt(Real x, Real y)       // relative to parent
    {
        OverlayElement* ret = NULL;
        if (contains(x , y ))
        {
            ret = this;
        }
        return ret;
    }
    //-----------------------------------------------------------------------
    OverlayElement* OverlayElement::clone(const String& instanceName)
    {
        OverlayElement* newElement;

        newElement = OverlayManager::getSingleton().createOverlayElement(
            getTypeName(), instanceName + "/" + mName);
        copyParametersTo(newElement);

        return newElement;
    }
    //-----------------------------------------------------------------------

}

