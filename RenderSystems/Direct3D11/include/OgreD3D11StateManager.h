#ifndef __D3D11STATEMANAGER_H__
#define __D3D11STATEMANAGER_H__
#include "OgrePrerequisites.h"
#include "OgreD3D11Prerequisites.h"
#include "OgreCRC.h"
#include "OgreRenderable.h"


namespace Ogre
{
    class RenderOperationStateDescriptor
    {
    public:
        struct Key
        {
            RenderTarget* mActiveRenderTarget;
            Camera* mCamera;
			uint32 mRenderStateHash;
            inline bool operator < ( const Key& rhs ) const
            {

				// Brute force approach, more readable

				if (mActiveRenderTarget < rhs.mActiveRenderTarget)  return true;
				if (mActiveRenderTarget > rhs.mActiveRenderTarget)  return false;

				if (mCamera < rhs.mCamera)  return true;
				if (mCamera > rhs.mCamera)  return false;

				if (mRenderStateHash < rhs.mRenderStateHash)  return true;
				if (mRenderStateHash > rhs.mRenderStateHash)  return false;
				

				return false;
				
            }
        };
    private:
        Key mKey;

    public:
        void setActiveRenderTarget(RenderTarget* activeRenderTarget)
        {
            mKey.mActiveRenderTarget = activeRenderTarget;
        }
        void setCamera(Camera* camera)
        {
            mKey.mCamera = camera;
        }
		void setRenderStateHash(uint32 renderStateHash)
		{
			mKey.mRenderStateHash = renderStateHash;
		}

        RenderOperationStateDescriptor()
        {
            memset(&mKey, 0, sizeof(mKey));
        }
        virtual ~RenderOperationStateDescriptor()
        {
        }

        bool VerifyValidaity()
        {
            return mKey.mActiveRenderTarget != NULL && mKey.mCamera != NULL;
        }

        const Key GetKey() const
        {
            return mKey;
        }

    };

    //---------------------------------------------------------------------
    
    template <class T>  class CRCClassMap : public map< crc, T >::type  {};

    class StateManager
    {
    private:
        unsigned int mVersion;
        CRCClassMap<ID3D11BlendState*> mBlendStateMap;
        CRCClassMap<ID3D11DepthStencilState*> mDepthStencilStateMap;
        CRCClassMap<ID3D11RasterizerState*> mRasterizerStateMap;
        CRCClassMap<ID3D11SamplerState*> mSamplerMap;
    public:
        CRCClassMap<ID3D11BlendState*> & getBlendStateMap();;
        CRCClassMap<ID3D11DepthStencilState*>& getDepthStencilStateMap();;
        CRCClassMap<ID3D11RasterizerState*>& getRasterizerStateMap();;
        CRCClassMap<ID3D11SamplerState*>& getSamplerMap();;
        StateManager();
        ~StateManager();

        void increaseVersion();
        const unsigned int getVersion();
    };

    
    class D3D11RenderOperationState
    {
    public:
        ID3D11BlendState * mBlendState;
        ID3D11RasterizerState * mRasterizer;
        ID3D11DepthStencilState * mDepthStencilState;

        ID3D11SamplerState * mSamplerStates[OGRE_MAX_TEXTURE_LAYERS];
        size_t mSamplerStatesCount;

        ID3D11ShaderResourceView * mTextures[OGRE_MAX_TEXTURE_LAYERS];
        size_t mTexturesCount;
        RenderOperationStateDescriptor descriptor;

        bool autoRelease;

        // Set to false to exclude the release of ID3D11BlendState, ID3D11DepthStencilState and  ID3D11RasterizerState.
        // If set to false, the user must be responsible to release these.
        bool autoReleaseBaseStates;
        D3D11RenderOperationState();

        ~D3D11RenderOperationState();

        void release();
    };


    class D3D11RenderOperationStateGroup : public Renderable::RenderSystemData
    {
    private:
        typedef map<RenderOperationStateDescriptor::Key, D3D11RenderOperationState*> StateMap;
        typedef StateMap::const_iterator StateMapConstInterator;
        typedef std::pair<RenderOperationStateDescriptor::Key, D3D11RenderOperationState*> StateMapPair;
        StateMap::type states;
        unsigned int mVersion;
    public:
        D3D11RenderOperationState * getState(const RenderOperationStateDescriptor& renderOperationStateDescriptor);
        void addState(const RenderOperationStateDescriptor& renderOperationStateDescriptor, D3D11RenderOperationState * opState);
        void SyncToVersion(const unsigned int version);
        void ClearAllStates();
        D3D11RenderOperationStateGroup();
        ~D3D11RenderOperationStateGroup();
    };
    typedef SharedPtr<D3D11RenderOperationStateGroup> D3D11RenderOperationStateGroupPtr;
}
#endif
