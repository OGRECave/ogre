#ifndef __D3D11STATEMANAGER_H__
#define __D3D11STATEMANAGER_H__
#include "OgrePrerequisites.h"
#include "OgreD3D11Prerequisites.h"
#include "OgreCRC.h"
#include "OgreRenderable.h"
#include "OgreD3D11Device.h"
#include <OgreTextureUnitState.h>


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


    template <class T>
    class StateObjectEntry
    {
    public:

        struct TimeStampComparer
        {
            bool operator() (StateObjectEntry* A, StateObjectEntry* B)
            {
                return A->getLastAccessTime() < B->getLastAccessTime();
            }
        };

        StateObjectEntry(T stateObject, crc descCrc)
        {
            mStateObject = stateObject;
            mCrc = descCrc;
            mLastAccess = static_cast<uint64>(clock());
        }
        T getStateObject()
        {
            mLastAccess = static_cast<uint64>(clock());
            return mStateObject;
        }

        uint64 getLastAccessTime() const 
        {
            return mLastAccess;
        }

        crc getCrc()
        {
            return mCrc;
        }

        ~StateObjectEntry()
        {
            SAFE_RELEASE(mStateObject);
        
        }


    private:
        T mStateObject;
        uint64 mLastAccess;
        crc mCrc;
        
    };


   



    //---------------------------------------------------------------------
    
    class MapCRCBlendState: public std::map< crc, StateObjectEntry<ID3D11BlendState*> *>{};
    class MapCRCDepthStencilState : public std::map< crc, StateObjectEntry<ID3D11DepthStencilState*> *>{};
    class MapCRCRasterizeState : public std::map< crc, StateObjectEntry<ID3D11RasterizerState*>* >{};
    class MapCRCSamplerState : public std::map< crc, StateObjectEntry<ID3D11SamplerState* >* >{};
 
    class StateManager
    {
    private:

        static const String StateCreateMethodName;
        unsigned int mVersion;
        MapCRCBlendState mBlendStateMap;
        MapCRCDepthStencilState mDepthStencilStateMap;
        MapCRCRasterizeState mRasterizerStateMap;
        MapCRCSamplerState mSamplerMap;
        D3D11Device& mDevice;

        String getCreateErrorMessage(const String& objectType);
    public:
        StateManager(D3D11Device& device);
        ~StateManager();

        template<class D3D11STATE, class D3DDESC, int MAX_OBJECTS>
        D3D11STATE createOrRetrieveStateTemplated(std::map<crc, StateObjectEntry<D3D11STATE>* >& container, const D3DDESC& desc);
        
        ID3D11BlendState* createOrRetrieveState(const D3D11_BLEND_DESC& blendDesc);
        ID3D11BlendState* createState(const D3D11_BLEND_DESC& blendDesc);
        ID3D11RasterizerState* createOrRetrieveState(const D3D11_RASTERIZER_DESC& rasterizerDesc);
        ID3D11RasterizerState* createState(const D3D11_RASTERIZER_DESC& rasterizerDesc);
        ID3D11DepthStencilState* createOrRetrieveState(const D3D11_DEPTH_STENCIL_DESC& depthStencilDesc);
        ID3D11DepthStencilState* createState(const D3D11_DEPTH_STENCIL_DESC& depthStencilDesc);
        ID3D11SamplerState* createOrRetrieveState(const D3D11_SAMPLER_DESC& samplerDesc);
        ID3D11SamplerState* createState(const D3D11_SAMPLER_DESC& samplerDesc);
    

        void increaseVersion();
        const unsigned int getVersion();
    };

    
    class D3D11RenderOperationState
    {
    public:
        
        // structure holding a set of D3D11 samplers and 'shader resource views' for a specific pipeline stage.
        struct SamplersStageGroup
        {
            ID3D11SamplerState * mSamplerStates[OGRE_MAX_TEXTURE_LAYERS];
            size_t mSamplerStatesCount;

            ID3D11ShaderResourceView * mTextures[OGRE_MAX_TEXTURE_LAYERS];
            size_t mTexturesCount;
        };

        //An array holding a set of all D3D11 samplers and 'shader resource views' for all the pipeline stages.
        typedef SamplersStageGroup Samplers[TextureUnitState::BT_SHIFT_COUNT];

        ID3D11BlendState * mBlendState;
        ID3D11RasterizerState * mRasterizer;
        ID3D11DepthStencilState * mDepthStencilState;

        Samplers mSamplers;
        
        RenderOperationStateDescriptor descriptor;

        bool autoRelease;

        // Set to false to exclude the release of ID3D11BlendState, ID3D11DepthStencilState and  ID3D11RasterizerState.
        // If set to false, the user must be responsible to release these.
        bool autoReleaseBaseStates;
        bool autoReleaseSamplers;
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
