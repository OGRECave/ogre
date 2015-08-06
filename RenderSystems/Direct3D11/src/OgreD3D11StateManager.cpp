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

        for (size_t i = 0 ; i < OGRE_MAX_TEXTURE_LAYERS ; i++)
        {
            for (size_t j = 0; j < TextureUnitState::BT_SHIFT_COUNT; j++)
            {
                SAFE_RELEASE(mSamplers[j].mSamplerStates[i] );
                mSamplers[j].mTextures[i] = NULL;
            }
        }
    }

    StateManager::StateManager()
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

    CRCClassMap <ID3D11BlendState *> & StateManager::getBlendStateMap()
    {
        return mBlendStateMap;
    }

    CRCClassMap <ID3D11DepthStencilState *> & StateManager::getDepthStencilStateMap()
    {
        return mDepthStencilStateMap;
    }

    CRCClassMap <ID3D11RasterizerState *> & StateManager::getRasterizerStateMap()
    {
        return mRasterizerStateMap;
    }

    CRCClassMap <ID3D11SamplerState *> & StateManager::getSamplerMap()
    {
        return mSamplerMap;
    }

    StateManager::~StateManager()
    {
        {
            CRCClassMap <ID3D11SamplerState *>::const_iterator it_end = mSamplerMap.end();
            for (CRCClassMap <ID3D11SamplerState *>::iterator it = mSamplerMap.begin(); it != it_end; it++)
            {
                SAFE_RELEASE(it->second);
            }
        }

        {
            CRCClassMap <ID3D11BlendState *>::const_iterator it_end = mBlendStateMap.end();
            for (CRCClassMap <ID3D11BlendState *>::iterator it = mBlendStateMap.begin(); it != it_end; it++)
            {
                SAFE_RELEASE(it->second);
            }
        }

        {
            CRCClassMap <ID3D11DepthStencilState *>::const_iterator it_end = mDepthStencilStateMap.end();
            for (CRCClassMap <ID3D11DepthStencilState *>::iterator it = mDepthStencilStateMap.begin(); it != it_end; it++)
            {
                SAFE_RELEASE(it->second);
            }
        }

        {
            CRCClassMap <ID3D11RasterizerState*>::const_iterator it_end = mRasterizerStateMap.end();
            for (CRCClassMap <ID3D11RasterizerState*>::iterator it = mRasterizerStateMap.begin(); it != it_end; it++)
            {
                SAFE_RELEASE(it->second);
            }
        }

    }




}
