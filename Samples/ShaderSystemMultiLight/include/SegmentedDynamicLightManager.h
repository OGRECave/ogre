/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _SegmentedLightManager_
#define _SegmentedLightManager_

#include "OgreShaderPrerequisites.h"
#include "OgreSingleton.h"
#include "OgreLight.h"
#include "OgreTexture.h"
#include "OgreLight.h"
#include "OgreSceneManager.h"

#define SDL_SEGMENT_DIVISIONS 9
#define SDL_SEGMENT_GRID_SIZE (SDL_SEGMENT_DIVISIONS * SDL_SEGMENT_DIVISIONS)
#define SDL_LIGHT_DATA_SIZE 3
#define SDL_TEXTURE_ROWS 32
#define SDL_TEXTURE_DATA_ROWS 1
#define SDL_LIGHT_PER_BLOCK (SDL_TEXTURE_ROWS - SDL_TEXTURE_DATA_ROWS)


using namespace Ogre;

class SegmentedDynamicLightManager : public Singleton<SegmentedDynamicLightManager>,
    public SceneManager::Listener
{
    
public:
    SegmentedDynamicLightManager();
    ~SegmentedDynamicLightManager();
            
    bool setDebugMode(bool i_IsDebugMode);
    //Set the system to active mode
    void setSceneManager(SceneManager* i_Manager);
    //Tells if the system is active
    bool isActive() const { return mManager != NULL; }
    //Get the name of the texture used to store the light information
    const String& getSDLTextureName();

    //Get the range of lights in the supplied texture data that need to be calculated for a given renderable 
    bool getLightListRange(const Renderable* i_Rend, Vector4& o_GridBounds, unsigned int& o_IndexMin, unsigned int& o_IndexMax);

    //Get the width of the texture containing the light information
    unsigned int getTextureWidth() const { return mTextureWidth; }
    //Get the height of the texture containing the light information
    unsigned int getTextureHeight() const { return mTextureHeight; }
    //Get the amount of cells the texture is divided into on either axis
    unsigned int getGridDivision() const { return SDL_SEGMENT_DIVISIONS; }
    //Get whether to display the lights in debug mode
    bool isDebugMode() const { return mIsDebugMode; }

    //Implementation of SceneManager::Listener
    virtual void postFindVisibleObjects(SceneManager* source, 
        SceneManager::IlluminationRenderStage irs, Viewport* v);
    
    /// @copydoc Singleton::getSingleton()
    static SegmentedDynamicLightManager& getSingleton(void);
    /// @copydoc Singleton::getSingleton()
    static SegmentedDynamicLightManager* getSingletonPtr(void);
private:
        
    class LightData 
    {
    public:
        //Constructor for LightData
        LightData();
        //Sets the values of the boundaries of the light
        void setBounds(const AxisAlignedBox& i_Bounds);
        //Add an index to the possible range of indexes
        void addIndexToRange(unsigned int i_LightIndex);
            
        unsigned int getIndexMin() const { return mIndexMin; }
        unsigned int getIndexMax() const { return mIndexMax; }

        Real getMinX() const { return mMinX; }
        Real getMaxX() const { return mMaxX; } 
        Real getMinZ() const { return mMinZ; }
        Real getMaxZ() const { return mMaxZ; } 

    private:
        unsigned int mIndexMin;
        unsigned int mIndexMax;
        
        Real mMinX;
        Real mMaxX;
        Real mMinZ;
        Real mMaxZ;
    };

    typedef std::map<const Light*,LightData> MapLightData;

private:
    //Update the systems internal light lists
    void updateLightList(const Camera* i_pCamera, const LightList& i_LightList);
    //Initialize the texture to be used to store the light information
    bool initTexture();
    //Arrange the lights in the different lists
    void arrangeLightsInSegmentedLists(const Camera* i_pCamera, const LightList& i_LightList);
    //Repopulate the m_ActiveLights list which keeps track of all lights being rendered in the frame
    void regenerateActiveLightList(const LightList& i_LightList);
    //Calculate the bounds of a single light
    void calculateLightBounds(const Light* i_Light, LightData &o_LightData);
    //Calculate the area which bounds area in which the lights exist
    void recalculateGridSize();
    //Distribute the lights in the active light list (mActiveLights) in the grid parameter (mSegmentedLightGrid)
    void distributeLightsInGrid();
    //Get the index in the grid of a given world position
    unsigned int calcGridColumn(Real i_Position, Real i_BoundStart, Real i_BoundEnd);
    //Returns a grid index for a given x and y index positions
    unsigned int calcGridIndex(unsigned int i_X, unsigned int i_Y);

    //Load the lights information from the internal lists to the texture
    void updateTextureFromSegmentedLists(const Camera* i_pCamera);

private:
    //Tells whether to run the lights in debug mode
    bool mIsDebugMode;
    //Pointer to a scene manager on which the lights will work
    SceneManager* mManager;
        
    //List of active lights in the frame
    MapLightData mActiveLights;
        
    //A Grid structures to contain the lights as they are represented in the light texture
    typedef std::vector<const Light*> VecLights;
    typedef std::vector<VecLights> SegmentedVecLight;
    SegmentedVecLight mSegmentedLightGrid;

    //A pointer to a texture which containing information from which a shader renders the lights
    TexturePtr mLightTexture;
    //The height of the width information texture
    unsigned int mTextureWidth;
    //The height of the light information texture
    unsigned int mTextureHeight;

        
    //Light grid bounds
    Real mGridMinX;
    Real mGridMinZ;
    Real mGridMaxX;
    Real mGridMaxZ;
};

#endif

