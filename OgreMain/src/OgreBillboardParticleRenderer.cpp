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
#include "OgreStableHeaders.h"

#include "OgreBillboardParticleRenderer.h"
#include "OgreParticle.h"
#include "OgreStringConverter.h"
#include "OgreSceneNode.h"

namespace Ogre {
    String rendererTypeName = "billboard";

    //-----------------------------------------------------------------------
    BillboardParticleRenderer::CmdBillboardType BillboardParticleRenderer::msBillboardTypeCmd;
    BillboardParticleRenderer::CmdBillboardOrigin BillboardParticleRenderer::msBillboardOriginCmd;
    BillboardParticleRenderer::CmdBillboardRotationType BillboardParticleRenderer::msBillboardRotationTypeCmd;
    BillboardParticleRenderer::CmdCommonDirection BillboardParticleRenderer::msCommonDirectionCmd;
    BillboardParticleRenderer::CmdCommonUpVector BillboardParticleRenderer::msCommonUpVectorCmd;
    BillboardParticleRenderer::CmdPointRendering BillboardParticleRenderer::msPointRenderingCmd;
	BillboardParticleRenderer::CmdAccurateFacing BillboardParticleRenderer::msAccurateFacingCmd;
    //-----------------------------------------------------------------------
    BillboardParticleRenderer::BillboardParticleRenderer()
    {
        if (createParamDictionary("BillboardParticleRenderer"))
        {
            ParamDictionary* dict = getParamDictionary();
            dict->addParameter(ParameterDef("billboard_type", 
                "The type of billboard to use. 'point' means a simulated spherical particle, " 
                "'oriented_common' means all particles in the set are oriented around common_direction, "
                "'oriented_self' means particles are oriented around their own direction, "
                "'perpendicular_common' means all particles are perpendicular to common_direction, "
                "and 'perpendicular_self' means particles are perpendicular to their own direction.",
                PT_STRING),
                &msBillboardTypeCmd);

            dict->addParameter(ParameterDef("billboard_origin", 
                "This setting controls the fine tuning of where a billboard appears in relation to it's position. "
                "Possible value are: 'top_left', 'top_center', 'top_right', 'center_left', 'center', 'center_right', "
                "'bottom_left', 'bottom_center' and 'bottom_right'. Default value is 'center'.",
                PT_STRING),
                &msBillboardOriginCmd);

            dict->addParameter(ParameterDef("billboard_rotation_type", 
                "This setting controls the billboard rotation type. "
				"'vertex' means rotate the billboard's vertices around their facing direction."
                "'texcoord' means rotate the billboard's texture coordinates. Default value is 'texcoord'.",
                PT_STRING),
                &msBillboardRotationTypeCmd);

            dict->addParameter(ParameterDef("common_direction", 
                "Only useful when billboard_type is oriented_common or perpendicular_common. "
				"When billboard_type is oriented_common, this parameter sets the common orientation for "
				"all particles in the set (e.g. raindrops may all be oriented downwards). "
				"When billboard_type is perpendicular_common, this parameter sets the perpendicular vector for "
				"all particles in the set (e.g. an aureola around the player and parallel to the ground).",
                PT_VECTOR3),
                &msCommonDirectionCmd);

            dict->addParameter(ParameterDef("common_up_vector",
                "Only useful when billboard_type is perpendicular_self or perpendicular_common. This "
				"parameter sets the common up-vector for all particles in the set (e.g. an aureola around "
				"the player and parallel to the ground).",
                PT_VECTOR3),
                &msCommonUpVectorCmd);
            dict->addParameter(ParameterDef("point_rendering",
                "Set whether or not particles will use point rendering "
				"rather than manually generated quads. This allows for faster "
				"rendering of point-oriented particles although introduces some "
				"limitations too such as requiring a common particle size."
				"Possible values are 'true' or 'false'.",
                PT_BOOL),
                &msPointRenderingCmd);
			dict->addParameter(ParameterDef("accurate_facing",
				"Set whether or not particles will be oriented to the camera "
				"based on the relative position to the camera rather than just "
				"the camera direction. This is more accurate but less optimal. "
				"Cannot be combined with point rendering.",
				PT_BOOL),
				&msAccurateFacingCmd);
        }

        // Create billboard set
        mBillboardSet = OGRE_NEW BillboardSet("", 0, true);
        // World-relative axes
        mBillboardSet->setBillboardsInWorldSpace(true);
    }
    //-----------------------------------------------------------------------
    BillboardParticleRenderer::~BillboardParticleRenderer()
    {
		// mBillboardSet is never actually attached to a node, we just passthrough
		// based on the particle system's attachment. So manually notify that it's
		// no longer attached.
		mBillboardSet->_notifyAttached(0);
        OGRE_DELETE  mBillboardSet;
    }
    //-----------------------------------------------------------------------
    const String& BillboardParticleRenderer::getType(void) const
    {
        return rendererTypeName;
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_updateRenderQueue(RenderQueue* queue, 
        list<Particle*>::type& currentParticles, bool cullIndividually)
    {
        mBillboardSet->setCullIndividually(cullIndividually);

        // Update billboard set geometry
        Vector3 bboxMin = Math::POS_INFINITY * Vector3::UNIT_SCALE;
        Vector3 bboxMax = Math::NEG_INFINITY * Vector3::UNIT_SCALE;
        Real radius = 0.0f;
        mBillboardSet->beginBillboards(currentParticles.size());
        Billboard bb;
        Matrix4 invWorld;

        if (mBillboardSet->getBillboardsInWorldSpace() && mBillboardSet->getParentSceneNode())
            invWorld = mBillboardSet->getParentSceneNode()->_getFullTransform().inverse();

        for (list<Particle*>::type::iterator i = currentParticles.begin();
            i != currentParticles.end(); ++i)
        {
            Particle* p = *i;
            bb.mPosition = p->position;
            Vector3 pos = p->position;

            if (mBillboardSet->getBillboardsInWorldSpace() && mBillboardSet->getParentSceneNode())
                pos = invWorld * pos;

            bboxMin.makeFloor( pos );
            bboxMax.makeCeil( pos );
            radius = std::max( radius, p->position.length() );

            if (mBillboardSet->getBillboardType() == BBT_ORIENTED_SELF ||
				mBillboardSet->getBillboardType() == BBT_PERPENDICULAR_SELF)
			{
				// Normalise direction vector
				bb.mDirection = p->direction;
				bb.mDirection.normalise();
			}
            bb.mColour = p->colour;
            bb.mRotation = p->rotation;
            // Assign and compare at the same time
            if ((bb.mOwnDimensions = p->mOwnDimensions) == true)
            {
                bb.mWidth = p->mWidth;
                bb.mHeight = p->mHeight;
            }
            mBillboardSet->injectBillboard(bb);

        }

        // Only set bounds if there are any active particles
        if(currentParticles.size())
            mBillboardSet->setBounds( AxisAlignedBox( bboxMin, bboxMax ), radius );

        mBillboardSet->endBillboards();

        // Update the queue
        mBillboardSet->_updateRenderQueue(queue);
    }
	//---------------------------------------------------------------------
	void BillboardParticleRenderer::visitRenderables(Renderable::Visitor* visitor, 
		bool debugRenderables)
	{
		mBillboardSet->visitRenderables(visitor, debugRenderables);
	}
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_setMaterial(MaterialPtr& mat)
    {
        mBillboardSet->setMaterialName(mat->getName(), mat->getGroup());
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::setBillboardType(BillboardType bbt)
    {
        mBillboardSet->setBillboardType(bbt);
    }
	//-----------------------------------------------------------------------
	void BillboardParticleRenderer::setUseAccurateFacing(bool acc)
	{
		mBillboardSet->setUseAccurateFacing(acc);
	}
	//-----------------------------------------------------------------------
	bool BillboardParticleRenderer::getUseAccurateFacing(void) const
	{
		return mBillboardSet->getUseAccurateFacing();
	}
    //-----------------------------------------------------------------------
    BillboardType BillboardParticleRenderer::getBillboardType(void) const
    {
        return mBillboardSet->getBillboardType();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::setBillboardRotationType(BillboardRotationType rotationType)
    {
        mBillboardSet->setBillboardRotationType(rotationType);
    }
    //-----------------------------------------------------------------------
    BillboardRotationType BillboardParticleRenderer::getBillboardRotationType(void) const
    {
        return mBillboardSet->getBillboardRotationType();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::setCommonDirection(const Vector3& vec)
    {
        mBillboardSet->setCommonDirection(vec);
    }
    //-----------------------------------------------------------------------
    const Vector3& BillboardParticleRenderer::getCommonDirection(void) const
    {
        return mBillboardSet->getCommonDirection();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::setCommonUpVector(const Vector3& vec)
    {
        mBillboardSet->setCommonUpVector(vec);
    }
    //-----------------------------------------------------------------------
    const Vector3& BillboardParticleRenderer::getCommonUpVector(void) const
    {
        return mBillboardSet->getCommonUpVector();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyCurrentCamera(Camera* cam)
    {
        mBillboardSet->_notifyCurrentCamera(cam);
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyParticleRotated(void)
    {
        mBillboardSet->_notifyBillboardRotated();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyDefaultDimensions(Real width, Real height)
    {
        mBillboardSet->setDefaultDimensions(width, height);
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyParticleResized(void)
    {
        mBillboardSet->_notifyBillboardResized();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyParticleQuota(size_t quota)
    {
        mBillboardSet->setPoolSize(quota);
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRenderer::_notifyAttached(Node* parent, bool isTagPoint)
    {
        mBillboardSet->_notifyAttached(parent, isTagPoint);
    }
	//-----------------------------------------------------------------------
	void BillboardParticleRenderer::setRenderQueueGroup(uint8 queueID)
	{
		assert(queueID <= RENDER_QUEUE_MAX && "Render queue out of range!");
		mBillboardSet->setRenderQueueGroup(queueID);
	}
	//-----------------------------------------------------------------------
	void BillboardParticleRenderer::setRenderQueueGroupAndPriority(uint8 queueID, ushort priority)
	{
		assert(queueID <= RENDER_QUEUE_MAX && "Render queue out of range!");
		mBillboardSet->setRenderQueueGroupAndPriority(queueID, priority);
	}
	//-----------------------------------------------------------------------
	void BillboardParticleRenderer::setKeepParticlesInLocalSpace(bool keepLocal)
	{
		mBillboardSet->setBillboardsInWorldSpace(!keepLocal);
	}
    //-----------------------------------------------------------------------
    SortMode BillboardParticleRenderer::_getSortMode(void) const
    {
        return mBillboardSet->_getSortMode();
    }
	//-----------------------------------------------------------------------
	void BillboardParticleRenderer::setPointRenderingEnabled(bool enabled)
	{
		mBillboardSet->setPointRenderingEnabled(enabled);
	}
	//-----------------------------------------------------------------------
	bool BillboardParticleRenderer::isPointRenderingEnabled(void) const
	{
		return mBillboardSet->isPointRenderingEnabled();
	}
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    const String& BillboardParticleRendererFactory::getType() const
    {
        return rendererTypeName;
    }
    //-----------------------------------------------------------------------
    ParticleSystemRenderer* BillboardParticleRendererFactory::createInstance( 
        const String& name )
    {
        return OGRE_NEW BillboardParticleRenderer();
    }
    //-----------------------------------------------------------------------
    void BillboardParticleRendererFactory::destroyInstance( 
        ParticleSystemRenderer* inst)
    {
        OGRE_DELETE  inst;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String BillboardParticleRenderer::CmdBillboardType::doGet(const void* target) const
    {
        BillboardType t = static_cast<const BillboardParticleRenderer*>(target)->getBillboardType();
        switch(t)
        {
        case BBT_POINT:
            return "point";
            break;
        case BBT_ORIENTED_COMMON:
            return "oriented_common";
            break;
        case BBT_ORIENTED_SELF:
            return "oriented_self";
            break;
        case BBT_PERPENDICULAR_COMMON:
            return "perpendicular_common";
        case BBT_PERPENDICULAR_SELF:
            return "perpendicular_self";
        }
        // Compiler nicety
        return "";
    }
    void BillboardParticleRenderer::CmdBillboardType::doSet(void* target, const String& val)
    {
        BillboardType t;
        if (val == "point")
        {
            t = BBT_POINT;
        }
        else if (val == "oriented_common")
        {
            t = BBT_ORIENTED_COMMON;
        }
        else if (val == "oriented_self")
        {
            t = BBT_ORIENTED_SELF;
        }
        else if (val == "perpendicular_common")
        {
            t = BBT_PERPENDICULAR_COMMON;
        }
        else if (val == "perpendicular_self")
        {
            t = BBT_PERPENDICULAR_SELF;
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Invalid billboard_type '" + val + "'", 
                "ParticleSystem::CmdBillboardType::doSet");
        }

        static_cast<BillboardParticleRenderer*>(target)->setBillboardType(t);
    }
    //-----------------------------------------------------------------------
    String BillboardParticleRenderer::CmdBillboardOrigin::doGet(const void* target) const
    {
        BillboardOrigin o = static_cast<const BillboardParticleRenderer*>(target)->getBillboardOrigin();
        switch (o)
        {
        case BBO_TOP_LEFT:
            return "top_left";
        case BBO_TOP_CENTER:
            return "top_center";
        case BBO_TOP_RIGHT:
            return "top_right";
        case BBO_CENTER_LEFT:
            return "center_left";
        case BBO_CENTER:
            return "center";
        case BBO_CENTER_RIGHT:
            return "center_right";
        case BBO_BOTTOM_LEFT:
            return "bottom_left";
        case BBO_BOTTOM_CENTER:
            return "bottom_center";
        case BBO_BOTTOM_RIGHT:
            return "bottom_right";
        }
        // Compiler nicety
        return StringUtil::BLANK;
    }
    void BillboardParticleRenderer::CmdBillboardOrigin::doSet(void* target, const String& val)
    {
        BillboardOrigin o;
        if (val == "top_left")
            o = BBO_TOP_LEFT;
        else if (val =="top_center")
            o = BBO_TOP_CENTER;
        else if (val =="top_right")
            o = BBO_TOP_RIGHT;
        else if (val =="center_left")
            o = BBO_CENTER_LEFT;
        else if (val =="center")
            o = BBO_CENTER;
        else if (val =="center_right")
            o = BBO_CENTER_RIGHT;
        else if (val =="bottom_left")
            o = BBO_BOTTOM_LEFT;
        else if (val =="bottom_center")
            o = BBO_BOTTOM_CENTER;
        else if (val =="bottom_right")
            o = BBO_BOTTOM_RIGHT;
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Invalid billboard_origin '" + val + "'", 
                "ParticleSystem::CmdBillboardOrigin::doSet");
        }

        static_cast<BillboardParticleRenderer*>(target)->setBillboardOrigin(o);
    }
    //-----------------------------------------------------------------------
    String BillboardParticleRenderer::CmdBillboardRotationType::doGet(const void* target) const
    {
        BillboardRotationType r = static_cast<const BillboardParticleRenderer*>(target)->getBillboardRotationType();
        switch(r)
        {
        case BBR_VERTEX:
            return "vertex";
        case BBR_TEXCOORD:
            return "texcoord";
        }
        // Compiler nicety
        return StringUtil::BLANK;
    }
    void BillboardParticleRenderer::CmdBillboardRotationType::doSet(void* target, const String& val)
    {
        BillboardRotationType r;
        if (val == "vertex")
            r = BBR_VERTEX;
        else if (val == "texcoord")
            r = BBR_TEXCOORD;
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Invalid billboard_rotation_type '" + val + "'", 
                "ParticleSystem::CmdBillboardRotationType::doSet");
        }

        static_cast<BillboardParticleRenderer*>(target)->setBillboardRotationType(r);
    }
    //-----------------------------------------------------------------------
    String BillboardParticleRenderer::CmdCommonDirection::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const BillboardParticleRenderer*>(target)->getCommonDirection() );
    }
    void BillboardParticleRenderer::CmdCommonDirection::doSet(void* target, const String& val)
    {
        static_cast<BillboardParticleRenderer*>(target)->setCommonDirection(
            StringConverter::parseVector3(val));
    }
    //-----------------------------------------------------------------------
    String BillboardParticleRenderer::CmdCommonUpVector::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const BillboardParticleRenderer*>(target)->getCommonUpVector() );
    }
    void BillboardParticleRenderer::CmdCommonUpVector::doSet(void* target, const String& val)
    {
        static_cast<BillboardParticleRenderer*>(target)->setCommonUpVector(
            StringConverter::parseVector3(val));
    }
    //-----------------------------------------------------------------------
    String BillboardParticleRenderer::CmdPointRendering::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const BillboardParticleRenderer*>(target)->isPointRenderingEnabled() );
    }
    void BillboardParticleRenderer::CmdPointRendering::doSet(void* target, const String& val)
    {
        static_cast<BillboardParticleRenderer*>(target)->setPointRenderingEnabled(
            StringConverter::parseBool(val));
    }
	//-----------------------------------------------------------------------
	String BillboardParticleRenderer::CmdAccurateFacing::doGet(const void* target) const
	{
		return StringConverter::toString(
			static_cast<const BillboardParticleRenderer*>(target)->getUseAccurateFacing() );
	}
	void BillboardParticleRenderer::CmdAccurateFacing::doSet(void* target, const String& val)
	{
		static_cast<BillboardParticleRenderer*>(target)->setUseAccurateFacing(
			StringConverter::parseBool(val));
	}

}


