/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2015 Torus Knot Software Ltd

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

#include "OgreD3D11StateManager.h"
#include <OgreTextureUnitState.h>

namespace Ogre
{
    const String StateManager::StateCreateMethodName = "StateManager::createOrRetrieveState";

    D3D11RenderOperationState * D3D11RenderOperationStateGroup::getState(const RenderOperationStateDescriptor& renderOperationStateDescriptor)
    {
        const RenderOperationStateDescriptor::Key key = renderOperationStateDescriptor.GetKey();
        StateMapConstInterator it = states.find(key);
        return it != states.end() ? it->second : NULL;
    }

    void D3D11RenderOperationStateGroup::addState(const RenderOperationStateDescriptor& renderOperationStateDescriptor, D3D11RenderOperationState * opState)
    {
        const RenderOperationStateDescriptor::Key key = renderOperationStateDescriptor.GetKey();
        opState->descriptor = renderOperationStateDescriptor;
        states.insert(StateMapPair(key, opState));
    }

    void D3D11RenderOperationStateGroup::SyncToVersion(const unsigned int version)
    {
        if (mVersion < version)
        {
            ClearAllStates();
            mVersion = version;
        }
    }

    void D3D11RenderOperationStateGroup::ClearAllStates()
    {
        StateMapConstInterator it_end = states.end();
        for (StateMapConstInterator it = states.begin(); it != it_end; it++)
        {
            delete it->second;
        }
        states.clear();
    }

    D3D11RenderOperationStateGroup::D3D11RenderOperationStateGroup()
    {
        mVersion = 0;
    }

    D3D11RenderOperationStateGroup::~D3D11RenderOperationStateGroup()
    {
        ClearAllStates();
    }


    D3D11RenderOperationState::D3D11RenderOperationState() :
        mBlendState(NULL)
        , mRasterizer(NULL)
        , mDepthStencilState(NULL)
        , autoRelease(true)
        , autoReleaseBaseStates(true)
        , autoReleaseSamplers(true)
    {
        memset(mSamplers, 0, sizeof(Samplers));
    }

    D3D11RenderOperationState::~D3D11RenderOperationState()
    {
        if (autoRelease)
        {
            release();
        }
    }

    void D3D11RenderOperationState::release()
    {
        if (autoReleaseBaseStates == true)
        {
            SAFE_RELEASE(mBlendState);
            SAFE_RELEASE(mRasterizer);
            SAFE_RELEASE(mDepthStencilState);

        }

        if (autoReleaseSamplers == true)
        {
            for (size_t i = 0; i < OGRE_MAX_TEXTURE_LAYERS; i++)
            {
                for (size_t j = 0; j < TextureUnitState::BT_SHIFT_COUNT; j++)
                {
                    SAFE_RELEASE(mSamplers[j].mSamplerStates[i]);
                }
            }
        }
    }

    StateManager::StateManager(D3D11Device& device) :  mDevice(device)
    {
        mVersion = 0;
    }

    void StateManager::increaseVersion()
    {
        mVersion++;
    }

    const unsigned int StateManager::getVersion()
    {
        return mVersion;
    }


    String StateManager::getCreateErrorMessage(const String& objectType)
    {
        return String("D3D11 device cannot create " + objectType +" state object");
    }


    ID3D11BlendState* StateManager::createOrRetrieveState(const D3D11_BLEND_DESC& blendDesc)
    {
        return createOrRetrieveStateTemplated<ID3D11BlendState*, D3D11_BLEND_DESC, D3D11_REQ_BLEND_OBJECT_COUNT_PER_DEVICE>
            (mBlendStateMap, blendDesc);
    }

    ID3D11BlendState* StateManager::createState(const D3D11_BLEND_DESC& blendDesc)
    {
        ID3D11BlendState* state = NULL;
        HRESULT hr = mDevice->CreateBlendState(&blendDesc, &state);

        if (FAILED(hr))
        {
            String errorDescription = mDevice.getErrorDescription(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                getCreateErrorMessage("blend") + "\n" + errorDescription,
                StateCreateMethodName);
        }

        return state;
    }

    ID3D11RasterizerState* StateManager::createOrRetrieveState(const D3D11_RASTERIZER_DESC& rasterizerDesc)
    {
        return createOrRetrieveStateTemplated<ID3D11RasterizerState*, D3D11_RASTERIZER_DESC, D3D11_REQ_RASTERIZER_OBJECT_COUNT_PER_DEVICE>
            (mRasterizerStateMap, rasterizerDesc);
    }

    ID3D11RasterizerState* StateManager::createState(const D3D11_RASTERIZER_DESC& rasterizerDesc)
    {
        ID3D11RasterizerState* state = NULL;
        HRESULT hr = mDevice->CreateRasterizerState(&rasterizerDesc, &state);

        if (FAILED(hr))
        {
            String errorDescription = mDevice.getErrorDescription(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                getCreateErrorMessage("rasterizer") + "\n" + errorDescription,
                StateCreateMethodName);
        }


        return state;
    }

    ID3D11DepthStencilState* StateManager::createOrRetrieveState(const D3D11_DEPTH_STENCIL_DESC& depthStencilDesc)
    {
        return createOrRetrieveStateTemplated<ID3D11DepthStencilState*, D3D11_DEPTH_STENCIL_DESC, D3D11_REQ_DEPTH_STENCIL_OBJECT_COUNT_PER_DEVICE>
            (mDepthStencilStateMap, depthStencilDesc);
    }

    ID3D11DepthStencilState* StateManager::createState(const D3D11_DEPTH_STENCIL_DESC& depthStencilDesc)
    {
        ID3D11DepthStencilState* state = NULL;
        HRESULT hr = mDevice->CreateDepthStencilState(&depthStencilDesc, &state);

        if (FAILED(hr))
        {
            String errorDescription = mDevice.getErrorDescription(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
                getCreateErrorMessage("depth-stencil") + "\n" + errorDescription,
                StateCreateMethodName);
        }
        return state;
    }


    ID3D11SamplerState* StateManager::createOrRetrieveState(const D3D11_SAMPLER_DESC& samplerDesc)
    {
        return createOrRetrieveStateTemplated<ID3D11SamplerState*, D3D11_SAMPLER_DESC, D3D11_REQ_SAMPLER_OBJECT_COUNT_PER_DEVICE>
            (mSamplerMap, samplerDesc);
    }

    ID3D11SamplerState* StateManager::createState(const D3D11_SAMPLER_DESC& samplerDesc)
    {
        ID3D11SamplerState* state = NULL;
        HRESULT hr = mDevice->CreateSamplerState(&samplerDesc, &state);
        if (FAILED(hr))
        {
            String errorDescription = mDevice.getErrorDescription(hr);


            OGRE_EXCEPT_EX(Exception::ERR_RENDERINGAPI_ERROR, hr,
                getCreateErrorMessage("sampler") + "\n" + errorDescription,
                "StateManager::createState");
        }
        return state;
    }

    StateManager::~StateManager()
    {
        {
            MapCRCSamplerState::const_iterator it_end = mSamplerMap.end();
            for (MapCRCSamplerState::iterator it = mSamplerMap.begin(); it != it_end; it++)
            {
                delete it->second;
            }
        }

        {
            MapCRCBlendState::const_iterator it_end = mBlendStateMap.end();
            for (MapCRCBlendState::iterator it = mBlendStateMap.begin(); it != it_end; it++)
            {
                delete it->second;
            }
        }

        {
            MapCRCDepthStencilState::const_iterator it_end = mDepthStencilStateMap.end();
            for (MapCRCDepthStencilState::iterator it = mDepthStencilStateMap.begin(); it != it_end; it++)
            {
                delete it->second;
            }
        }

        {
            MapCRCRasterizeState::const_iterator it_end = mRasterizerStateMap.end();
            for (MapCRCRasterizeState::iterator it = mRasterizerStateMap.begin(); it != it_end; it++)
            {
                delete it->second;
            }
        }

    }




    template<class D3D11STATE, class D3DDESC, int MAX_OBJECTS>
    D3D11STATE StateManager::createOrRetrieveStateTemplated(std::map<crc, StateObjectEntry<D3D11STATE>* >& container, const D3DDESC& desc)
    {
        crc descCrc = crc32_8bytes((unsigned char  *)&desc, sizeof(desc));
        typedef  D3D11STATE UnderlyingState;

        typedef  StateObjectEntry<UnderlyingState> StateObject;
        typedef  std::map< crc, StateObject* >StateObjectContainer;
        typedef  StateObjectContainer::iterator StateObjectContainerIterator;



        UnderlyingState state = NULL;
        StateObjectContainerIterator it = container.find(descCrc);
        if (it == container.end())
        {
            if (container.size() >= MAX_OBJECTS)
            {

#if SAVE_STATE_RENDERABLE == 1
                OGRE_EXCEPT(Exception::ERR_INVALID_STATE,
                    "can not free render state object while SAVE_STATE_RENDERABLE optimisation is enabled.", 
                    "StateManager::createOrRetrieveStateTemplated");
#else
                std::vector<StateObject*> vec;
                vec.reserve(MAX_OBJECTS);

                StateObjectContainerIterator it_end = container.end();
                for (StateObjectContainerIterator it = container.begin(); it != it_end; it++)
                {
                    vec.push_back(it->second);
                }

                std::sort(vec.begin(), vec.end(), StateObject::TimeStampComparer());

                uint64 fillTime = vec[MAX_OBJECTS - 1]->getLastAccessTime() - vec[0]->getLastAccessTime();
                assert("State object are being created too fast, check your code or disable SAVE_STATE_OBJECTS optimization" && fillTime > 3000);

                // purge 15 percent of state objects
                size_t count = MAX_OBJECTS * 0.15;

                for (size_t i = 0; i < count; i++)
                {
                    StateObjectContainerIterator it = container.find(vec[i]->getCrc());
                    assert(it != container.end());
                    delete it->second;
                    container.erase(it);
                }

#endif
            }
            // make sure state objects get destroyed by calling flush
            mDevice.GetImmediateContext()->Flush();
            state = createState(desc);
            container.insert(std::make_pair(descCrc, new StateObject(state, descCrc)));
        }
        else
        {
            state = it->second->getStateObject();
        }
        return state;
    }

}
