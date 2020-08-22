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
#include "OgreRibbonTrail.h"
#include "OgreController.h"

namespace Ogre
{
    namespace
    {
        /** Controller value for pass frame time to RibbonTrail
        */
        class _OgrePrivate TimeControllerValue : public ControllerValue<Real>
        {
        protected:
            RibbonTrail* mTrail;
        public:
            TimeControllerValue(RibbonTrail* r) { mTrail = r; }

            Real getValue(void) const { return 0; }// not a source 
            void setValue(Real value) { mTrail->_timeUpdate(value); }
        };
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    RibbonTrail::RibbonTrail(const String& name, size_t maxElements, 
        size_t numberOfChains, bool useTextureCoords, bool useColours)
        :BillboardChain(name, maxElements, 0, useTextureCoords, useColours, true),
        mFadeController(0)
    {
        setTrailLength(100);
        setNumberOfChains(numberOfChains);
        mTimeControllerValue = ControllerValueRealPtr(OGRE_NEW TimeControllerValue(this));

        // use V as varying texture coord, so we can use 1D textures to 'smear'
        setTextureCoordDirection(TCD_V);


    }
    //-----------------------------------------------------------------------
    RibbonTrail::~RibbonTrail()
    {
        // Detach listeners
        for (NodeList::iterator i = mNodeList.begin(); i != mNodeList.end(); ++i)
        {
            (*i)->setListener(0);
        }

        if (mFadeController)
        {
            // destroy controller
            ControllerManager::getSingleton().destroyController(mFadeController);
        }

    }
    //-----------------------------------------------------------------------
    void RibbonTrail::addNode(Node* n)
    {
        if (mNodeList.size() == mChainCount)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                mName + " cannot monitor any more nodes, chain count exceeded",
                "RibbonTrail::addNode");
        }
        if (n->getListener())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                mName + " cannot monitor node " + n->getName() + " since it already has a listener.",
                "RibbonTrail::addNode");
        }

        // get chain index
        size_t chainIndex = mFreeChains.back();
        mFreeChains.pop_back();
        mNodeToChainSegment.push_back(chainIndex);
        mNodeToSegMap[n] = chainIndex;

        // initialise the chain
        resetTrail(chainIndex, n);

        mNodeList.push_back(n);
        n->setListener(this);

    }
    //-----------------------------------------------------------------------
    size_t RibbonTrail::getChainIndexForNode(const Node* n)
    {
        NodeToChainSegmentMap::const_iterator i = mNodeToSegMap.find(n);
        if (i == mNodeToSegMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "This node is not being tracked", "RibbonTrail::getChainIndexForNode");
        }
        return i->second;
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::removeNode(const Node* n)
    {
        NodeList::iterator i = std::find(mNodeList.begin(), mNodeList.end(), n);
        if (i != mNodeList.end())
        {
            // also get matching chain segment
            size_t index = std::distance(mNodeList.begin(), i);
            IndexVector::iterator mi = mNodeToChainSegment.begin();
            std::advance(mi, index);
            size_t chainIndex = *mi;
            BillboardChain::clearChain(chainIndex);
            // mark as free now
            mFreeChains.push_back(chainIndex);
            (*i)->setListener(0);
            mNodeList.erase(i);
            mNodeToChainSegment.erase(mi);
            mNodeToSegMap.erase(mNodeToSegMap.find(n));

        }
    }
    //-----------------------------------------------------------------------
    RibbonTrail::NodeIterator 
    RibbonTrail::getNodeIterator(void) const
    {
        return NodeIterator(mNodeList.begin(), mNodeList.end());
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::setTrailLength(Real len)
    {
        mTrailLength = len;
        mElemLength = mTrailLength / mMaxElementsPerChain;
        mSquaredElemLength = mElemLength * mElemLength;
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::setMaxChainElements(size_t maxElements)
    {
        BillboardChain::setMaxChainElements(maxElements);
        mElemLength = mTrailLength / mMaxElementsPerChain;
        mSquaredElemLength = mElemLength * mElemLength;

        resetAllTrails();
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::setNumberOfChains(size_t numChains)
    {
        if (numChains < mNodeList.size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Can't shrink the number of chains less than number of tracking nodes",
                "RibbonTrail::setNumberOfChains");
        }

        size_t oldChains = getNumberOfChains();

        BillboardChain::setNumberOfChains(numChains);

        mInitialColour.resize(numChains, ColourValue::White);
        mDeltaColour.resize(numChains, ColourValue::ZERO);
        mInitialWidth.resize(numChains, 10);
        mDeltaWidth.resize(numChains, 0);

        if (oldChains > numChains)
        {
            // remove free chains
            for (IndexVector::iterator i = mFreeChains.begin(); i != mFreeChains.end();)
            {
                if (*i >= numChains)
                    i = mFreeChains.erase(i);
                else
                    ++i;
            }
        }
        else if (oldChains < numChains)
        {
            // add new chains, at front to preserve previous ordering (pop_back)
            for (size_t i = oldChains; i < numChains; ++i)
                mFreeChains.insert(mFreeChains.begin(), i);
        }
        resetAllTrails();
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::clearChain(size_t chainIndex)
    {
        BillboardChain::clearChain(chainIndex);

        // Reset if we are tracking for this chain
        IndexVector::iterator i = std::find(mNodeToChainSegment.begin(), mNodeToChainSegment.end(), chainIndex);
        if (i != mNodeToChainSegment.end())
        {
            size_t nodeIndex = std::distance(mNodeToChainSegment.begin(), i);
            resetTrail(*i, mNodeList[nodeIndex]);
        }
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::setInitialColour(size_t chainIndex, const ColourValue& col)
    {
        setInitialColour(chainIndex, col.r, col.g, col.b, col.a);
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::setInitialColour(size_t chainIndex, float r, float g, float b, float a)
    {
        if (chainIndex >= mChainCount)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "chainIndex out of bounds", "RibbonTrail::setInitialColour");
        }
        mInitialColour[chainIndex].r = r;
        mInitialColour[chainIndex].g = g;
        mInitialColour[chainIndex].b = b;
        mInitialColour[chainIndex].a = a;
    }
    //-----------------------------------------------------------------------
    const ColourValue& RibbonTrail::getInitialColour(size_t chainIndex) const
    {
        if (chainIndex >= mChainCount)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "chainIndex out of bounds", "RibbonTrail::getInitialColour");
        }
        return mInitialColour[chainIndex];
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::setInitialWidth(size_t chainIndex, Real width)
    {
        if (chainIndex >= mChainCount)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "chainIndex out of bounds", "RibbonTrail::setInitialWidth");
        }
        mInitialWidth[chainIndex] = width;
    }
    //-----------------------------------------------------------------------
    Real RibbonTrail::getInitialWidth(size_t chainIndex) const
    {
        if (chainIndex >= mChainCount)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "chainIndex out of bounds", "RibbonTrail::getInitialWidth");
        }
        return mInitialWidth[chainIndex];
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::setColourChange(size_t chainIndex, const ColourValue& valuePerSecond)
    {
        setColourChange(chainIndex, 
            valuePerSecond.r, valuePerSecond.g, valuePerSecond.b, valuePerSecond.a);
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::setColourChange(size_t chainIndex, float r, float g, float b, float a)
    {
        if (chainIndex >= mChainCount)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "chainIndex out of bounds", "RibbonTrail::setColourChange");
        }
        mDeltaColour[chainIndex].r = r;
        mDeltaColour[chainIndex].g = g;
        mDeltaColour[chainIndex].b = b;
        mDeltaColour[chainIndex].a = a;

        manageController();

    }
    //-----------------------------------------------------------------------
    const ColourValue& RibbonTrail::getColourChange(size_t chainIndex) const
    {
        if (chainIndex >= mChainCount)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "chainIndex out of bounds", "RibbonTrail::getColourChange");
        }
        return mDeltaColour[chainIndex];
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::setWidthChange(size_t chainIndex, Real widthDeltaPerSecond)
    {
        if (chainIndex >= mChainCount)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "chainIndex out of bounds", "RibbonTrail::setWidthChange");
        }
        mDeltaWidth[chainIndex] = widthDeltaPerSecond;
        manageController();
    }
    //-----------------------------------------------------------------------
    Real RibbonTrail::getWidthChange(size_t chainIndex) const
    {
        if (chainIndex >= mChainCount)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "chainIndex out of bounds", "RibbonTrail::getWidthChange");
        }
        return mDeltaWidth[chainIndex];

    }
    //-----------------------------------------------------------------------
    void RibbonTrail::manageController(void)
    {
        bool needController = false;
        for (size_t i = 0; i < mChainCount; ++i)
        {
            if (mDeltaWidth[i] != 0 || mDeltaColour[i] != ColourValue::ZERO)
            {
                needController = true;
                break;
            }
        }
        if (!mFadeController && needController)
        {
            // Set up fading via frame time controller
            ControllerManager& mgr = ControllerManager::getSingleton();
            mFadeController = mgr.createFrameTimePassthroughController(mTimeControllerValue);
        }
        else if (mFadeController && !needController)
        {
            // destroy controller
            ControllerManager::getSingleton().destroyController(mFadeController);
            mFadeController = 0;
        }

    }
    //-----------------------------------------------------------------------
    void RibbonTrail::nodeUpdated(const Node* node)
    {
        size_t chainIndex = getChainIndexForNode(node);
        updateTrail(chainIndex, node);
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::nodeDestroyed(const Node* node)
    {
        removeNode(node);

    }
    //-----------------------------------------------------------------------
    void RibbonTrail::updateTrail(size_t index, const Node* node)
    {
        // Repeat this entire process if chain is stretched beyond its natural length
        bool done = false;
        while (!done)
        {
            // Node has changed somehow, we're only interested in the derived position
            ChainSegment& seg = mChainSegmentList[index];
            Element& headElem = mChainElementList[seg.start + seg.head];
            size_t nextElemIdx = seg.head + 1;
            // wrap
            if (nextElemIdx == mMaxElementsPerChain)
                nextElemIdx = 0;
            Element& nextElem = mChainElementList[seg.start + nextElemIdx];

            // Vary the head elem, but bake new version if that exceeds element len
            Vector3 newPos = node->_getDerivedPosition();
            if (mParentNode)
            {
                // Transform position to ourself space
                newPos = mParentNode->convertWorldToLocalPosition(newPos);
            }
            Vector3 diff = newPos - nextElem.position;
            Real sqlen = diff.squaredLength();
            if (sqlen >= mSquaredElemLength)
            {
                // Move existing head to mElemLength
                Vector3 scaledDiff = diff * (mElemLength / Math::Sqrt(sqlen));
                headElem.position = nextElem.position + scaledDiff;
                // Add a new element to be the new head
                Element newElem( newPos, mInitialWidth[index], 0.0f,
                                 mInitialColour[index], node->_getDerivedOrientation() );
                addChainElement(index, newElem);
                // alter diff to represent new head size
                diff = newPos - headElem.position;
                // check whether another step is needed or not
                if (diff.squaredLength() <= mSquaredElemLength)   
                    done = true;

            }
            else
            {
                // Extend existing head
                headElem.position = newPos;
                done = true;
            }

            // Is this segment full?
            if ((seg.tail + 1) % mMaxElementsPerChain == seg.head)
            {
                // If so, shrink tail gradually to match head extension
                Element& tailElem = mChainElementList[seg.start + seg.tail];
                size_t preTailIdx;
                if (seg.tail == 0)
                    preTailIdx = mMaxElementsPerChain - 1;
                else
                    preTailIdx = seg.tail - 1;
                Element& preTailElem = mChainElementList[seg.start + preTailIdx];

                // Measure tail diff from pretail to tail
                Vector3 taildiff = tailElem.position - preTailElem.position;
                Real taillen = taildiff.length();
                if (taillen > 1e-06)
                {
                    Real tailsize = mElemLength - diff.length();
                    taildiff *= tailsize / taillen;
                    tailElem.position = preTailElem.position + taildiff;
                }

            }
        } // end while


        mBoundsDirty = true;
        // Need to dirty the parent node, but can't do it using needUpdate() here 
        // since we're in the middle of the scene graph update (node listener), 
        // so re-entrant calls don't work. Queue.
        if (mParentNode)
        {
            Node::queueNeedUpdate(getParentSceneNode());
        }

    }
    //-----------------------------------------------------------------------
    void RibbonTrail::_timeUpdate(Real time)
    {
        // Apply all segment effects
        for (size_t s = 0; s < mChainSegmentList.size(); ++s)
        {
            ChainSegment& seg = mChainSegmentList[s];
            if (seg.head != SEGMENT_EMPTY && seg.head != seg.tail)
            {
                
                for(size_t e = seg.head + 1;; ++e) // until break
                {
                    e = e % mMaxElementsPerChain;

                    Element& elem = mChainElementList[seg.start + e];
                    elem.width = elem.width - (time * mDeltaWidth[s]);
                    elem.width = std::max(Real(0.0f), elem.width);
                    elem.colour = elem.colour - (mDeltaColour[s] * time);
                    elem.colour.saturate();

                    if (e == seg.tail)
                        break;
                }
            }
        }
        mVertexContentDirty = true;
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::resetTrail(size_t index, const Node* node)
    {
        assert(index < mChainCount);

        ChainSegment& seg = mChainSegmentList[index];
        // set up this segment
        seg.head = seg.tail = SEGMENT_EMPTY;
        // Create new element, v coord is always 0.0f
        // need to convert to take parent node's position into account
        Vector3 position = node->_getDerivedPosition();
        if (mParentNode)
        {
            position = mParentNode->convertWorldToLocalPosition(position);
        }
        Element e(position,
            mInitialWidth[index], 0.0f, mInitialColour[index], node->_getDerivedOrientation());
        // Add the start position
        addChainElement(index, e);
        // Add another on the same spot, this will extend
        addChainElement(index, e);
    }
    //-----------------------------------------------------------------------
    void RibbonTrail::resetAllTrails(void)
    {
        for (size_t i = 0; i < mNodeList.size(); ++i)
        {
            resetTrail(i, mNodeList[i]);
        }
    }
    //-----------------------------------------------------------------------
    const String& RibbonTrail::getMovableType(void) const
    {
        return RibbonTrailFactory::FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String RibbonTrailFactory::FACTORY_TYPE_NAME = "RibbonTrail";
    //-----------------------------------------------------------------------
    const String& RibbonTrailFactory::getType(void) const
    {
        return FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    MovableObject* RibbonTrailFactory::createInstanceImpl( const String& name,
        const NameValuePairList* params)
    {
        size_t maxElements = 20;
        size_t numberOfChains = 1;
        bool useTex = true;
        bool useCol = true;
        // optional params
        if (params != 0)
        {
            NameValuePairList::const_iterator ni = params->find("maxElements");
            if (ni != params->end())
            {
                maxElements = StringConverter::parseSizeT(ni->second);
            }
            ni = params->find("numberOfChains");
            if (ni != params->end())
            {
                numberOfChains = StringConverter::parseSizeT(ni->second);
            }
            ni = params->find("useTextureCoords");
            if (ni != params->end())
            {
                useTex = StringConverter::parseBool(ni->second);
            }
            ni = params->find("useVertexColours");
            if (ni != params->end())
            {
                useCol = StringConverter::parseBool(ni->second);
            }

        }

        return OGRE_NEW RibbonTrail(name, maxElements, numberOfChains, useTex, useCol);

    }

}

