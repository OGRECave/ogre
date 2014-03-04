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
#include "OgreGrid3DPageStrategy.h"

#include "OgreStreamSerialiser.h"
#include "OgreCamera.h"
#include "OgrePagedWorldSection.h"
#include "OgrePage.h"
#include "OgreSceneNode.h"
#include "OgreSceneManager.h"
#include "OgreMaterialManager.h"
#include "OgreManualObject.h"
#include "OgrePageManager.h"
#include "OgreTechnique.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    const uint32 Grid3DPageStrategyData::CHUNK_ID = StreamSerialiser::makeIdentifier("G3DD");
    const uint16 Grid3DPageStrategyData::CHUNK_VERSION = 1;
    //---------------------------------------------------------------------
    Grid3DPageStrategyData::Grid3DPageStrategyData()
        : PageStrategyData()
        , mWorldOrigin(Vector3::ZERO)
        , mOrigin(Vector3::ZERO)
        , mCellSize(1000,1000,1000)
        , mLoadRadius(2000)
        , mHoldRadius(3000)
        , mMinCellX(-512)
        , mMinCellY(-512)
        , mMinCellZ(-512)
        , mMaxCellX(511)
        , mMaxCellY(511)
        , mMaxCellZ(511)
    {
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setCellRange(int32 minX, int32 minY, int32 minZ, int32 maxX, int32 maxY, int32 maxZ)
    {
        mMinCellX = minX;
        mMinCellY = minY;
        mMinCellZ = minZ;
        mMaxCellX = maxX;
        mMaxCellY = maxY;
        mMaxCellZ = maxZ;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setCellRangeMinX(int32 minX)
    {
        mMinCellX = minX;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setCellRangeMinY(int32 minY)
    {
        mMinCellY = minY;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setCellRangeMinZ(int32 minZ)
    {
        mMinCellZ = minZ;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setCellRangeMaxX(int32 maxX)
    {
        mMaxCellX = maxX;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setCellRangeMaxY(int32 maxY)
    {
        mMaxCellY = maxY;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setCellRangeMaxZ(int32 maxZ)
    {
        mMaxCellZ = maxZ;
    }
    //---------------------------------------------------------------------
    Grid3DPageStrategyData::~Grid3DPageStrategyData()
    {
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setOrigin(const Vector3& origin)
    {
        mWorldOrigin = origin;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::determineGridLocation(const Vector3& pos, int32 *x, int32 *y, int32 *z)
    {
        Vector3 relPos = pos - mOrigin + mCellSize * 0.5f;

        *x = static_cast<int32>(floor(relPos.x / mCellSize.x));
        *y = static_cast<int32>(floor(relPos.y / mCellSize.y));
        *z = static_cast<int32>(floor(relPos.z / mCellSize.z));
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::getBottomLeftGridSpace(int32 x, int32 y, int32 z, Vector3& bl)
    {
        bl.x = mOrigin.x + (x-0.5f) * mCellSize.x;
        bl.y = mOrigin.y + (y-0.5f) * mCellSize.y;
        bl.z = mOrigin.z + (z-0.5f) * mCellSize.z;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::getMidPointGridSpace(int32 x, int32 y, int32 z, Vector3& mid)
    {
        mid.x = mOrigin.x + float(x) * mCellSize.x;
        mid.y = mOrigin.y + float(y) * mCellSize.y;
        mid.z = mOrigin.z + float(z) * mCellSize.z;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::getCornersGridSpace(int32 x, int32 y, int32 z, Vector3* pEightPoints)
    {
        getBottomLeftGridSpace(x, y, z, pEightPoints[0]);
        pEightPoints[1] = pEightPoints[0] + Vector3(mCellSize.x, 0,           0);
        pEightPoints[2] = pEightPoints[0] + Vector3(mCellSize.x, mCellSize.y, 0);
        pEightPoints[3] = pEightPoints[0] + Vector3(0,           mCellSize.y, 0);
        pEightPoints[4] = pEightPoints[0] + Vector3(0, 0, mCellSize.z);
        pEightPoints[5] = pEightPoints[1] + Vector3(0, 0, mCellSize.z);
        pEightPoints[6] = pEightPoints[2] + Vector3(0, 0, mCellSize.z);
        pEightPoints[7] = pEightPoints[3] + Vector3(0, 0, mCellSize.z);
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setCellSize(const Vector3& sz)
    {
        mCellSize = sz;
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setLoadRadius(Real sz)
    {
        mLoadRadius = sz;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::setHoldRadius(Real sz)
    {
        mHoldRadius = sz;
    }
    //---------------------------------------------------------------------
    PageID Grid3DPageStrategyData::calculatePageID(int32 x, int32 y, int32 z)
    {
        // Go into positive range (simpler on decode)
        x -= mMinCellX;
        y -= mMinCellY;
        z -= mMinCellZ;
        // Push index bits into place
        uint32 key = z & 1023;
        key = (key<<10) | (y&1023);
        key = (key<<10) | (x&1023);
        return (PageID)key;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::calculateCell(PageID inPageID, int32 *x, int32 *y, int32* z)
    {
        // Pop bits from page index:
        int32 ox = inPageID & 1023; inPageID >>= 10;
        int32 oy = inPageID & 1023; inPageID >>= 10;
        int32 oz = inPageID & 1023;
        //// Hand made sign extension from 10bits to 32:
        //NOT NEEDED ANYMORE because this is crafted positive...
        //if( ox > 511 ) ox = 1024 - ox;
        //if( oy > 511 ) oy = 1024 - oy;
        //if( oz > 511 ) oz = 1024 - oz;
        // Back in the [min..max] range:
        *x = ox + mMinCellX;
        *y = oy + mMinCellY;
        *z = oz + mMinCellZ;
    }
    //---------------------------------------------------------------------
    bool Grid3DPageStrategyData::load(StreamSerialiser& ser)
    {
        if (!ser.readChunkBegin(CHUNK_ID, CHUNK_VERSION, "Grid3DPageStrategyData"))
            return false;

        ser.read(&mOrigin);
        ser.read(&mCellSize);
        ser.read(&mLoadRadius);
        ser.read(&mHoldRadius);
        ser.read(&mMinCellX);
        ser.read(&mMaxCellX);
        ser.read(&mMinCellY);
        ser.read(&mMaxCellY);
        ser.read(&mMinCellZ);
        ser.read(&mMaxCellZ);

        ser.readChunkEnd(CHUNK_ID);

        return true;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategyData::save(StreamSerialiser& ser)
    {
        ser.writeChunkBegin(CHUNK_ID, CHUNK_VERSION);

        ser.write(&mWorldOrigin);
        ser.write(&mCellSize);
        ser.write(&mLoadRadius);
        ser.write(&mHoldRadius);
        ser.write(&mMinCellX);
        ser.write(&mMaxCellX);
        ser.write(&mMinCellY);
        ser.write(&mMaxCellY);
        ser.write(&mMinCellZ);
        ser.write(&mMaxCellZ);

        ser.writeChunkEnd(CHUNK_ID);
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    Grid3DPageStrategy::Grid3DPageStrategy(PageManager* manager)
        : PageStrategy("Grid3D", manager)
    {

    }
    //---------------------------------------------------------------------
    Grid3DPageStrategy::~Grid3DPageStrategy()
    {

    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategy::notifyCamera(Camera* cam, PagedWorldSection* section)
    {
        Grid3DPageStrategyData* stratData = static_cast<Grid3DPageStrategyData*>(section->getStrategyData());

        const Vector3& pos = cam->getDerivedPosition();
        int32 x, y, z;
        stratData->determineGridLocation(pos, &x, &y, &z);

        Real loadRadius = stratData->getLoadRadius();
        Real holdRadius = stratData->getHoldRadius();
        // scan the whole Hold range
        Real fxmin = (Real)x - holdRadius/stratData->getCellSize().x;
        Real fxmax = (Real)x + holdRadius/stratData->getCellSize().x;
        Real fymin = (Real)y - holdRadius/stratData->getCellSize().y;
        Real fymax = (Real)y + holdRadius/stratData->getCellSize().y;
        Real fzmin = (Real)z - holdRadius/stratData->getCellSize().z;
        Real fzmax = (Real)z + holdRadius/stratData->getCellSize().z;

        int32 xmin = stratData->getCellRangeMinX();
        int32 xmax = stratData->getCellRangeMaxX();
        int32 ymin = stratData->getCellRangeMinY();
        int32 ymax = stratData->getCellRangeMaxY();
        int32 zmin = stratData->getCellRangeMinZ();
        int32 zmax = stratData->getCellRangeMaxZ();

        // Round UP max, round DOWN min
        xmin = fxmin < xmin ? xmin : (int32)floor(fxmin);
        xmax = fxmax > xmax ? xmax : (int32)ceil(fxmax);
        ymin = fymin < ymin ? ymin : (int32)floor(fymin);
        ymax = fymax > ymax ? ymax : (int32)ceil(fymax);
        zmin = fzmin < zmin ? zmin : (int32)floor(fzmin);
        zmax = fzmax > zmax ? zmax : (int32)ceil(fzmax);
        // the inner, active load range
        fxmin = (Real)x - loadRadius/stratData->getCellSize().x;
        fxmax = (Real)x + loadRadius/stratData->getCellSize().x;
        fymin = (Real)y - loadRadius/stratData->getCellSize().y;
        fymax = (Real)y + loadRadius/stratData->getCellSize().y;
        fzmin = (Real)z - loadRadius/stratData->getCellSize().z;
        fzmax = (Real)z + loadRadius/stratData->getCellSize().z;
        // Round UP max, round DOWN min
        int32 loadxmin = fxmin < xmin ? xmin : (int32)floor(fxmin);
        int32 loadxmax = fxmax > xmax ? xmax : (int32)ceil(fxmax);
        int32 loadymin = fymin < ymin ? ymin : (int32)floor(fymin);
        int32 loadymax = fymax > ymax ? ymax : (int32)ceil(fymax);
        int32 loadzmin = fzmin < zmin ? zmin : (int32)floor(fzmin);
        int32 loadzmax = fzmax > zmax ? zmax : (int32)ceil(fzmax);

        for (int32 cz = zmin; cz <= zmax; ++cz)
        {
            for (int32 cy = ymin; cy <= ymax; ++cy)
            {
                for (int32 cx = xmin; cx <= xmax; ++cx)
                {
                    PageID pageID = stratData->calculatePageID(cx, cy, cz);

                    if (cx >= loadxmin && cx <= loadxmax 
                     && cy >= loadymin && cy <= loadymax
                     && cz >= loadzmin && cz <= loadzmax )
                    {
                        Vector3 bl;
                        stratData->getBottomLeftGridSpace(cx, cy, cz, bl);
                        Ogre::AxisAlignedBox bbox(bl, bl+stratData->getCellSize());

                        if( cam->isVisible(bbox) )
                            section->loadPage(pageID);
                        else
                            section->holdPage(pageID);
                    }
                    else
                    {
                        // in the outer 'hold' range, keep it but don't actively load
                        section->holdPage(pageID);
                    }
                    // other pages will by inference be marked for unloading
                }
            }
        }
    }
    //---------------------------------------------------------------------
    PageStrategyData* Grid3DPageStrategy::createData()
    {
        return OGRE_NEW Grid3DPageStrategyData();
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategy::destroyData(PageStrategyData* d)
    {
        OGRE_DELETE d;
    }
    //---------------------------------------------------------------------
    void Grid3DPageStrategy::updateDebugDisplay(Page* p, SceneNode* sn)
    {
        uint8 dbglvl = mManager->getDebugDisplayLevel();
        if (dbglvl)
        {
            // we could try to avoid updating the geometry every time here, but this 
            // wouldn't easily deal with paging parameter changes. There shouldn't 
            // be that many pages anyway, and this is debug after all, so update every time
            int32 x, y, z;
            Grid3DPageStrategyData* stratData = static_cast<Grid3DPageStrategyData*>(
                p->getParentSection()->getStrategyData());
            stratData->calculateCell(p->getID(), &x, &y, &z);

            Grid3DPageStrategyData* data = static_cast<Grid3DPageStrategyData*>(p->getParentSection()->getStrategyData());

            // Determine our centre point, we'll anchor here
            Vector3 midPoint;
            data->getMidPointGridSpace(x, y, z, midPoint);
            sn->setPosition(midPoint);

            //--- Get coordinates, relative to midPoint :
            Vector3 corners[8];
            Vector3 hSize = 0.500f * stratData->getCellSize();
            for (int i = 0; i < 8; ++i)
            {
                corners[i].x = i&1 ? hSize.x : -hSize.x;
                corners[i].z = i&2 ? hSize.y : -hSize.y;
                corners[i].y = i&4 ? hSize.z : -hSize.z;
            }

            //--- Get a material
            String matName = "Ogre/G3D/Debug";
            MaterialPtr mat = MaterialManager::getSingleton().getByName(matName);
            if (mat.isNull())
            {
                mat = MaterialManager::getSingleton().create(matName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                Pass* pass = mat->getTechnique(0)->getPass(0);
                pass->setLightingEnabled(false);
                pass->setVertexColourTracking(TVC_AMBIENT);
                pass->setDepthWriteEnabled(false);
                mat->load();
            }

            ManualObject* mo = 0;
            bool update = sn->numAttachedObjects() > 0;
            if( update )
            {
                mo = static_cast<ManualObject*>(sn->getAttachedObject(0));
                mo->beginUpdate(0);
            }
            else
            {
                mo = p->getParentSection()->getSceneManager()->createManualObject();
                mo->begin(matName, RenderOperation::OT_LINE_STRIP);
            }

            ColourValue vcol = ColourValue::Green;
            mo->position(corners[0]); mo->colour(vcol); // First Square...
            mo->position(corners[1]); mo->colour(vcol);
            mo->position(corners[3]); mo->colour(vcol);
            mo->position(corners[2]); mo->colour(vcol);
            mo->position(corners[0]); mo->colour(vcol);
            mo->position(corners[4]); mo->colour(vcol); // 0-4 link + second square and 
            mo->position(corners[5]); mo->colour(vcol);
            mo->position(corners[7]); mo->colour(vcol);
            mo->position(corners[6]); mo->colour(vcol);
            mo->position(corners[4]); mo->colour(vcol);
            mo->position(corners[5]); mo->colour(vcol); // 5-1 link
            mo->position(corners[1]); mo->colour(vcol);
            mo->position(corners[3]); mo->colour(vcol); // 3-7 link
            mo->position(corners[7]); mo->colour(vcol);
            mo->position(corners[6]); mo->colour(vcol); // 6-2 link
            mo->position(corners[2]); mo->colour(vcol);
            mo->end();

            if(!update)
                sn->attachObject(mo);
        }
    }

    //---------------------------------------------------------------------
    PageID Grid3DPageStrategy::getPageID(const Vector3& pos, PagedWorldSection* section)
    {
        Grid3DPageStrategyData* stratData = static_cast<Grid3DPageStrategyData*>(section->getStrategyData());

        int32 x, y, z;
        stratData->determineGridLocation(pos, &x, &y, &z);
        return stratData->calculatePageID(x, y, z);
    }
}
