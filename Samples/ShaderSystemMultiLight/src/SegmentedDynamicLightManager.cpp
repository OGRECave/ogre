#include "SegmentedDynamicLightManager.h"
#include "OgreTextureManager.h"
#include "OgreCamera.h"
#include "OgreSceneManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderable.h"
#include "OgreBitwise.h"
#include "OgrePixelFormat.h"
#include "OgreRoot.h"
#include "OgreViewport.h"

#define SDL_LIGHT_DATA_SIZE 3 // 12 floats divided by 4 slots (rgba) 

namespace Ogre
{
    template<> SegmentedDynamicLightManager* Singleton<SegmentedDynamicLightManager>::msSingleton = 0;
}

SegmentedDynamicLightManager* SegmentedDynamicLightManager::getSingletonPtr(void)
{
    return msSingleton;
}
SegmentedDynamicLightManager& SegmentedDynamicLightManager::getSingleton(void)
{
    assert( msSingleton );  return ( *msSingleton );
}

using namespace Ogre;

const String c_SDLTextureName = "Simigon/SDLTexture";

SegmentedDynamicLightManager::SegmentedDynamicLightManager() :
    mIsDebugMode(false),
    mManager(NULL),
    mSegmentedLightGrid(SDL_SEGMENT_GRID_SIZE),
    mLightTexture(),
    mTextureWidth(0),
    mTextureHeight(SDL_TEXTURE_ROWS)
{
    //calculate needed texture width
    mTextureWidth = SDL_LIGHT_DATA_SIZE * SDL_SEGMENT_GRID_SIZE;
    //round up to the nearest power of 2
    unsigned int pow2Val = 1;
    for( ; mTextureWidth > pow2Val; pow2Val = pow2Val << 1);
    mTextureWidth = pow2Val;
}

//------------------------------------------------------------------------------
SegmentedDynamicLightManager::~SegmentedDynamicLightManager()
{
    setSceneManager(NULL);
    if (mLightTexture.get())
    {
        TextureManager::getSingleton().remove(mLightTexture->getHandle());
    }
}

//------------------------------------------------------------------------------
bool SegmentedDynamicLightManager::setDebugMode(bool i_IsDebugMode)
{
    bool requireInvalidate = false;
    if (mIsDebugMode != i_IsDebugMode)
    {
        mIsDebugMode = i_IsDebugMode;
        requireInvalidate = true;
    }
    return requireInvalidate;
}
    
//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::postFindVisibleObjects(SceneManager* source, 
    SceneManager::IlluminationRenderStage irs, Viewport* v)
{
    if (irs == SceneManager::IRS_NONE)
    {
        updateLightList(v->getCamera(), source->_getLightsAffectingFrustum());
    }
}

//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::setSceneManager(SceneManager* i_Manager) 
{ 
    if (mManager != i_Manager)
    {
        if (mManager) mManager->removeListener(this);
        mManager = i_Manager;
        if (mManager) 
        {
            mManager->addListener(this);
            initTexture();
        }
    }
}
        
//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::updateLightList(const Camera* i_pCamera, const LightList& i_LightList)
{
    if (isActive())
    {
        arrangeLightsInSegmentedLists(i_pCamera, i_LightList);
        updateTextureFromSegmentedLists(i_pCamera);
    }
}

//------------------------------------------------------------------------------
bool SegmentedDynamicLightManager::initTexture()
{
    if (mLightTexture.get() == NULL)
    {
        const String& sdlTextureName = getSDLTextureName();
        // create the render texture
        mLightTexture = TextureManager::getSingleton().createManual(sdlTextureName, 
            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,TEX_TYPE_2D,
            mTextureWidth,mTextureHeight,0,PF_FLOAT16_RGBA,TU_STATIC_WRITE_ONLY);
    }
    return mLightTexture.get() != NULL;
}

//------------------------------------------------------------------------------
const String& SegmentedDynamicLightManager::getSDLTextureName()
{
    return c_SDLTextureName;
}

//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::arrangeLightsInSegmentedLists(const Camera* i_pCamera, const LightList& i_LightList)
{
    //Clear the previous buffers
    for(int i = 0; i < SDL_SEGMENT_GRID_SIZE; ++i)
    {
        mSegmentedLightGrid[i].clear();
    }
    mActiveLights.clear();
        
    regenerateActiveLightList(i_LightList);
    recalculateGridSize();
    distributeLightsInGrid();
}
    
//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::regenerateActiveLightList(const LightList& i_LightList)
{
    //add the buffers to the segmented lists
    LightList::const_iterator itLight = i_LightList.begin(),
        itLightEnd = i_LightList.end();
    for(;itLight != itLightEnd ; ++itLight)
    {
        const Light* pLight = (*itLight);
        Light::LightTypes type = pLight->getType();
        if (((type == Light::LT_SPOTLIGHT) || (type == Light::LT_POINT)) &&
            (pLight->getAttenuationRange() > 0))
        {
                
            MapLightData::iterator it = mActiveLights.emplace(pLight,LightData()).first;
            LightData& lightData = it->second;
                
            calculateLightBounds(pLight, lightData);
        }
    }
}

//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::calculateLightBounds(const Light* i_Light, LightData& o_LightData)
{ 
    Real lightRange = i_Light->getAttenuationRange();
    const Vector3& lightPosition = i_Light->getDerivedPosition(true);

    AxisAlignedBox boundBox(lightPosition - lightRange, lightPosition + lightRange);

    if (i_Light->getType() == Light::LT_SPOTLIGHT)
    {
        static const Radian c_RadianPI(Math::PI);
        static const Radian c_RadianZero(0);

        Radian halfOuterAngle = i_Light->getSpotlightOuterAngle() * 0.5;
        Real boxOffset = Math::Sin(halfOuterAngle) * lightRange;
        const Vector3& lightDirection = i_Light->getDerivedDirection();

        Radian dirUpAngle(fabs(Math::ASin(lightDirection.y).valueRadians()));
        Radian dirUpMaxAngle = std::max<Radian>(dirUpAngle - halfOuterAngle,c_RadianZero);
        Radian dirUpMinAngle = std::min<Radian>(dirUpAngle + halfOuterAngle, c_RadianPI);
        Real dirDistanceMax = Math::Cos(dirUpMaxAngle) * lightRange;
        Real dirDistanceMin = Math::Cos(dirUpMinAngle) * lightRange;
            
        Vector3 flatDirection(lightDirection.x, 0, lightDirection.z);
        Real flatDirLen = flatDirection.length();
        if (flatDirLen != 0) flatDirection /= flatDirLen; 
        else flatDirection = Vector3(1,0,0);

        Vector3 flatDirectionPerp(flatDirection.z, 0, -flatDirection.x);
        flatDirectionPerp *= boxOffset;

        Vector3 flatPositionMax = lightPosition + dirDistanceMax * flatDirection;
        Vector3 flatPositionMin = lightPosition + dirDistanceMin * flatDirection;

        AxisAlignedBox spotBox;
        spotBox.merge(flatPositionMax + flatDirectionPerp);
        spotBox.merge(flatPositionMax - flatDirectionPerp);
        spotBox.merge(flatPositionMin + flatDirectionPerp);
        spotBox.merge(flatPositionMin - flatDirectionPerp);
        spotBox.merge(lightPosition);
            
        boundBox.getMaximum().makeFloor(spotBox.getMaximum());
        boundBox.getMinimum().makeCeil(spotBox.getMinimum());
    }

    o_LightData.setBounds(boundBox);
}

//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::recalculateGridSize()
{
    mGridMinX = std::numeric_limits<Real>::max();
    mGridMinZ = std::numeric_limits<Real>::max();
    mGridMaxX = -std::numeric_limits<Real>::max();
    mGridMaxZ = -std::numeric_limits<Real>::max();
        
    MapLightData::const_iterator it = mActiveLights.begin(),
        itEnd = mActiveLights.end();
    for(;it != itEnd ; ++it)
    {
        const LightData& lightData = it->second;
        mGridMinX = std::min<Real>(mGridMinX,lightData.getMinX());
        mGridMaxX = std::max<Real>(mGridMaxX,lightData.getMaxX());
        mGridMinZ = std::min<Real>(mGridMinZ,lightData.getMinZ());
        mGridMaxZ = std::max<Real>(mGridMaxZ,lightData.getMaxZ());
    }
}
    
//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::distributeLightsInGrid()
{
    MapLightData::iterator it = mActiveLights.begin(),
        itEnd = mActiveLights.end();
    for(;it != itEnd ; ++it)
    {
        LightData& lightData = it->second;
        unsigned int indexXStart = calcGridColumn(lightData.getMinX(), mGridMinX, mGridMaxX);
        unsigned int indexXEnd = calcGridColumn(lightData.getMaxX(), mGridMinX, mGridMaxX);
        unsigned int indexZStart = calcGridColumn(lightData.getMinZ(), mGridMinZ, mGridMaxZ);
        unsigned int indexZEnd = calcGridColumn(lightData.getMaxZ(), mGridMinZ, mGridMaxZ);
        for(unsigned int i = indexXStart ; i <= indexXEnd ; ++i)
        {
            for(unsigned int j = indexZStart ; j <= indexZEnd ; ++j)
            {
                VecLights& block = mSegmentedLightGrid[calcGridIndex(i,j)];
                unsigned int lightIndex = (unsigned int)block.size();
                if (lightIndex < SDL_LIGHT_PER_BLOCK)
                {
                    block.push_back(it->first);
                    lightData.addIndexToRange(lightIndex);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
unsigned int SegmentedDynamicLightManager::calcGridColumn(Real i_Position, 
    Real i_BoundStart, Real i_BoundEnd)
{
    int index = (unsigned int)
        (((i_Position - i_BoundStart) / (i_BoundEnd - i_BoundStart)) * SDL_SEGMENT_DIVISIONS);
    return (unsigned int)Math::Clamp<int>(index, 0 ,SDL_SEGMENT_DIVISIONS - 1);
}

unsigned int SegmentedDynamicLightManager::calcGridIndex(unsigned int i_X, unsigned int i_Y)
{
    return i_X + i_Y * SDL_SEGMENT_DIVISIONS;
}

    
//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::updateTextureFromSegmentedLists(const Camera* i_pCamera)
{
    float spotIntensity = 1;
        
    HardwarePixelBufferSharedPtr pBuf = mLightTexture->getBuffer();
    void* pStartPos = pBuf->lock(HardwareBuffer::HBL_DISCARD);
    uint16* pData = (uint16*)pStartPos;

    size_t remainBufWidth = mTextureWidth;
    for(size_t j = 0; j < SDL_SEGMENT_GRID_SIZE; ++j)
    {
        //assign first row with number of indexes in the block
        float maxRow = (float)(mSegmentedLightGrid[j].size() - 1 + SDL_TEXTURE_DATA_ROWS);
        PixelUtil::packColour(maxRow,0.0f,0.0f,0.0f,PF_FLOAT16_RGBA, pData);
        pData += 4 * SDL_LIGHT_DATA_SIZE;       
        remainBufWidth -= SDL_LIGHT_DATA_SIZE;
    }

    //advance the remaining space of the row 
    pData += 4 * remainBufWidth;        
        
    for(size_t i = 0 ; i < SDL_LIGHT_PER_BLOCK ; ++i)
    {
        remainBufWidth = mTextureWidth;
        for(size_t j = 0; j < SDL_SEGMENT_GRID_SIZE; ++j)
        {
            if (i < mSegmentedLightGrid[j].size())
            {
                const Light* pLight = mSegmentedLightGrid[j][i];
                    
                const Vector3& position = pLight->getDerivedPosition(true);
                Vector3 direction = -pLight->getDerivedDirection();
                direction.normalise();

                // Update spotlight parameters.
                Vector3 spotParam;
                float inverseRange = 1.0f / (float)pLight->getAttenuationRange();
                float spotAngle = -1;
                float spotInvAngleRange = std::numeric_limits<float>::max();
                if (pLight->getType() == Light::LT_SPOTLIGHT)
                {
                    Real phi   = Math::Cos(pLight->getSpotlightOuterAngle().valueRadians() * 0.5f);
                    Real theta = Math::Cos(pLight->getSpotlightInnerAngle().valueRadians() * 0.5f);
                    spotAngle = (float)phi;
                    spotInvAngleRange = 1.0f / (float)(theta - phi);
                }
                    
                PixelUtil::packColour(
                    (float)position.x,
                    (float)position.y,
                    (float)position.z,
                    inverseRange,
                    PF_FLOAT16_RGBA, pData);
                pData += 4;         

                PixelUtil::packColour(
                    (float)direction.x,
                    (float)direction.y,
                    (float)direction.z,
                    spotAngle,
                    PF_FLOAT16_RGBA, pData);
                pData += 4; 

                PixelUtil::packColour(
                    pLight->getDiffuseColour().r * spotIntensity,
                    pLight->getDiffuseColour().g * spotIntensity,
                    pLight->getDiffuseColour().b * spotIntensity,
                    spotInvAngleRange,
                    PF_FLOAT16_RGBA, pData);
                pData += 4;         

            }
            else
            {
                //assign position zero with zero width
                PixelUtil::packColour(0.0f,0.0f,0.0f,std::numeric_limits<float>::max(),
                    PF_FLOAT16_RGBA, pData);
                pData += 4;         
                for(int d = 0 ; d < (SDL_LIGHT_DATA_SIZE - 1) ; ++d)
                {
                    PixelUtil::packColour(0.0f,0.0f,0.0f,0.0f,PF_FLOAT16_RGBA, pData);
                    pData += 4;         
                }
            }
            remainBufWidth -= 3;
        }

        //advance the remaining space of the row 
        pData += 4 * remainBufWidth;        
    }

    //Check for memory overrun
    if (pBuf->getSizeInBytes() != (size_t)((const char*)(void*)pData - (const char*)pStartPos))
    {
        throw "memory overrun";
    }

    pBuf->unlock();
}

//------------------------------------------------------------------------------
bool SegmentedDynamicLightManager::getLightListRange(const Renderable* i_Rend, 
    Vector4& o_GridBounds, unsigned int& o_IndexMin, unsigned int& o_IndexMax)
{
    o_IndexMin = 100000;
    o_IndexMax = 0;
        
    const LightList& lights = i_Rend->getLights();
    LightList::const_iterator it = lights.begin(), itEnd = lights.end();
    for(; it != itEnd ; ++it)
    {
        MapLightData::const_iterator itActive = mActiveLights.find(*it);
        if (itActive != mActiveLights.end())
        {
            o_IndexMin = (unsigned int)std::min<unsigned int>(o_IndexMin, itActive->second.getIndexMin());
            o_IndexMax = (unsigned int)std::max<unsigned int>(o_IndexMax, itActive->second.getIndexMax());
        }
    }

    o_GridBounds.x = mGridMinX;
    o_GridBounds.y = mGridMinZ;
    o_GridBounds.z = SDL_SEGMENT_DIVISIONS / (mGridMaxX - mGridMinX);
    o_GridBounds.w = SDL_SEGMENT_DIVISIONS / (mGridMaxZ - mGridMinZ);
    o_IndexMin += SDL_TEXTURE_DATA_ROWS;
    o_IndexMax += SDL_TEXTURE_DATA_ROWS;
    return o_IndexMin <= o_IndexMax;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////      SegmentedDynamicLightManager::LightData
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
SegmentedDynamicLightManager::LightData::LightData() 
{   
    mIndexMin = 100000;
    mIndexMax = 0;
    mMinX = std::numeric_limits<Real>::max(); 
    mMaxX = -std::numeric_limits<Real>::max(); 
    mMinZ = std::numeric_limits<Real>::max(); 
    mMaxZ = -std::numeric_limits<Real>::max(); 
}

//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::LightData::setBounds(const AxisAlignedBox& i_Bounds)
{
    mMinX = i_Bounds.getMinimum().x;
    mMaxX = i_Bounds.getMaximum().x;
    mMinZ = i_Bounds.getMinimum().z;
    mMaxZ = i_Bounds.getMaximum().z;
}

//------------------------------------------------------------------------------
void SegmentedDynamicLightManager::LightData::addIndexToRange(unsigned int i_LightIndex)
{
    mIndexMin = (unsigned int)std::min<unsigned int>(mIndexMin, i_LightIndex);
    mIndexMax = (unsigned int)std::max<unsigned int>(mIndexMax, i_LightIndex);
}
