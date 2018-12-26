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
#include "OgrePatchSurface.h"

#define LEVEL_WIDTH(lvl) ((1 << (lvl+1)) + 1)

namespace Ogre {

    // TODO: make this deal with specular colours and more than 2 texture coords

    //-----------------------------------------------------------------------
    PatchSurface::PatchSurface()
    {
        mType = PST_BEZIER;
    }
    //-----------------------------------------------------------------------
    PatchSurface::~PatchSurface()
    {
    }
    //-----------------------------------------------------------------------
    void PatchSurface::defineSurface(void* controlPointBuffer, 
            VertexDeclaration *declaration, size_t width, size_t height,
            PatchSurfaceType pType, size_t uMaxSubdivisionLevel, 
            size_t vMaxSubdivisionLevel, VisibleSide visibleSide)
    {
        if (height == 0 || width == 0)
            return; // Do nothing - garbage

        mType = pType;
        mCtlWidth = width;
        mCtlHeight = height;
        mCtlCount = width * height;
        mControlPointBuffer = controlPointBuffer;
        mDeclaration = declaration;

        // Copy positions into Vector3 vector
        mVecCtlPoints.clear();
        const VertexElement* elem = declaration->findElementBySemantic(VES_POSITION);
        size_t vertSize = declaration->getVertexSize(0);
        uchar *pVert = static_cast<uchar*>(controlPointBuffer);
        float* pFloat;
        for (size_t i = 0; i < mCtlCount; ++i)
        {
            elem->baseVertexPointerToElement(pVert, &pFloat);
            mVecCtlPoints.push_back(Vector3(pFloat[0], pFloat[1], pFloat[2]));
            pVert += vertSize;
        }

        mVSide = visibleSide;

        // Determine max level
        // Initialise to 100% detail
        mSubdivisionFactor = 1.0f;
        if (uMaxSubdivisionLevel == (size_t)AUTO_LEVEL)
        {
            mULevel = mMaxULevel = getAutoULevel();
        }
        else
        {
            mULevel = mMaxULevel = uMaxSubdivisionLevel;
        }

        if (vMaxSubdivisionLevel == (size_t)AUTO_LEVEL)
        {
            mVLevel = mMaxVLevel = getAutoVLevel();
        }
        else
        {
            mVLevel = mMaxVLevel = vMaxSubdivisionLevel;
        }



        // Derive mesh width / height
        mMeshWidth  = (LEVEL_WIDTH(mMaxULevel)-1) * ((mCtlWidth-1)/2) + 1;
        mMeshHeight = (LEVEL_WIDTH(mMaxVLevel)-1) * ((mCtlHeight-1)/2) + 1;


        // Calculate number of required vertices / indexes at max resolution
        mRequiredVertexCount = mMeshWidth * mMeshHeight;
        int iterations = (mVSide == VS_BOTH)? 2 : 1;
        mRequiredIndexCount = (mMeshWidth-1) * (mMeshHeight-1) * 2 * iterations * 3;

        // Calculate bounds based on control points
        std::vector<Vector3>::const_iterator ctli;
        Vector3 min = Vector3::ZERO, max = Vector3::UNIT_SCALE;
        Real maxSqRadius = 0;
        bool first = true;
        for (ctli = mVecCtlPoints.begin(); ctli != mVecCtlPoints.end(); ++ctli)
        {
            if (first)
            {
                min = max = *ctli;
                maxSqRadius = ctli->squaredLength();
                first = false;
            }
            else
            {
                min.makeFloor(*ctli);
                max.makeCeil(*ctli);
                maxSqRadius = std::max(ctli->squaredLength(), maxSqRadius);

            }
        }
        mAABB.setExtents(min, max);
        mBoundingSphere = Math::Sqrt(maxSqRadius);

    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& PatchSurface::getBounds(void) const
    {
        return mAABB;
    }
    //-----------------------------------------------------------------------
    Real PatchSurface::getBoundingSphereRadius(void) const
    {
        return mBoundingSphere;
    }
    //-----------------------------------------------------------------------
    size_t PatchSurface::getRequiredVertexCount(void) const
    {
        return mRequiredVertexCount;
    }
    //-----------------------------------------------------------------------
    size_t PatchSurface::getRequiredIndexCount(void) const
    {
        return mRequiredIndexCount;
    }
    //-----------------------------------------------------------------------
    void PatchSurface::build(HardwareVertexBufferSharedPtr destVertexBuffer, 
        size_t vertexStart, HardwareIndexBufferSharedPtr destIndexBuffer, size_t indexStart)
    {

        if (mVecCtlPoints.empty())
            return;

        mVertexBuffer = destVertexBuffer;
        mVertexOffset = vertexStart;
        mIndexBuffer = destIndexBuffer;
        mIndexOffset = indexStart;

        // Lock just the region we are interested in 
        HardwareBufferLockGuard vertexLock(mVertexBuffer,
            mVertexOffset * mDeclaration->getVertexSize(0), 
            mRequiredVertexCount * mDeclaration->getVertexSize(0),
            HardwareBuffer::HBL_NO_OVERWRITE);

        distributeControlPoints(vertexLock.pData);

        // Subdivide the curve to the MAX :)
        // Do u direction first, so need to step over v levels not done yet
        size_t vStep = 1 << mMaxVLevel;
        size_t uStep = 1 << mMaxULevel;

        size_t v, u;
        for (v = 0; v < mMeshHeight; v += vStep)
        {
            // subdivide this row in u
            subdivideCurve(vertexLock.pData, v*mMeshWidth, uStep, mMeshWidth / uStep, mULevel);
        }

        // Now subdivide in v direction, this time all the u direction points are there so no step
        for (u = 0; u < mMeshWidth; ++u)
        {
            subdivideCurve(vertexLock.pData, u, vStep*mMeshWidth, mMeshHeight / vStep, mVLevel);
        }
        

        vertexLock.unlock();

        // Make triangles from mesh at this current level of detail
        makeTriangles();

    }
    //-----------------------------------------------------------------------
    size_t PatchSurface::getAutoULevel(bool forMax)
    {
        // determine levels
        // Derived from work by Bart Sekura in Rogl
        Vector3 a,b,c;
        size_t u,v;
        bool found=false;
        // Find u level
        for(v = 0; v < mCtlHeight; v++) {
            for(u = 0; u < mCtlWidth-1; u += 2) {
                a = mVecCtlPoints[v * mCtlWidth + u];
                b = mVecCtlPoints[v * mCtlWidth + u+1];
                c = mVecCtlPoints[v * mCtlWidth + u+2];
                if(a!=c) {
                    found=true;
                    break;
                }
            }
            if(found) break;
        }
        if(!found) {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Can't find suitable control points for determining U subdivision level",
                "PatchSurface::getAutoULevel");
        }

        return findLevel(a,b,c);

    }
    //-----------------------------------------------------------------------
    size_t PatchSurface::getAutoVLevel(bool forMax)
    {
        Vector3 a,b,c;
        size_t u,v;
        bool found=false;
        for(u = 0; u < mCtlWidth; u++) {
            for(v = 0; v < mCtlHeight-1; v += 2) {
                a = mVecCtlPoints[v * mCtlWidth + u];
                b = mVecCtlPoints[(v+1) * mCtlWidth + u];
                c = mVecCtlPoints[(v+2) * mCtlWidth + u];
                if(a!=c) {
                    found=true;
                    break;
                }
            }
            if(found) break;
        }
        if(!found) {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Can't find suitable control points for determining V subdivision level",
                "PatchSurface::getAutoVLevel");
        }

        return findLevel(a,b,c);

    }
    //-----------------------------------------------------------------------
    void PatchSurface::setSubdivisionFactor(Real factor)
    {
        assert(factor >= 0.0f && factor <= 1.0f);

        mSubdivisionFactor = factor;
        mULevel = static_cast<size_t>(factor * mMaxULevel);
        mVLevel = static_cast<size_t>(factor * mMaxVLevel);

        makeTriangles();


    }
    //-----------------------------------------------------------------------
    Real PatchSurface::getSubdivisionFactor(void) const
    {
        return mSubdivisionFactor;
    }
    //-----------------------------------------------------------------------
    size_t PatchSurface::getCurrentIndexCount(void) const
    {
        return mCurrIndexCount;
    }
    //-----------------------------------------------------------------------
    size_t PatchSurface::findLevel(Vector3& a, Vector3& b, Vector3& c)
    {
        // Derived from work by Bart Sekura in rogl
        // Apart from I think I fixed a bug - see below
        // I also commented the code, the only thing wrong with rogl is almost no comments!!

        const size_t max_levels = 5;
        const float subdiv = 10;
        size_t level;

        float test=subdiv*subdiv;
        Vector3 s,t,d;
        for(level=0; level<max_levels-1; level++)
        {
            // Subdivide the 2 lines
            s = a.midPoint(b);
            t = b.midPoint(c);
            // Find the midpoint between the 2 midpoints
            c = s.midPoint(t);
            // Get the vector between this subdivided midpoint and the middle point of the original line
            d = c - b;
            // Find the squared length, and break when small enough
            if(d.dotProduct(d) < test) {
                break;
            }
            b=a; 
        }

        return level;

    }

    /*
    //-----------------------------------------------------------------------
    void PatchSurface::allocateMemory(void)
    {
        if (mMemoryAllocated)
            deallocateMemory();

        // Allocate to the size of max level

        // Create mesh
        mMesh = MeshManager::getSingleton().createManual(mMeshName);
        mMesh->sharedVertexData = OGRE_NEW VertexData();
        // Copy all vertex parameters
        mMesh->sharedVertexData->vertexStart = 0;
        // Vertex count will be set on build() because it depends on current level
        // NB clone the declaration because Mesh's VertexData will destroy it
        mMesh->sharedVertexData->vertexDeclaration = mDeclaration->clone();
        // Create buffer (only a single buffer)
        // Allocate enough buffer memory for maximum subdivision, not current subdivision
        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton().
            createVertexBuffer(
                mDeclaration->getVertexSize(0), 
                mMaxMeshHeight * mMaxMeshWidth, // maximum size 
                HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY); // dynamic for changing level

        // Set binding
        mMesh->sharedVertexData->vertexBufferBinding->setBinding(0, vbuf);

        SubMesh* sm = mMesh->createSubMesh();
        // Allocate enough index data for max subdivision
        sm->indexData->indexStart = 0;
        // Index count will be set on build()
        unsigned short iterations = (mVSide == VS_BOTH ? 2 : 1);
        sm->indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT, 
            (mMaxMeshWidth-1) * (mMaxMeshHeight-1) * 2 * iterations * 3,  
            HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY);

        mMesh->load();

        // Derive bounds from control points, cannot stray outside that
        Vector3 min, max;
        Real maxSquaredRadius;
        bool first = true;
        std::vector<Vector3>::iterator i, iend;
        iend = mVecCtlPoints.end();
        for (i = mVecCtlPoints.begin(); i != iend; ++i)
        {
            if (first)
            {
                min = max = *i;
                maxSquaredRadius = i->squaredLength();
            }
            else
            {
                min.makeFloor(*i);
                max.makeCeil(*i);
                maxSquaredRadius = std::max(maxSquaredRadius, i->squaredLength());
            }

        }
        mMesh->_setBounds(AxisAlignedBox(min, max));
        mMesh->_setBoundingSphereRadius(Math::Sqrt(maxSquaredRadius));



    }
    */
    //-----------------------------------------------------------------------
    void PatchSurface::distributeControlPoints(void* lockedBuffer)
    {
        // Insert original control points into expanded mesh
        size_t uStep = 1 << mULevel;
        size_t vStep = 1 << mVLevel;

        void* pSrc = mControlPointBuffer;
        size_t vertexSize = mDeclaration->getVertexSize(0);
        float *pSrcReal, *pDestReal;
        RGBA *pSrcRGBA, *pDestRGBA;
        const VertexElement* elemPos = mDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* elemNorm = mDeclaration->findElementBySemantic(VES_NORMAL);
        const VertexElement* elemTex0 = mDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES, 0);
        const VertexElement* elemTex1 = mDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES, 1);
        const VertexElement* elemDiffuse = mDeclaration->findElementBySemantic(VES_DIFFUSE);
        for (size_t v = 0; v < mMeshHeight; v += vStep)
        {
            // set dest by v from base
            void* pDest = static_cast<void*>(
                static_cast<unsigned char*>(lockedBuffer) + (vertexSize * mMeshWidth * v));
            for (size_t u = 0; u < mMeshWidth; u += uStep)
            {

                // Copy Position
                elemPos->baseVertexPointerToElement(pSrc, &pSrcReal);
                elemPos->baseVertexPointerToElement(pDest, &pDestReal);
                *pDestReal++ = *pSrcReal++;
                *pDestReal++ = *pSrcReal++;
                *pDestReal++ = *pSrcReal++;

                // Copy Normals
                if (elemNorm)
                {
                    elemNorm->baseVertexPointerToElement(pSrc, &pSrcReal);
                    elemNorm->baseVertexPointerToElement(pDest, &pDestReal);
                    *pDestReal++ = *pSrcReal++;
                    *pDestReal++ = *pSrcReal++;
                    *pDestReal++ = *pSrcReal++;
                }

                // Copy Diffuse
                if (elemDiffuse)
                {
                    elemDiffuse->baseVertexPointerToElement(pSrc, &pSrcRGBA);
                    elemDiffuse->baseVertexPointerToElement(pDest, &pDestRGBA);
                    *pDestRGBA++ = *pSrcRGBA++;
                }

                // Copy texture coords
                if (elemTex0)
                {
                    elemTex0->baseVertexPointerToElement(pSrc, &pSrcReal);
                    elemTex0->baseVertexPointerToElement(pDest, &pDestReal);
                    for (size_t dim = 0; dim < VertexElement::getTypeCount(elemTex0->getType()); ++dim)
                        *pDestReal++ = *pSrcReal++;
                }
                if (elemTex1)
                {
                    elemTex1->baseVertexPointerToElement(pSrc, &pSrcReal);
                    elemTex1->baseVertexPointerToElement(pDest, &pDestReal);
                    for (size_t dim = 0; dim < VertexElement::getTypeCount(elemTex1->getType()); ++dim)
                        *pDestReal++ = *pSrcReal++;
                }

                // Increment source by one vertex
                pSrc = static_cast<void*>(
                    static_cast<unsigned char*>(pSrc) + vertexSize);
                // Increment dest by 1 vertex * uStep
                pDest = static_cast<void*>(
                    static_cast<unsigned char*>(pDest) + (vertexSize * uStep));
            } // u
        } // v

       
    }
    //-----------------------------------------------------------------------
    void PatchSurface::subdivideCurve(void* lockedBuffer, size_t startIdx, size_t stepSize, size_t numSteps, size_t iterations)
    {
        // Subdivides a curve within a sparsely populated buffer (gaps are already there to be interpolated into)
        size_t maxIdx;

        maxIdx = startIdx + (numSteps * stepSize);
        size_t step = stepSize;

        while(iterations--)
        {
            size_t leftIdx, rightIdx, destIdx, halfStep;
            halfStep = step / 2;
            leftIdx = startIdx;
            destIdx = leftIdx + halfStep;
            rightIdx = leftIdx + step;
            bool firstSegment = true;
            while (leftIdx < maxIdx)
            {
                // Interpolate
                interpolateVertexData(lockedBuffer, leftIdx, rightIdx, destIdx);

                // If 2nd or more segment, interpolate current left between current and last mid points
                if (!firstSegment)
                {
                    interpolateVertexData(lockedBuffer, leftIdx - halfStep, leftIdx + halfStep, leftIdx);
                }
                // Next segment
                leftIdx = rightIdx;
                destIdx = leftIdx + halfStep;
                rightIdx = leftIdx + step;
                firstSegment = false;
            }

            step = halfStep;
        }
    }
    //-----------------------------------------------------------------------
    void PatchSurface::makeTriangles(void)
    {
        // Our vertex buffer is subdivided to the highest level, we need to generate tris
        // which step over the vertices we don't need for this level of detail.

        // Calculate steps
        int vStep = 1 << (mMaxVLevel - mVLevel);
        int uStep = 1 << (mMaxULevel - mULevel);
        size_t currWidth = (LEVEL_WIDTH(mULevel)-1) * ((mCtlWidth-1)/2) + 1;
        size_t currHeight = (LEVEL_WIDTH(mVLevel)-1) * ((mCtlHeight-1)/2) + 1;

        bool use32bitindexes = (mIndexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);

        // The mesh is built, just make a list of indexes to spit out the triangles
        int vInc;
        
        size_t uCount, v, iterations;

        if (mVSide == VS_BOTH)
        {
            iterations = 2;
            vInc = vStep;
            v = 0; // Start with front
        }
        else
        {
            iterations = 1;
            if (mVSide == VS_FRONT)
            {
                vInc = vStep;
                v = 0;
            }
            else
            {
                vInc = -vStep;
                v = mMeshHeight - 1;
            }
        }

        // Calc num indexes
        mCurrIndexCount = (currWidth - 1) * (currHeight - 1) * 6 * iterations;

        size_t v1, v2, v3;
        // Lock just the section of the buffer we need
        HardwareBufferLockGuard indexLock(mIndexBuffer,
                                          mIndexOffset * (use32bitindexes ? 4 : 2),
                                          mRequiredIndexCount * (use32bitindexes ? 4 : 2),
                                          HardwareBuffer::HBL_NO_OVERWRITE);
        unsigned short* p16 = static_cast<unsigned short*>(indexLock.pData);
        unsigned int* p32 = static_cast<unsigned int*>(indexLock.pData);

        while (iterations--)
        {
            // Make tris in a zigzag pattern (compatible with strips)
            size_t u = 0;
            int uInc = uStep; // Start with moving +u

            size_t vCount = currHeight - 1;
            while (vCount--)
            {
                uCount = currWidth - 1;
                while (uCount--)
                {
                    // First Tri in cell
                    // -----------------
                    v1 = ((v + vInc) * mMeshWidth) + u;
                    v2 = (v * mMeshWidth) + u;
                    v3 = ((v + vInc) * mMeshWidth) + (u + uInc);
                    // Output indexes
                    if (use32bitindexes)
                    {
                        *p32++ = static_cast<unsigned int>(v1);
                        *p32++ = static_cast<unsigned int>(v2);
                        *p32++ = static_cast<unsigned int>(v3);
                    }
                    else
                    {
                        *p16++ = static_cast<unsigned short>(v1);
                        *p16++ = static_cast<unsigned short>(v2);
                        *p16++ = static_cast<unsigned short>(v3);
                    }
                    // Second Tri in cell
                    // ------------------
                    v1 = ((v + vInc) * mMeshWidth) + (u + uInc);
                    v2 = (v * mMeshWidth) + u;
                    v3 = (v * mMeshWidth) + (u + uInc);
                    // Output indexes
                    if (use32bitindexes)
                    {
                        *p32++ = static_cast<unsigned int>(v1);
                        *p32++ = static_cast<unsigned int>(v2);
                        *p32++ = static_cast<unsigned int>(v3);
                    }
                    else
                    {
                        *p16++ = static_cast<unsigned short>(v1);
                        *p16++ = static_cast<unsigned short>(v2);
                        *p16++ = static_cast<unsigned short>(v3);
                    }

                    // Next column
                    u += uInc;
                }
                // Next row
                v += vInc;
                u = 0;


            }

            // Reverse vInc for double sided
            v = mMeshHeight - 1;
            vInc = -vInc;

        }
    }
    //-----------------------------------------------------------------------
    void PatchSurface::interpolateVertexData(void* lockedBuffer, size_t leftIdx, size_t rightIdx, size_t destIdx)
    {
        size_t vertexSize = mDeclaration->getVertexSize(0);
        const VertexElement* elemPos = mDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* elemNorm = mDeclaration->findElementBySemantic(VES_NORMAL);
        const VertexElement* elemDiffuse = mDeclaration->findElementBySemantic(VES_DIFFUSE);
        const VertexElement* elemTex0 = mDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES, 0);
        const VertexElement* elemTex1 = mDeclaration->findElementBySemantic(VES_TEXTURE_COORDINATES, 1);

        float *pDestReal, *pLeftReal, *pRightReal;
        unsigned char *pDestChar, *pLeftChar, *pRightChar;
        unsigned char *pDest, *pLeft, *pRight;

        // Set up pointers & interpolate
        pDest = static_cast<unsigned char*>(lockedBuffer) + (vertexSize * destIdx);
        pLeft = static_cast<unsigned char*>(lockedBuffer) + (vertexSize * leftIdx);
        pRight = static_cast<unsigned char*>(lockedBuffer) + (vertexSize * rightIdx);

        // Position
        elemPos->baseVertexPointerToElement(pDest, &pDestReal);
        elemPos->baseVertexPointerToElement(pLeft, &pLeftReal);
        elemPos->baseVertexPointerToElement(pRight, &pRightReal);

        *pDestReal++ = (*pLeftReal++ + *pRightReal++) * 0.5f;
        *pDestReal++ = (*pLeftReal++ + *pRightReal++) * 0.5f;
        *pDestReal++ = (*pLeftReal++ + *pRightReal++) * 0.5f;

        if (elemNorm)
        {
            elemNorm->baseVertexPointerToElement(pDest, &pDestReal);
            elemNorm->baseVertexPointerToElement(pLeft, &pLeftReal);
            elemNorm->baseVertexPointerToElement(pRight, &pRightReal);
            Vector3 norm;
            norm.x = (*pLeftReal++ + *pRightReal++) * 0.5f;
            norm.y = (*pLeftReal++ + *pRightReal++) * 0.5f;
            norm.z = (*pLeftReal++ + *pRightReal++) * 0.5f;
            norm.normalise();

            *pDestReal++ = norm.x;
            *pDestReal++ = norm.y;
            *pDestReal++ = norm.z;
        }
        if (elemDiffuse)
        {
            // Blend each byte individually
            elemDiffuse->baseVertexPointerToElement(pDest, &pDestChar);
            elemDiffuse->baseVertexPointerToElement(pLeft, &pLeftChar);
            elemDiffuse->baseVertexPointerToElement(pRight, &pRightChar);
            // 4 bytes to RGBA
            *pDestChar++ = static_cast<unsigned char>(((*pLeftChar++) + (*pRightChar++)) * 0.5);
            *pDestChar++ = static_cast<unsigned char>(((*pLeftChar++) + (*pRightChar++)) * 0.5);
            *pDestChar++ = static_cast<unsigned char>(((*pLeftChar++) + (*pRightChar++)) * 0.5);
            *pDestChar++ = static_cast<unsigned char>(((*pLeftChar++) + (*pRightChar++)) * 0.5);
        }
        if (elemTex0)
        {
            elemTex0->baseVertexPointerToElement(pDest, &pDestReal);
            elemTex0->baseVertexPointerToElement(pLeft, &pLeftReal);
            elemTex0->baseVertexPointerToElement(pRight, &pRightReal);

            for (size_t dim = 0; dim < VertexElement::getTypeCount(elemTex0->getType()); ++dim)
                *pDestReal++ = ((*pLeftReal++) + (*pRightReal++)) * 0.5f;
        }
        if (elemTex1)
        {
            elemTex1->baseVertexPointerToElement(pDest, &pDestReal);
            elemTex1->baseVertexPointerToElement(pLeft, &pLeftReal);
            elemTex1->baseVertexPointerToElement(pRight, &pRightReal);

            for (size_t dim = 0; dim < VertexElement::getTypeCount(elemTex1->getType()); ++dim)
                *pDestReal++ = ((*pLeftReal++) + (*pRightReal++)) * 0.5f;
        }
    }

}

