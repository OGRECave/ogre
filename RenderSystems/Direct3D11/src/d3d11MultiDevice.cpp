#include"d3d11MultiDevice.h"

namespace D3D11MultiDevice
{
    IDXGIFactory1* sDxgiFactory = NULL;
//---------------------------------------------------------------------
    template <class Type, class IType>
    IType *const * ConvertArray(IType *const * theArray, UINT arraySize, size_t index)
    {
        if(arraySize == 0)
        {
            return NULL;
        }
        // not good for multi thread... todo...
        static std::vector<IType *> staticArray;
        if(staticArray.size() < arraySize)
        {
            staticArray.resize(arraySize);
        }
        for(size_t i = 0 ; i < arraySize ; i++)
        {
            if(theArray[i] == NULL)
            {
                staticArray[i] = NULL;
            }
            else
            {
                staticArray[i] = static_cast<Type *>(theArray[i])->GetInstance(index);
            }
        }

        return &staticArray[0];
    }
    //---------------------------------------------------------------------

    template <class Type, class IType>
    void CreateMultiDeviceArrayBySingleDeviceArray(IType ** singleDeviceArray, IType ** multiDeviceArray, UINT arraySize, size_t instanceCount)
    {

        for(UINT i = 0 ; i < arraySize ; i++)
        {
            if(singleDeviceArray[i] == NULL)
            {
                multiDeviceArray[i] = NULL;
            }
            else
            {
                Type * newD3D11RenderTargetView = new Type();
                newD3D11RenderTargetView->InitInstancesCount(instanceCount);

                multiDeviceArray[i] = newD3D11RenderTargetView;

            }

        }
    }

    //---------------------------------------------------------------------


    template <class Type, class IType>
    void ConvertToMultiDeviceArray(IType ** singleDeviceArray, IType ** multiDeviceArray, UINT arraySize, size_t instanceIndex)
    {

        for(UINT i = 0 ; i < arraySize ; i++)
        {
            if(singleDeviceArray[i] != NULL)
            {
                *(static_cast<Type *>(multiDeviceArray[i])->GetInstancePtr(instanceIndex)) = singleDeviceArray[i];
            }


        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //            SingleDeviceResource
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

    template <class T>
    SingleDeviceResource<T>::SingleDeviceResource()
    {
        _parent = NULL;
        _d3d11Device = NULL;
        _dxgiDevice = NULL;
        _deviceIdx = -1;
    }

    //---------------------------------------------------------------------

    template <class T>
    SingleDeviceResource<T>::~SingleDeviceResource()
    {
        std::set<SingleDeviceItem *>::iterator iter = _children.begin();
        std::set<SingleDeviceItem *>::iterator iterE = _children.end();
        for(; iter != iterE ; iter++)
        {
            (*iter)->SetParent(NULL);
        }
        if(_d3d11Device != NULL)
        {
            _d3d11Device->Release();
        }
        if(_dxgiDevice != NULL)
        {
            _dxgiDevice->Release();
        }

    }

    //---------------------------------------------------------------------

    template <class T>
    void SingleDeviceResource<T>::SetParent( SingleDeviceItem * i_parent )
    {
        _parent = i_parent;
        if(i_parent != NULL)
        {
            _parent->AddChild(this);
        }
    }

    //---------------------------------------------------------------------

    template <class T>
    SingleDeviceItem * SingleDeviceResource<T>::GetParent()
    {
        return _parent;
    }

    //---------------------------------------------------------------------

    template <class T>
    void SingleDeviceResource<T>::SetD3d11Device( D3D11Device * i_device )
    {
        _d3d11Device = i_device;
        if(_d3d11Device != NULL)
        {
            _d3d11Device->AddRef();
        }

    }

    //---------------------------------------------------------------------

    template <class T>
    D3D11Device * SingleDeviceResource<T>::GetD3d11Device()
    {
        return _d3d11Device;
    }

    //---------------------------------------------------------------------

    template <class T>
    void SingleDeviceResource<T>::SetDxgiDevice( DXGIDevice * i_device )
    {
        _dxgiDevice = i_device;
        if(i_device != NULL)
        {
            _dxgiDevice->AddRef();
        }
    }

    //---------------------------------------------------------------------

    template <class T>
    DXGIDevice * SingleDeviceResource<T>::GetDxgiDevice()
    {
        return _dxgiDevice;
    }

    //---------------------------------------------------------------------

    template <class T>
    ULONG STDMETHODCALLTYPE SingleDeviceResource<T>::Release( void )
    {
        if(GetParent() != NULL)
        {
            GetParent()->RemoveChild(this); 
        }

        return __super::Release();
    }

    //---------------------------------------------------------------------

    template <class T>
    void SingleDeviceResource<T>::RemoveChild( SingleDeviceItem * child )
    {
        _children.erase(child);
    }

    //---------------------------------------------------------------------

    template <class T>
    void SingleDeviceResource<T>::AddChild( SingleDeviceItem * child )
    {
        _children.insert(child);
    }

    //---------------------------------------------------------------------

    template <class T>
    void SingleDeviceResource<T>::LoopChildrenAndSetDeviceIdx( int newIdx )
    {
        std::set<SingleDeviceItem *>::iterator iter = _children.begin();
        std::set<SingleDeviceItem *>::iterator iterE = _children.end();
        for(; iter != iterE ; iter++)
        {
            (*iter)->SetDeviceIdx(newIdx);
        }
    }

    //---------------------------------------------------------------------

    template <class T>
    int SingleDeviceResource<T>::GetDeviceIdx()
    {
        return _deviceIdx;
    }

    //---------------------------------------------------------------------

    template <class T>
    void SingleDeviceResource<T>::SetDeviceIdx( int newIdx )
    {
        _deviceIdx = newIdx;
    }

    //---------------------------------------------------------------------

    template <class T>
    void SingleDeviceResource<T>::InitDeviceIdx( int newIdx )
    {
        _deviceIdx = newIdx;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //            BaseObject
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool g_multiDeviceIsUsed = false;
    int BaseObject::s_numberOfDevices = 0;
    int BaseObject::s_activeDeviceIndex = -1;
    DXGIFactory1 * BaseObject::s_dxgiFactory = NULL;
    std::vector<DXGIAdapter1 *> BaseObject::s_adapters;
    std::map<HMONITOR, int> BaseObject::s_monitorToDeviceIndex;
    std::map<HWND, DXGISwapChain *> BaseObject::s_hwndToSwapChain;

    //---------------------------------------------------------------------

    int BaseObject::GetNumberOfDevices()
    {
        return s_numberOfDevices;
    }

    //---------------------------------------------------------------------

    void BaseObject::SetNumberOfDevices( int val )
    {
        s_numberOfDevices = val;
    }

    //---------------------------------------------------------------------

    int BaseObject::GetActiveDeviceIndex()
    {
        return s_activeDeviceIndex;
    }

    //---------------------------------------------------------------------

    void BaseObject::SetActiveDeviceIndex( int val )
    {
        s_activeDeviceIndex = val;
    }

    //---------------------------------------------------------------------

    DXGIFactory1 * BaseObject::GetFactory()
    {
        return s_dxgiFactory;
    }

    //---------------------------------------------------------------------

    void BaseObject::SetFactory( DXGIFactory1 * val )
    {
        s_dxgiFactory = val;
    }

    //---------------------------------------------------------------------

    std::vector<DXGIAdapter1 *> & BaseObject::GetAdaptersList()
    {
        return s_adapters;
    }

    //---------------------------------------------------------------------

    int BaseObject::GetMonitorDeviceIndex( HMONITOR val )
    {
        if(s_monitorToDeviceIndex.find(val) == s_monitorToDeviceIndex.end())
        {
            return -1;
        }
        return s_monitorToDeviceIndex[val];
    }

    //---------------------------------------------------------------------

    void BaseObject::SetMonitorDeviceIndex( HMONITOR mon, int idx )
    {
        s_monitorToDeviceIndex[mon] = idx;
    }

    //---------------------------------------------------------------------

    DXGISwapChain * BaseObject::GetSwapChainFromHwnd( HWND val )
    {
        return s_hwndToSwapChain[val];
    }

    //---------------------------------------------------------------------

    void BaseObject::SetSwapChainFromHwnd( HWND hwnd, DXGISwapChain * swapChain )
    {
        s_hwndToSwapChain[hwnd] = swapChain;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //            Unknown
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LinkFixUnknown()
    {
        Unknown linkFixUnknown;
    }

    //---------------------------------------------------------------------
    template <class T>
    _Unknown<T>::_Unknown() : _instances(BaseObject::GetNumberOfDevices(), NULL)
    {
        _baseRef = 1;
    }
    //---------------------------------------------------------------------
    template <class T>
    _Unknown<T>::~_Unknown()
    {
    }
    //---------------------------------------------------------------------
    template <class T>
    void _Unknown<T>::InitInstancesCount( const size_t count )
    {
        if(GetInstanceCount()  != count)
        {
            _instances.resize(count);
        }
    }
    //---------------------------------------------------------------------
    template <class T>
    size_t _Unknown<T>::GetInstanceCount()
    {
        return _instances.size();
    }
    //---------------------------------------------------------------------
    template <class T>
    HRESULT _Unknown<T>::QueryInterface( 
        /* [in] */ REFIID riid,
        /* [annotation][iid_is][out] */ 
        __RPC__deref_out  void **ppvObject)
    {
        std::vector<void **> instancesPtr(_instances.size(), NULL);
        void * newUnknown = BaseObject::CreateObjectByGuid(riid, instancesPtr, GetInstanceCount());
        *ppvObject = newUnknown;
        HRESULT result = -1;

        if(newUnknown != NULL)
        {
            ResourcePerDeviceListIter iter = _instances.begin();
            ResourcePerDeviceListIter iterE = _instances.end();
            for(int i = 0; iter != iterE ; i++, iter++)
            {
                if((*iter) != NULL)
                {
                    result = (*iter)->QueryInterface(riid, instancesPtr[i]);
                    if(FAILED(result))
                    {
                        delete newUnknown;
                        *ppvObject = NULL;
                        break;
                    }
                }
            }
        }
        return result;
    }
    //---------------------------------------------------------------------
    template <class T>
    ULONG _Unknown<T>::AddRef(void)
    {
        _baseRef++;
        ULONG res = _baseRef;
        ResourcePerDeviceListIter iter = _instances.begin();
        ResourcePerDeviceListIter iterE = _instances.end();
        for(; iter != iterE ; iter++)
        {
            if(*iter != NULL)
            {
                res = (*iter)->AddRef();
            }
        }

        return _baseRef;
    }
    //---------------------------------------------------------------------
    template <class T>
    ULONG _Unknown<T>::Release(void)
    {
        _baseRef--;
        ULONG res = _baseRef;
        ResourcePerDeviceListIter iter = _instances.begin();
        ResourcePerDeviceListIter iterE = _instances.end();
        for(; iter != iterE ; iter++)
        {
            if(*iter != NULL)
            {
                res = (*iter)->Release();
            }       
        }

        if(_baseRef == 0)
        {
            delete this;
        }
        return _baseRef;
    }
    //---------------------------------------------------------------------
    template <class T>
    T * _Unknown<T>::GetInstance( int index ) const
    {
        return _instances[index];
    }
    //---------------------------------------------------------------------
    template <class T>
    T ** _Unknown<T>::GetInstancePtr( int index ) 
    {
        return &_instances[index];
    }
    //---------------------------------------------------------------------
    template <class T>
    void ** _Unknown<T>::GetInstancePtrAsVoid( int index ) 
    {
        return (void **) GetInstancePtr(index);
    }
    //---------------------------------------------------------------------
    template <class T>
    T * _Unknown<T>::GetActiveDeviceInstance() 
    {
        T * result = NULL;
        int index = BaseObject::GetActiveDeviceIndex();
        if(index != -1)
        {
            result = GetInstance(index);
        }
        return result;
    }
    //---------------------------------------------------------------------
    template <class T>
    T * _Unknown<T>::GetAnyDeviceInstance() 
    {
        // first try to get the active
        T * result = GetActiveDeviceInstance();

        // if active is not set get the first
        if(result == NULL)
        {
            ResourcePerDeviceListIter iter = _instances.begin();
            ResourcePerDeviceListIter iterE = _instances.end();
            for(; iter != iterE ; iter++)
            {
                result = (*iter);
                if(result != NULL)
                {
                    break;
                }
            }
        }
        return result;
    }



    //---------------------------------------------------------------------
    template <class T>
    HRESULT _DXGIFactory<T>::CreateSwapChain(
        IUnknown *pDevice,
        DXGI_SWAP_CHAIN_DESC *pDesc,
        IDXGISwapChain **ppSwapChain)
    {
        HRESULT result;
        if(g_multiDeviceIsUsed == true) // if multi device - create the multi device type of DXGISwapChain
        {
            DXGISwapChain * newDXGISwapChain = new DXGISwapChain();
            newDXGISwapChain->SetParent(this);
            IDXGIDevice * pDXGIDevice = newDXGISwapChain->GetDxgiDevice();
            if(pDXGIDevice == NULL)
            {
                pDevice->QueryInterface( __uuidof(IDXGIDevice), (void**)&pDXGIDevice );
                newDXGISwapChain->SetDxgiDevice(dynamic_cast<DXGIDevice *>(pDXGIDevice));
                pDXGIDevice->Release();
            }

            newDXGISwapChain->InitInstancesCount(dynamic_cast<DXGIDevice *>(pDXGIDevice)->GetInstanceCount());
            ID3D11Device * pD3d11Device = newDXGISwapChain->GetD3d11Device();
            if(pD3d11Device == NULL)
            {
                newDXGISwapChain->GetDxgiDevice()->QueryInterface( __uuidof(ID3D11Device), (void**)&pD3d11Device );
                newDXGISwapChain->SetD3d11Device(dynamic_cast<D3D11Device *>(pD3d11Device));
                pD3d11Device->Release();
            }
            InitDeviceIdx(0);
            BaseObject::SetSwapChainFromHwnd(pDesc->OutputWindow, newDXGISwapChain);
            HMONITOR windowMonitor = MonitorFromWindow(pDesc->OutputWindow, MONITOR_DEFAULTTONEAREST);
            int windowDeviceIdx = BaseObject::GetMonitorDeviceIndex(windowMonitor);
            newDXGISwapChain->InitDeviceIdx(windowDeviceIdx);


            result = GetInstance(0)->CreateSwapChain(newDXGISwapChain->GetDxgiDevice()->GetInstance(windowDeviceIdx), pDesc, newDXGISwapChain->GetInstancePtr(windowDeviceIdx));
            *ppSwapChain = newDXGISwapChain;
        }
        else // no multi device
        {
            result = GetInstance(0)->CreateSwapChain(pDevice, pDesc, ppSwapChain);
        }
        return result;
    }
    //---------------------------------------------------------------------
    template <class T>
    void _DXGISwapChain<T>::SetDeviceIdx(int newIdx)
    {
        int currentIndex = GetDeviceIdx();

        if(currentIndex != newIdx)
        {
            // first release the current 
            DXGI_SWAP_CHAIN_DESC desc;
            GetInstance(GetDeviceIdx())->GetDesc(&desc);
            int refCount = 0;
            while(GetInstance(GetDeviceIdx())->Release() != 0)
            {
                refCount++;
            }
            *GetInstancePtr(GetDeviceIdx()) = NULL;


            // then update to new index
            __super::SetDeviceIdx(newIdx);

            DXGIFactory * parentAsDXGIFactory = dynamic_cast<DXGIFactory *>(GetParent());
            if(parentAsDXGIFactory != NULL)
            {
                parentAsDXGIFactory->GetInstance(GetDeviceIdx())->CreateSwapChain(
                    GetDxgiDevice()->GetInstance(GetDeviceIdx()), 
                    &desc, 
                    GetInstancePtr(GetDeviceIdx()) );
            }
            else
            {
                DXGIFactory1 * parentAsDXGIFactory1 = dynamic_cast<DXGIFactory1 *>(GetParent());
                if(parentAsDXGIFactory1 != NULL)
                {
                    parentAsDXGIFactory1->GetInstance(0)->CreateSwapChain(
                        GetDxgiDevice()->GetInstance(GetDeviceIdx()), 
                        &desc, 
                        GetInstancePtr(GetDeviceIdx()) );
                }

            }

            while(refCount > 0)
            {
                GetInstance(GetDeviceIdx())->AddRef();
                refCount--;
            }

            // then loop children
            LoopChildrenAndSetDeviceIdx(newIdx);

        }

    }
    //---------------------------------------------------------------------
    template <class T>
    HRESULT _DXGISwapChain<T>::GetBuffer(
        /* [in] */ UINT Buffer,
        /* [in] */ REFIID riid,
        /* [out][in] */ void **ppSurface)
    {
        if (IsEqualGUID(riid, IID_ID3D11Texture2D))
        {
            HRESULT result;
            D3D11Texture2D * newD3D11Texture2D = new D3D11Texture2D();
            newD3D11Texture2D->InitInstancesCount(GetInstanceCount());
            newD3D11Texture2D->SetParent(this);
            newD3D11Texture2D->SetD3d11Device(GetD3d11Device());
            newD3D11Texture2D->SetDxgiDevice(GetDxgiDevice());
            newD3D11Texture2D->InitDeviceIdx(GetDeviceIdx());

            result = GetInstance(GetDeviceIdx())->GetBuffer(Buffer, riid, newD3D11Texture2D->GetInstancePtrAsVoid(GetDeviceIdx()));
            *ppSurface = newD3D11Texture2D;
            return result;
        }

        return -1;
    }

    //---------------------------------------------------------------------
    template <class T>
    void _D3D11Texture2D<T>::SetDeviceIdx(int newIdx)
    {
        int currentIndex = GetDeviceIdx();

        if(currentIndex != newIdx)
        {
            // first release the current 
            int refCount = 0;
            while(GetInstance(GetDeviceIdx())->Release() != 0)
            {
                refCount++;
            }
            *GetInstancePtr(GetDeviceIdx()) = NULL;


            // then update to new index
            __super::SetDeviceIdx(newIdx);

            DXGISwapChain * parentAsDXGISwapChain = dynamic_cast<DXGISwapChain *>(GetParent());
            if(parentAsDXGISwapChain != NULL)
            {
                parentAsDXGISwapChain->GetInstance(GetDeviceIdx())->GetBuffer(
                    0,  // TODO!!!
                    IID_ID3D11Texture2D, 
                    GetInstancePtrAsVoid(GetDeviceIdx()) );
            }

            while(refCount > 0)
            {
                GetInstance(GetDeviceIdx())->AddRef();
                refCount--;
            }

            // then loop children
            LoopChildrenAndSetDeviceIdx(newIdx);

        }

    }
    //---------------------------------------------------------------------
    template <class T>
    HRESULT _D3D11Device<T>::CreateRenderTargetView(
        /* [annotation] */ 
        __in  ID3D11Resource *pResource,
        /* [annotation] */ 
        __in_opt  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
        /* [annotation] */ 
        __out_opt  ID3D11RenderTargetView **ppRTView)
    {

        D3D11Texture2D * tex2D = dynamic_cast<D3D11Texture2D *>(pResource);
        D3D11RenderTargetView * newD3D11RenderTargetView = new D3D11RenderTargetView();
        newD3D11RenderTargetView->InitInstancesCount(GetInstanceCount());
        HRESULT result;

        if(tex2D != NULL && tex2D->GetParent() != NULL)
        {
            newD3D11RenderTargetView->SetParent(tex2D);
            newD3D11RenderTargetView->SetDxgiDevice(tex2D->GetDxgiDevice());
            newD3D11RenderTargetView->SetD3d11Device(tex2D->GetD3d11Device());
            newD3D11RenderTargetView->InitDeviceIdx(tex2D->GetDeviceIdx());

            int i = tex2D->GetDeviceIdx();
            result = GetInstance(i)->CreateRenderTargetView(tex2D->GetInstance(i), pDesc, newD3D11RenderTargetView->GetInstancePtr(i));

            *ppRTView = newD3D11RenderTargetView;
        }
        else
        {
            ResourcePerDeviceListIter iter = _instances.begin();
            ResourcePerDeviceListIter iterE = _instances.end();
            for(int i = 0; iter != iterE ;i++, iter++)
            {
                result = (*iter)->CreateRenderTargetView((static_cast<D3D11Resource*>(pResource))->GetInstance(i), pDesc, newD3D11RenderTargetView->GetInstancePtr(i));
            }
        }
        *ppRTView = newD3D11RenderTargetView;
        return result;

    }


    //---------------------------------------------------------------------
    template <class T>
    void _D3D11RenderTargetView<T>::SetDeviceIdx(int newIdx)
    {
        int currentIndex = GetDeviceIdx();

        if(GetParent() != NULL && currentIndex != newIdx)
        {
            // first release the current 
            D3D11_RENDER_TARGET_VIEW_DESC desc;
            GetInstance(GetDeviceIdx())->GetDesc(&desc);

            int refCount = 0;
            while(GetInstance(GetDeviceIdx())->Release() != 0)
            {
                refCount++;
            }
            *GetInstancePtr(GetDeviceIdx()) = NULL;


            // then update to new index
            __super::SetDeviceIdx(newIdx);

            D3D11Texture2D * parentD3D11Texture2D = dynamic_cast<D3D11Texture2D *>(GetParent());
            if(parentD3D11Texture2D != NULL)
            {
                GetD3d11Device()->GetInstance(GetDeviceIdx())->CreateRenderTargetView(
                    parentD3D11Texture2D->GetInstance(GetDeviceIdx()), 
                    &desc, 
                    GetInstancePtr(GetDeviceIdx()) );

            }

            while(refCount > 0)
            {
                GetInstance(GetDeviceIdx())->AddRef();
                refCount--;
            }

            // then loop children
            LoopChildrenAndSetDeviceIdx(newIdx);

        }
    }

    //---------------------------------------------------------------------
    template <class T>
    void _DXGIFactory<T>::SetDeviceIdx(int newIdx)
    {
        // should not be called...!!!!!!!!!!!!!!!!!!!!!
        // !!!!!!!!!!!!!!!!!!!
        int currentIndex = GetDeviceIdx();

        if(currentIndex != newIdx)
        {
            // first release the current 

            // then update to new index
            __super::SetDeviceIdx(newIdx);
        }
    }


    //---------------------------------------------------------------------

    HRESULT WINAPI CreateMultiDeviceDXGIFactory1(IDXGIFactory1 ** ppFactory)
    {
        BaseObject::SetNumberOfDevices(1);
        DXGIFactory1 * res = new DXGIFactory1();
        *ppFactory = res;
        sDxgiFactory = *ppFactory;
        return CreateDXGIFactory1(IID_IDXGIFactory1, res->GetInstancePtrAsVoid(0));
    }
    //---------------------------------------------------------------------
     

#if 0 // debug code


    HRESULT WINAPI D3D11CreateMultiDevice(
        D3D_DRIVER_TYPE DriverType,
        UINT Flags,
        __in_ecount_opt( FeatureLevels ) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        __out_opt ID3D11Device** ppDevice,
        __out_opt D3D_FEATURE_LEVEL* pFeatureLevel,
        __out_opt ID3D11DeviceContext** ppImmediateContext)
    {
        HRESULT result = -1;
        if(BaseObject::GetFactory() == NULL)
        {   
            BaseObject::SetFactory(static_cast<DXGIFactory1 *>(sDxgiFactory) );

            UINT nAdapter = 0;
            IDXGIAdapter1* pAdapter = NULL;

            while( BaseObject::GetFactory()->EnumAdapters1( nAdapter++, &pAdapter ) != DXGI_ERROR_NOT_FOUND )
            {

                if ( pAdapter )
                {
                    DXGI_ADAPTER_DESC1 desc1;
                    pAdapter->GetDesc1(&desc1);


                    //We add this conditional compilation so we could check software adapters on older DXSDK's (June 2010)  
#if defined(_WIN32_WINNT_WIN8) && _WIN32_WINNT >= _WIN32_WINNT_WIN8
                    if ((desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
#else
                    if ((desc1.Flags & 2) == 0)
#endif
                    {
                        BaseObject::GetAdaptersList().push_back(dynamic_cast<DXGIAdapter1 *>(pAdapter));
                        BaseObject::GetAdaptersList().push_back(dynamic_cast<DXGIAdapter1 *>(pAdapter)); // HACK
                        BaseObject::GetAdaptersList().push_back(dynamic_cast<DXGIAdapter1 *>(pAdapter)); // HACK
                    }
                }
            }


            BaseObject::SetNumberOfDevices(BaseObject::GetAdaptersList().size());
        }
        if(DriverType != D3D_DRIVER_TYPE_HARDWARE && DriverType != D3D_DRIVER_TYPE_UNKNOWN)
        {
            result = D3D11CreateDevice(
                NULL, 
                DriverType, 
                NULL,  
                Flags, 
                pFeatureLevels, 
                FeatureLevels,
                SDKVersion,
                ppDevice,
                pFeatureLevel,
                ppImmediateContext
                );

        }
        else
            // if one device - don't use multi device!
            if(BaseObject::GetNumberOfDevices() == 1)
            {
                IDXGIAdapter1* adapter = BaseObject::GetAdaptersList()[0]->GetInstance(0);

                result = D3D11CreateDevice(
                    adapter, 
                    D3D_DRIVER_TYPE_UNKNOWN, 
                    NULL,  
                    Flags, 
                    pFeatureLevels, 
                    FeatureLevels,
                    SDKVersion,
                    ppDevice,
                    pFeatureLevel,
                    ppImmediateContext
                    );

            }
            else // more then one device - multi 
            {
                g_multiDeviceIsUsed = true;

                D3D11Device * resDevice = new D3D11Device();
                *ppDevice = resDevice;
                D3D11DeviceContext * resDeviceContext = NULL;
                if(ppImmediateContext != NULL)
                {
                    resDeviceContext = new D3D11DeviceContext();
                    *ppImmediateContext = resDeviceContext;
                }
                for(size_t adapterIdx = 0 ; adapterIdx < BaseObject::GetAdaptersList().size() ; adapterIdx++)
                {
                    IDXGIAdapter1* adapter = BaseObject::GetAdaptersList()[adapterIdx]->GetInstance(0);
                    ID3D11DeviceContext ** context  = NULL;
                    if(ppImmediateContext != NULL)
                    {
                        context = resDeviceContext->GetInstancePtr(adapterIdx);
                    }

                    result = D3D11CreateDevice(
                        adapter, 
                        D3D_DRIVER_TYPE_UNKNOWN, 
                        NULL,  
                        Flags, 
                        pFeatureLevels, 
                        FeatureLevels,
                        SDKVersion,
                        resDevice->GetInstancePtr(adapterIdx),
                        pFeatureLevel,
                        context
                        );


                    UINT outIdx = 0;
                    IDXGIOutput * pOutput;
                    while(adapter->EnumOutputs(outIdx, &pOutput) != DXGI_ERROR_NOT_FOUND)
                    {
                        DXGI_OUTPUT_DESC pDesc;
                        pOutput->GetDesc(&pDesc);
                        BaseObject::SetMonitorDeviceIndex(pDesc.Monitor, adapterIdx);
                        ++outIdx;
                    }




                }

            }

            return result;
    }
#else
    //No DEBUG

    HRESULT WINAPI D3D11CreateMultiDevice(
        D3D_DRIVER_TYPE DriverType,
        UINT Flags,
        __in_ecount_opt( FeatureLevels ) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        __out_opt ID3D11Device** ppDevice,
        __out_opt D3D_FEATURE_LEVEL* pFeatureLevel,
        __out_opt ID3D11DeviceContext** ppImmediateContext)
    {
        HRESULT result = -1;
        if(BaseObject::GetFactory() == NULL)
        {   
            BaseObject::SetFactory(static_cast<DXGIFactory1 *>(sDxgiFactory) );

            UINT nAdapter = 0;
            IDXGIAdapter1* pAdapter = NULL;

            while( BaseObject::GetFactory()->EnumAdapters1( nAdapter++, &pAdapter ) != DXGI_ERROR_NOT_FOUND )
            {

                if ( pAdapter )
                {
                    DXGI_ADAPTER_DESC1 desc1;
                    pAdapter->GetDesc1(&desc1);
                    
                    
                    //We add this conditional compilation so we could check software adapters on older DXSDK's (June 2010)
#if defined(_WIN32_WINNT_WIN8) && _WIN32_WINNT >= _WIN32_WINNT_WIN8
                    if ((desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
#else
                    if ((desc1.Flags & 2) == 0)
#endif
                    {
                        BaseObject::GetAdaptersList().push_back(dynamic_cast<DXGIAdapter1 *>(pAdapter));
                    }
                }
            }


            BaseObject::SetNumberOfDevices(BaseObject::GetAdaptersList().size());
        }
        if(DriverType != D3D_DRIVER_TYPE_HARDWARE && DriverType != D3D_DRIVER_TYPE_UNKNOWN)
        {
            result = D3D11CreateDevice(
                NULL, 
                DriverType, 
                NULL,  
                Flags, 
                pFeatureLevels, 
                FeatureLevels,
                SDKVersion,
                ppDevice,
                pFeatureLevel,
                ppImmediateContext
                );

        }
        else
        {
            // count the number of adapters with active outputs
            int numberOfAdaptersWithActiveOutput = 0;
            int adapterWithActiveOutputIndex = -1; // if we have only one active outputs adapter - we will use this
            for(size_t adapterIdx = 0 ; adapterIdx < BaseObject::GetAdaptersList().size() ; adapterIdx++)
            {
                IDXGIAdapter1* adapter = BaseObject::GetAdaptersList()[adapterIdx]->GetInstance(0);

                IDXGIOutput * pOutput;
                for (UINT outIdx = 0; adapter->EnumOutputs(outIdx, &pOutput) != DXGI_ERROR_NOT_FOUND ; outIdx++)
                {
                    DXGI_OUTPUT_DESC pDesc;
                    pOutput->GetDesc(&pDesc);
                    if(pDesc.AttachedToDesktop == TRUE)
                    {
                        adapterWithActiveOutputIndex = adapterIdx;
                        numberOfAdaptersWithActiveOutput++;
                        break;
                    }
                }
            }

            // if one device with outputs - don't use multi device!
            if(numberOfAdaptersWithActiveOutput == 1)
            {
                IDXGIAdapter1* adapter = BaseObject::GetAdaptersList()[adapterWithActiveOutputIndex]->GetInstance(0);

                result = D3D11CreateDevice(
                    adapter, 
                    D3D_DRIVER_TYPE_UNKNOWN, 
                    NULL,  
                    Flags, 
                    pFeatureLevels, 
                    FeatureLevels,
                    SDKVersion,
                    ppDevice,
                    pFeatureLevel,
                    ppImmediateContext
                    );

            }
            else // more then one device - multi 
            {
                g_multiDeviceIsUsed = true;

                D3D11Device * resDevice = new D3D11Device();
                *ppDevice = resDevice;
                D3D11DeviceContext * resDeviceContext = NULL;
                if(ppImmediateContext != NULL)
                {
                    resDeviceContext = new D3D11DeviceContext();
                    *ppImmediateContext = resDeviceContext;
                }
                for(size_t adapterIdx = 0 ; adapterIdx < BaseObject::GetAdaptersList().size() ; adapterIdx++)
                {
                    IDXGIAdapter1* adapter = BaseObject::GetAdaptersList()[adapterIdx]->GetInstance(0);
                    ID3D11DeviceContext ** context  = NULL;
                    if(ppImmediateContext != NULL)
                    {
                        context = resDeviceContext->GetInstancePtr(adapterIdx);
                    }

                    result = D3D11CreateDevice(
                        adapter, 
                        D3D_DRIVER_TYPE_UNKNOWN, 
                        NULL,  
                        Flags, 
                        pFeatureLevels, 
                        FeatureLevels,
                        SDKVersion,
                        resDevice->GetInstancePtr(adapterIdx),
                        pFeatureLevel,
                        context
                        );


                    UINT outIdx = 0;
                    IDXGIOutput * pOutput;
                    while(adapter->EnumOutputs(outIdx, &pOutput) != DXGI_ERROR_NOT_FOUND)
                    {
                        DXGI_OUTPUT_DESC pDesc;
                        pOutput->GetDesc(&pDesc);
                        BaseObject::SetMonitorDeviceIndex(pDesc.Monitor, adapterIdx);
                        ++outIdx;
                    }
                }
            }
        }

        return result;

    }
#endif

    //---------------------------------------------------------------------

    void RenderToDeviceByHwnd( HWND val )
    {
        if(g_multiDeviceIsUsed == true)
        {
            // set active device
            HMONITOR windowMonitor = MonitorFromWindow(val, MONITOR_DEFAULTTONEAREST);
            int activeDeviceIdx = BaseObject::GetMonitorDeviceIndex(windowMonitor);
            BaseObject::SetActiveDeviceIndex(activeDeviceIdx);

            // fix swapchain to use the right device
            DXGISwapChain * swapChain =  BaseObject::GetSwapChainFromHwnd(val);
            swapChain->SetDeviceIdx(activeDeviceIdx);
        }
    }

    //---------------------------------------------------------------------

    void RenderToAllDevices()
    {
        BaseObject::SetActiveDeviceIndex(-1);
    }

    //---------------------------------------------------------------------

    template <class T>
    HRESULT _D3D11DeviceContext<T>::Map(
        /* [annotation] */ 
        __in  ID3D11Resource *pResource,
        /* [annotation] */ 
        __in  UINT Subresource,
        /* [annotation] */ 
        __in  D3D11_MAP MapType,
        /* [annotation] */ 
        __in  UINT MapFlags,
        /* [annotation] */ 
        __out  D3D11_MAPPED_SUBRESOURCE *pMappedResource)
    {
        // only map active device - if you want to copy to the others - use CopyBufferNoneActiveDevices

        HRESULT result;
        size_t sizeOfBuffer = 0;

        D3D11_RESOURCE_DIMENSION rType;
        pResource->GetType(&rType);
        ID3D11DeviceContext * activeDeviceInstance = GetAnyDeviceInstance();

        switch(rType)
        {
        case D3D11_RESOURCE_DIMENSION_BUFFER:
            {
                D3D11Buffer * d3d11Resource = static_cast<D3D11Buffer*>(pResource);

                result = activeDeviceInstance->Map(d3d11Resource->GetAnyDeviceInstance(), Subresource, MapType, MapFlags, pMappedResource);
                d3d11Resource->SetMapType(MapType);

                sizeOfBuffer = d3d11Resource->CalcSizeOfMappedMemory(
                    Subresource, 
                    pMappedResource->RowPitch, 
                    pMappedResource->DepthPitch
                    );

                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                D3D11Texture1D * d3d11Resource = static_cast<D3D11Texture1D*>(pResource);
                result = activeDeviceInstance->Map(d3d11Resource->GetAnyDeviceInstance(), Subresource, MapType, MapFlags, pMappedResource);
                d3d11Resource->SetMapType(MapType);

                sizeOfBuffer = d3d11Resource->CalcSizeOfMappedMemory(
                    Subresource, 
                    pMappedResource->RowPitch, 
                    pMappedResource->DepthPitch
                    );

                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                D3D11Texture2D * d3d11Resource = static_cast<D3D11Texture2D*>(pResource);
                result = activeDeviceInstance->Map(d3d11Resource->GetAnyDeviceInstance(), Subresource, MapType, MapFlags, pMappedResource);
                d3d11Resource->SetMapType(MapType);

                sizeOfBuffer = d3d11Resource->CalcSizeOfMappedMemory(
                    Subresource, 
                    pMappedResource->RowPitch, 
                    pMappedResource->DepthPitch
                    );

                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                D3D11Texture3D * d3d11Resource = static_cast<D3D11Texture3D*>(pResource);
                result = activeDeviceInstance->Map(d3d11Resource->GetAnyDeviceInstance(), Subresource, MapType, MapFlags, pMappedResource);
                d3d11Resource->SetMapType(MapType);

                sizeOfBuffer = d3d11Resource->CalcSizeOfMappedMemory(
                    Subresource, 
                    pMappedResource->RowPitch, 
                    pMappedResource->DepthPitch
                    );

                break;
            }

        }


        return result;
    }

    //---------------------------------------------------------------------

    template <class T>
    void _D3D11DeviceContext<T>::Unmap(
        /* [annotation] */ 
        __in  ID3D11Resource *pResource,
        /* [annotation] */ 
        __in  UINT Subresource)
    {
        // only unmap active device - if you want to copy to the others - use CopyBufferNoneActiveDevices

        D3D11_RESOURCE_DIMENSION rType;
        pResource->GetType(&rType);

        ID3D11DeviceContext * activeDeviceInstance = GetAnyDeviceInstance();

        switch(rType)
        {
        case D3D11_RESOURCE_DIMENSION_BUFFER:
            {
                D3D11Buffer * d3d11Resource = static_cast<D3D11Buffer*>(pResource);
                activeDeviceInstance->Unmap(d3d11Resource->GetAnyDeviceInstance(), Subresource);
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                D3D11Texture1D * d3d11Resource = static_cast<D3D11Texture1D*>(pResource);
                activeDeviceInstance->Unmap(d3d11Resource->GetAnyDeviceInstance(), Subresource);
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                D3D11Texture2D * d3d11Resource = static_cast<D3D11Texture2D*>(pResource);
                activeDeviceInstance->Unmap(d3d11Resource->GetAnyDeviceInstance(), Subresource);
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                D3D11Texture3D * d3d11Resource = static_cast<D3D11Texture3D*>(pResource);
                activeDeviceInstance->Unmap(d3d11Resource->GetAnyDeviceInstance(), Subresource);
                break;
            }
        }
    }

    //---------------------------------------------------------------------

    template <class T>
    size_t _D3D11Resource<T>::GetSizeOfMappedMemory( __in  UINT Subresource)
    {
        return _SizeOfSubResource[Subresource];
    }

    //---------------------------------------------------------------------

    template <class T>
    size_t _D3D11Buffer<T>::CalcSizeOfMappedMemory(
        /* [annotation] */
        __in  UINT Subresource,
        /* [annotation] */
        __in  UINT RowPitch,
        /* [annotation] */
        __in  UINT DepthPitch)
    {
        size_t result = 0;
        if(_SizeOfSubResource.find(Subresource) != _SizeOfSubResource.end())
        {
            result = _SizeOfSubResource[Subresource];
        }
        else
        {
            D3D11_BUFFER_DESC pDesc;
            GetDesc(&pDesc);
            size_t resourceSize = pDesc.ByteWidth;
            result = _SizeOfSubResource[Subresource] = resourceSize;
        }

        return result;

    }

    //---------------------------------------------------------------------

    template <class T>
    size_t _D3D11Texture1D<T>::CalcSizeOfMappedMemory(
        /* [annotation] */
        __in  UINT Subresource,
        /* [annotation] */
        __in  UINT RowPitch,
        /* [annotation] */
        __in  UINT DepthPitch)
    {
        size_t result = 0;
        if(_SizeOfSubResource.find(Subresource) != _SizeOfSubResource.end())
        {
            result = _SizeOfSubResource[Subresource];
        }
        else
        {
            size_t resourceSize = DepthPitch;
            result = _SizeOfSubResource[Subresource] = resourceSize;
        }

        return result;
    }

    //---------------------------------------------------------------------

    template <class T>
    size_t _D3D11Texture2D<T>::CalcSizeOfMappedMemory(
        /* [annotation] */
        __in  UINT Subresource,
        /* [annotation] */
        __in  UINT RowPitch,
        /* [annotation] */
        __in  UINT DepthPitch)
    {
        size_t result = 0;
        if(_SizeOfSubResource.find(Subresource) != _SizeOfSubResource.end())
        {
            result = _SizeOfSubResource[Subresource];
        }
        else
        {
            D3D11_TEXTURE2D_DESC pDesc;
            GetDesc(&pDesc);
            size_t resourceSize = DepthPitch;
            result = _SizeOfSubResource[Subresource] = resourceSize;
        }

        return result;
    }

    //---------------------------------------------------------------------

    template <class T>
    size_t _D3D11Texture3D<T>::CalcSizeOfMappedMemory(
        /* [annotation] */
        __in  UINT Subresource,
        /* [annotation] */
        __in  UINT RowPitch,
        /* [annotation] */
        __in  UINT DepthPitch)
    {
        size_t result = 0;
        if(_SizeOfSubResource.find(Subresource) != _SizeOfSubResource.end())
        {
            result = _SizeOfSubResource[Subresource];
        }
        else
        {
            D3D11_TEXTURE3D_DESC pDesc;
            GetDesc(&pDesc);
            size_t resourceSize = DepthPitch * pDesc.Depth;
            result = _SizeOfSubResource[Subresource] = resourceSize;
        }

        return result;
    }


    //---------------------------------------------------------------------

    template <class T>
    void _D3D11Resource<T>::CopyBufferToSpecificDevice(
        /* [annotation] */ 
        __in D3D11_MAPPED_SUBRESOURCE *pMappedResource, 
        /* [annotation] */ 
        __in ID3D11DeviceContext * context, 
        /* [annotation] */ 
        __in  UINT Subresource, 
        /* [annotation] */ 
        __in size_t instanceIdx,
        /* [annotation] */ 
        __in  UINT Offset,
        /* [annotation] */ 
        __in  UINT Length
        )
    {
        if(Length == 0)
        {
            Offset = 0;
            Length = GetSizeOfMappedMemory(Subresource);
        }
        D3D11_MAPPED_SUBRESOURCE mappedData;
        context->Map(GetInstance(instanceIdx), Subresource, _mapType, 0, &mappedData);
        if(_mapType != D3D11_MAP_READ)
        {
            memcpy((char *)mappedData.pData + Offset, (char *)pMappedResource->pData + Offset, Length);
        }
        context->Unmap(GetInstance(instanceIdx), Subresource);
    }


    //---------------------------------------------------------------------

    template <class T>
    void _D3D11DeviceContext<T>::CopyBufferNoneActiveDevices(
        /* [annotation] */ 
        __in D3D11_MAPPED_SUBRESOURCE *pMappedResource,
        /* [annotation] */ 
        __in  ID3D11Resource *pResource,
        /* [annotation] */ 
        __in  UINT Subresource,
        /* [annotation] */ 
        __in  UINT Offset,
        /* [annotation] */ 
        __in  UINT Length
        )
    {
        // copy to all but the active device
        ResourcePerDeviceListIter iter = _instances.begin();
        ResourcePerDeviceListIter iterE = _instances.end();

        D3D11_RESOURCE_DIMENSION rType;
        pResource->GetType(&rType);
        switch(rType)
        {
        case D3D11_RESOURCE_DIMENSION_BUFFER:
            {
                D3D11Buffer * d3d11Resource = static_cast<D3D11Buffer*>(pResource);
                for(int i = 0; iter != iterE ;i++, iter++)
                {
                    if((*iter) != NULL && GetAnyDeviceInstance() != GetInstance(i))
                    {
                        d3d11Resource->CopyBufferToSpecificDevice(pMappedResource, (*iter), Subresource, i, Offset, Length);
                    }
                }
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                D3D11Texture1D * d3d11Resource = static_cast<D3D11Texture1D*>(pResource);
                for(int i = 0; iter != iterE ;i++, iter++)
                {
                    if((*iter) != NULL && GetAnyDeviceInstance() != GetInstance(i))
                    {
                        d3d11Resource->CopyBufferToSpecificDevice(pMappedResource, (*iter), Subresource, i, Offset, Length);
                    }
                }
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                D3D11Texture2D * d3d11Resource = static_cast<D3D11Texture2D*>(pResource);
                for(int i = 0; iter != iterE ;i++, iter++)
                {
                    if((*iter) != NULL && GetAnyDeviceInstance() != GetInstance(i))
                    {
                        d3d11Resource->CopyBufferToSpecificDevice(pMappedResource, (*iter), Subresource, i, Offset, Length);
                    }
                }
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                D3D11Texture3D * d3d11Resource = static_cast<D3D11Texture3D*>(pResource);

                for(int i = 0; iter != iterE ;i++, iter++)
                {
                    if((*iter) != NULL &&GetAnyDeviceInstance() != GetInstance(i))
                    {
                        d3d11Resource->CopyBufferToSpecificDevice(pMappedResource, (*iter), Subresource, i, Offset, Length);
                    }           
                }
                break;
            }
        }
    }

    //---------------------------------------------------------------------
    
    void CopyBufferNoneActiveDevices(
        /* [annotation] */ 
        __in ID3D11DeviceContext *pDeviceContext,
        /* [annotation] */ 
        __in D3D11_MAPPED_SUBRESOURCE *pMappedResource,
        /* [annotation] */ 
        __in  ID3D11Resource *pResource,
        /* [annotation] */ 
        __in  UINT Subresource,
        /* [annotation] */ 
        __in  UINT Offset,
        /* [annotation] */ 
        __in  UINT Length,
        /* [annotation] */ 
        __in  BOOL CopyOnlyToActiveDevice
        )
    {
        if( (g_multiDeviceIsUsed == false)
            || 
            (BaseObject::GetActiveDeviceIndex() != -1 && CopyOnlyToActiveDevice == TRUE) 
            )
        {
            return;
        }

        D3D11DeviceContext * d3d11DeviceContext = static_cast<D3D11DeviceContext*>(pDeviceContext);


        d3d11DeviceContext->CopyBufferNoneActiveDevices(
            pMappedResource,
            pResource,
            Subresource,
            Offset,
            Length
            );
    }

    //---------------------------------------------------------------------

    int GetWindowDeviceIndex( HWND val )
    {
        HMONITOR windowMonitor = MonitorFromWindow(val, MONITOR_DEFAULTTONEAREST);
        return BaseObject::GetMonitorDeviceIndex(windowMonitor);
    }

    //---------------------------------------------------------------------
    template <class T>
    HRESULT _D3D11Device<T>::CreateDeferredContext(
        UINT ContextFlags,
        /* [annotation] */ 
        __out_opt  ID3D11DeviceContext **ppDeferredContext)
    {
        HRESULT result;
        D3D11DeviceContext * newD3D11DeviceContext = new D3D11DeviceContext();
        newD3D11DeviceContext->InitInstancesCount(GetInstanceCount());
        newD3D11DeviceContext->SetDevice(this);

        ResourcePerDeviceListIter iter = _instances.begin();
        ResourcePerDeviceListIter iterE = _instances.end();
        for(int i = 0; iter != iterE ;i++, iter++)
        {
            if((*iter) != NULL)
            {
                result = (*iter)->CreateDeferredContext(ContextFlags, newD3D11DeviceContext->GetInstancePtr(i));
            }
        }

        *ppDeferredContext = newD3D11DeviceContext;
        return result;
    }

    //---------------------------------------------------------------------
    template <class T>
    void _D3D11Device<T>::GetImmediateContext(
        /* [annotation] */ 
        __out  ID3D11DeviceContext **ppImmediateContext)
    {
        D3D11DeviceContext * newD3D11DeviceContext = new D3D11DeviceContext();
        newD3D11DeviceContext->InitInstancesCount(GetInstanceCount());
        newD3D11DeviceContext->SetDevice(this);

        ResourcePerDeviceListIter iter = _instances.begin();
        ResourcePerDeviceListIter iterE = _instances.end();
        for(int i = 0; iter != iterE ;i++, iter++)
        {
            if((*iter) != NULL)
            {
                (*iter)->GetImmediateContext( newD3D11DeviceContext->GetInstancePtr(i));
            }
        }

        *ppImmediateContext = newD3D11DeviceContext;
    }

    //---------------------------------------------------------------------
    template <class T>
    _D3D11DeviceContext<T>::_D3D11DeviceContext()
    {       
        _device = NULL;
    }

    //---------------------------------------------------------------------
    template <class T>
    void _D3D11DeviceContext<T>::OMGetRenderTargets(
        /* [annotation] */ 
        __in_range( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumViews,
        /* [annotation] */ 
        __out_ecount_opt(NumViews)  ID3D11RenderTargetView **ppRenderTargetViews,
        /* [annotation] */ 
        __out_opt  ID3D11DepthStencilView **ppDepthStencilView)
    {
        bool firstLoop = true;
        ResourcePerDeviceListIter iter = _instances.begin();
        ResourcePerDeviceListIter iterE = _instances.end();
        for(int i = 0; iter != iterE ;i++, iter++)
        {
            if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
            {
                continue;
            }
                
            if((*iter) != NULL)
            {
                static ID3D11RenderTargetView * tempRenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
                static ID3D11DepthStencilView * tempDepthStencilView[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
                (*iter)->OMGetRenderTargets(NumViews, tempRenderTargetViews, tempDepthStencilView);
                if(firstLoop == true)
                {
                    firstLoop = false;
                    CreateMultiDeviceArrayBySingleDeviceArray<D3D11RenderTargetView, ID3D11RenderTargetView>(tempRenderTargetViews, ppRenderTargetViews, NumViews, GetInstanceCount());
                    CreateMultiDeviceArrayBySingleDeviceArray<D3D11DepthStencilView, ID3D11DepthStencilView>(tempDepthStencilView, ppDepthStencilView, NumViews, GetInstanceCount());
                }
                ConvertToMultiDeviceArray<D3D11RenderTargetView, ID3D11RenderTargetView>(tempRenderTargetViews, ppRenderTargetViews, NumViews, i);
                ConvertToMultiDeviceArray<D3D11DepthStencilView, ID3D11DepthStencilView>(tempDepthStencilView, ppDepthStencilView, NumViews, i);

            }
        }
    }
    //---------------------------------------------------------------------
    template <class T>
    void _D3D11DeviceContext<T>::OMGetRenderTargetsAndUnorderedAccessViews(
        /* [annotation] */ 
        __in_range( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumRTVs,
        /* [annotation] */ 
        __out_ecount_opt(NumRTVs)  ID3D11RenderTargetView **ppRenderTargetViews,
        /* [annotation] */ 
        __out_opt  ID3D11DepthStencilView **ppDepthStencilView,
        /* [annotation] */ 
        __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - 1 )  UINT UAVStartSlot,
        /* [annotation] */ 
        __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - UAVStartSlot )  UINT NumUAVs,
        /* [annotation] */ 
        __out_ecount_opt(NumUAVs)  ID3D11UnorderedAccessView **ppUnorderedAccessViews)
    {
        bool firstLoop = true;
        ResourcePerDeviceListIter iter = _instances.begin();
        ResourcePerDeviceListIter iterE = _instances.end();
        for(int i = 0; iter != iterE ;i++, iter++)
        {
            if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
            {
                continue;
            }

            if((*iter) != NULL)
            {
                static ID3D11RenderTargetView * tempRenderTargetViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
                static ID3D11DepthStencilView * tempDepthStencilView[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
                static ID3D11UnorderedAccessView * tempUnorderedAccessViews[D3D11_PS_CS_UAV_REGISTER_COUNT];
                (*iter)->OMGetRenderTargetsAndUnorderedAccessViews(NumRTVs, tempRenderTargetViews, tempDepthStencilView, UAVStartSlot, NumUAVs, tempUnorderedAccessViews);
                if(firstLoop == true)
                {
                    firstLoop = false;
                    CreateMultiDeviceArrayBySingleDeviceArray<D3D11RenderTargetView, ID3D11RenderTargetView>(tempRenderTargetViews, ppRenderTargetViews, NumRTVs, GetInstanceCount());
                    CreateMultiDeviceArrayBySingleDeviceArray<D3D11DepthStencilView, ID3D11DepthStencilView>(tempDepthStencilView, ppDepthStencilView, NumRTVs, GetInstanceCount());
                    CreateMultiDeviceArrayBySingleDeviceArray<D3D11UnorderedAccessView, ID3D11UnorderedAccessView>(tempUnorderedAccessViews, ppUnorderedAccessViews, NumUAVs, GetInstanceCount());
                }
                ConvertToMultiDeviceArray<D3D11RenderTargetView, ID3D11RenderTargetView>(tempRenderTargetViews, ppRenderTargetViews, NumRTVs, i);
                ConvertToMultiDeviceArray<D3D11DepthStencilView, ID3D11DepthStencilView>(tempDepthStencilView, ppDepthStencilView, NumRTVs, i);
                ConvertToMultiDeviceArray<D3D11UnorderedAccessView, ID3D11UnorderedAccessView>(tempUnorderedAccessViews, ppUnorderedAccessViews, NumUAVs, i);

            }
        }
    }
//---------------------------------------------------------------------

std::map<HANDLE, std::vector<HANDLE>> g_openDevices;
std::map<HANDLE, std::vector<HANDLE>> g_registeredObjects;


#if MULTI_DEVICE_WRAP_OPENGL_INTEROP == 1
//---------------------------------------------------------------------

HANDLE WINAPI D3D11MultiDevice_wglDXOpenDeviceNV( void* dxDevice )
{
    if(D3D11MultiDevice::g_multiDeviceIsUsed == false)
        return wglDXOpenDeviceNV(dxDevice);

    D3D11MultiDevice::D3D11Device * multiDevice = static_cast<D3D11MultiDevice::D3D11Device *>(dxDevice);
    HANDLE result = 0;
    for(int i = 0; i < multiDevice->GetInstanceCount() ;i++)
    {
        ID3D11Device * curDevice = multiDevice->GetInstance(i);
        if(curDevice != NULL)
        {
            HANDLE curResult = wglDXOpenDeviceNV(curDevice);
            if(result == 0)
            {
                result = curResult;
            }
            g_openDevices[result].push_back(curResult);
        }
    }

    return result;
}


//---------------------------------------------------------------------

BOOL WINAPI D3D11MultiDevice_wglDXCloseDeviceNV( HANDLE hDevice )
{
    if(D3D11MultiDevice::g_multiDeviceIsUsed == false)
        return wglDXCloseDeviceNV(hDevice);

    BOOL result = FALSE;
    std::vector<HANDLE> & handles = g_openDevices[hDevice];
    for(int deviceIndex = 0; deviceIndex < handles.size() ;deviceIndex++)
    {
        HANDLE curDevice = handles[deviceIndex];
        result = wglDXCloseDeviceNV(curDevice);
    }

    g_openDevices.erase(hDevice);

    return result;
}

//---------------------------------------------------------------------

HANDLE WINAPI D3D11MultiDevice_wglDXRegisterObjectNV( HANDLE hDevice, void* dxObject, GLuint name, GLenum type, GLenum access )
{
    using namespace D3D11MultiDevice;

    if(g_multiDeviceIsUsed == false)
        return wglDXRegisterObjectNV(hDevice, dxObject, name, type, access);

    HANDLE result = FALSE;
    std::vector<HANDLE> & handles = g_openDevices[hDevice];
    for(int deviceIndex = 0; deviceIndex < handles.size() ;deviceIndex++)
    {
        HANDLE curDevice = handles[deviceIndex];
        ID3D11Resource *pResource = static_cast<ID3D11Resource *>(dxObject);

        D3D11_RESOURCE_DIMENSION rType;
        pResource->GetType(&rType);
        void* curDxObject = NULL;
        switch(rType)
        {
        case D3D11_RESOURCE_DIMENSION_BUFFER:
            {
                D3D11Buffer * d3d11Resource = static_cast<D3D11Buffer*>(pResource);
                curDxObject = (void *)d3d11Resource->GetInstance(deviceIndex);
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                D3D11Texture1D * d3d11Resource = static_cast<D3D11Texture1D*>(pResource);
                curDxObject = (void *)d3d11Resource->GetInstance(deviceIndex);
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                D3D11Texture2D * d3d11Resource = static_cast<D3D11Texture2D*>(pResource);
                curDxObject = (void *)d3d11Resource->GetInstance(deviceIndex);
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                D3D11Texture3D * d3d11Resource = static_cast<D3D11Texture3D*>(pResource);
                curDxObject = (void *)d3d11Resource->GetInstance(deviceIndex);
                break;
            }
        }

        HANDLE curResult = wglDXRegisterObjectNV(curDevice, curDxObject, name, type, access);
        if(result == 0)
        {
            result = curResult;
        }

        g_registeredObjects[result].push_back(curResult);

    }

    return result;
}


//---------------------------------------------------------------------

BOOL WINAPI D3D11MultiDevice_wglDXUnregisterObjectNV( HANDLE hDevice, HANDLE hObject )
{
    if(D3D11MultiDevice::g_multiDeviceIsUsed == false)
        return wglDXUnregisterObjectNV(hDevice, hObject);

    BOOL result = FALSE;
    std::vector<HANDLE> & deviceHandles = g_openDevices[hDevice];
    std::vector<HANDLE> & objectHandles = g_registeredObjects[hObject];
    for(int deviceIndex = 0; deviceIndex < deviceHandles.size() ;deviceIndex++)
    {
        HANDLE curDevice = deviceHandles[deviceIndex];
        HANDLE curObject = objectHandles[deviceIndex];
        result = wglDXUnregisterObjectNV(curDevice, curObject);
    }

    g_registeredObjects.erase(hObject);

    return result;
}

//---------------------------------------------------------------------

BOOL WINAPI D3D11MultiDevice_wglDXLockObjectsNV( HANDLE hDevice, GLint count, HANDLE* hObjects )
{
    if(D3D11MultiDevice::g_multiDeviceIsUsed == false)
        return wglDXLockObjectsNV(hDevice, count, hObjects);

    BOOL result = FALSE;
    std::vector<HANDLE> & deviceHandles = g_openDevices[hDevice];
    for(int deviceIndex = 0; deviceIndex < deviceHandles.size() ;deviceIndex++)
    {
        HANDLE curDevice = deviceHandles[deviceIndex];
        for(int objectIndex = 0; objectIndex < count ;objectIndex++)
        {
            std::vector<HANDLE> & objectHandles = g_registeredObjects[hObjects[objectIndex]];

            HANDLE curObject = objectHandles[deviceIndex];
            result =  wglDXLockObjectsNV(curDevice, 1, &curObject);
        }
    }

    return result;
}

//---------------------------------------------------------------------

BOOL WINAPI D3D11MultiDevice_wglDXUnlockObjectsNV( HANDLE hDevice, GLint count, HANDLE* hObjects )
{
    if(D3D11MultiDevice::g_multiDeviceIsUsed == false)
        return wglDXUnlockObjectsNV(hDevice, count, hObjects);

    BOOL result = FALSE;
    std::vector<HANDLE> & deviceHandles = g_openDevices[hDevice];
    for(int deviceIndex = 0; deviceIndex < deviceHandles.size() ;deviceIndex++)
    {
        HANDLE curDevice = deviceHandles[deviceIndex];
        for(int objectIndex = 0; objectIndex < count ;objectIndex++)
        {
            std::vector<HANDLE> & objectHandles = g_registeredObjects[hObjects[objectIndex]];

            HANDLE curObject = objectHandles[deviceIndex];
            result = wglDXUnlockObjectsNV(curDevice, 1, &curObject);
        }
    }

    return result;
}

//---------------------------------------------------------------------

BOOL WINAPI D3D11MultiDevice_wglDXObjectAccessNV( HANDLE hObject, GLenum access )
{
    if(D3D11MultiDevice::g_multiDeviceIsUsed == false)
        return wglDXObjectAccessNV(hObject, access);

    BOOL result = FALSE;
    std::vector<HANDLE> & objectHandles = g_registeredObjects[hObject];
    for(int objectIndex = 0; objectIndex < objectHandles.size() ;objectIndex++)
    {
        HANDLE curObject = objectHandles[objectIndex];

        result = wglDXObjectAccessNV(curObject, access);        
    }

    return result;

}


//---------------------------------------------------------------------

BOOL WINAPI D3D11MultiDevice_wglDXSetResourceShareHandleNV( void* dxObject, HANDLE shareHandle )
{
    if(D3D11MultiDevice::g_multiDeviceIsUsed == false)
        return wglDXSetResourceShareHandleNV( dxObject, shareHandle);

    // TODO
    return FALSE;
}

#endif
//---------------------------------------------------------------------

//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIObject
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIObject()
{
  DXGIObject linkFixDXGIObject;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIObject<T>::SetPrivateData(
            /* [in] */ REFGUID Name,
            /* [in] */ UINT DataSize,
            /* [in] */ const void *pData)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetPrivateData(Name, DataSize, pData);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIObject<T>::SetPrivateDataInterface(
            /* [in] */ REFGUID Name,
            /* [in] */ const IUnknown *pUnknown)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetPrivateDataInterface(Name, pUnknown == NULL ? NULL : (static_cast<const Unknown*>(pUnknown))->GetInstance(i));
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIObject<T>::GetPrivateData(
            /* [in] */ REFGUID Name,
            /* [out][in] */ UINT *pDataSize,
            /* [out] */ void *pData)
{
  return GetAnyDeviceInstance()->GetPrivateData(Name, pDataSize, pData);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIObject<T>::GetParent(
            /* [in] */ REFIID riid,
            /* [retval][out] */ void **ppParent)
{
  return GetAnyDeviceInstance()->GetParent(riid, ppParent);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIDeviceSubObject
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIDeviceSubObject()
{
  DXGIDeviceSubObject linkFixDXGIDeviceSubObject;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIDeviceSubObject<T>::GetDevice(
            /* [in] */ REFIID riid,
            /* [retval][out] */ void **ppDevice)
{
  return GetAnyDeviceInstance()->GetDevice(riid, ppDevice);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIResource
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIResource()
{
  DXGIResource linkFixDXGIResource;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIResource<T>::GetSharedHandle(
            /* [out] */ HANDLE *pSharedHandle)
{
  return GetAnyDeviceInstance()->GetSharedHandle(pSharedHandle);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIResource<T>::GetUsage(
            /* [out] */ DXGI_USAGE *pUsage)
{
  return GetAnyDeviceInstance()->GetUsage(pUsage);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIResource<T>::SetEvictionPriority(
            /* [in] */ UINT EvictionPriority)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetEvictionPriority(EvictionPriority);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIResource<T>::GetEvictionPriority(
            /* [retval][out] */ UINT *pEvictionPriority)
{
  return GetAnyDeviceInstance()->GetEvictionPriority(pEvictionPriority);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIKeyedMutex
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIKeyedMutex()
{
  DXGIKeyedMutex linkFixDXGIKeyedMutex;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIKeyedMutex<T>::AcquireSync(
            /* [in] */ UINT64 Key,
            /* [in] */ DWORD dwMilliseconds)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->AcquireSync(Key, dwMilliseconds);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIKeyedMutex<T>::ReleaseSync(
            /* [in] */ UINT64 Key)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->ReleaseSync(Key);
    }
  }

  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGISurface
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGISurface()
{
  DXGISurface linkFixDXGISurface;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISurface<T>::GetDesc(
            /* [out] */ DXGI_SURFACE_DESC *pDesc)
{
  return GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISurface<T>::Map(
            /* [out] */ DXGI_MAPPED_RECT *pLockedRect,
            /* [in] */ UINT MapFlags)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->Map(pLockedRect, MapFlags);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISurface<T>::Unmap( void )
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->Unmap();
    }
  }

  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGISurface1
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGISurface1()
{
  DXGISurface1 linkFixDXGISurface1;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISurface1<T>::GetDC(
            /* [in] */ BOOL Discard,
            /* [out] */ HDC *phdc)
{
  return GetAnyDeviceInstance()->GetDC(Discard, phdc);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISurface1<T>::ReleaseDC(
            /* [in] */ RECT *pDirtyRect)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->ReleaseDC(pDirtyRect);
    }
  }

  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIAdapter
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIAdapter()
{
  DXGIAdapter linkFixDXGIAdapter;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIAdapter<T>::EnumOutputs(
            /* [in] */ UINT Output,
            /* [out][in] */ IDXGIOutput **ppOutput)
{
  HRESULT result;
  DXGIOutput * newDXGIOutput = new DXGIOutput();
  newDXGIOutput->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->EnumOutputs(Output, newDXGIOutput->GetInstancePtr(i));
    }
  }

  *ppOutput = newDXGIOutput;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIAdapter<T>::GetDesc(
            /* [out] */ DXGI_ADAPTER_DESC *pDesc)
{
  return GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIAdapter<T>::CheckInterfaceSupport(
            /* [in] */ REFGUID InterfaceName,
            /* [out] */ LARGE_INTEGER *pUMDVersion)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->CheckInterfaceSupport(InterfaceName, pUMDVersion);
    }
  }

  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIOutput
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIOutput()
{
  DXGIOutput linkFixDXGIOutput;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::GetDesc(
            /* [out] */ DXGI_OUTPUT_DESC *pDesc)
{
  return GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::GetDisplayModeList(
            /* [in] */ DXGI_FORMAT EnumFormat,
            /* [in] */ UINT Flags,
            /* [out][in] */ UINT *pNumModes,
            /* [out] */ DXGI_MODE_DESC *pDesc)
{
  return GetAnyDeviceInstance()->GetDisplayModeList(EnumFormat, Flags, pNumModes, pDesc);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::FindClosestMatchingMode(
            /* [in] */ const DXGI_MODE_DESC *pModeToMatch,
            /* [out] */ DXGI_MODE_DESC *pClosestMatch,
            /* [in] */ IUnknown *pConcernedDevice)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->FindClosestMatchingMode(pModeToMatch, pClosestMatch, pConcernedDevice == NULL ? NULL : (static_cast<Unknown*>(pConcernedDevice))->GetInstance(i));
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::WaitForVBlank( void )
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->WaitForVBlank();
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::TakeOwnership(
            /* [in] */ IUnknown *pDevice,
            BOOL Exclusive)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->TakeOwnership(pDevice == NULL ? NULL : (static_cast<Unknown*>(pDevice))->GetInstance(i), Exclusive);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
void _DXGIOutput<T>::ReleaseOwnership( void )
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->ReleaseOwnership();
    }
  }

}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::GetGammaControlCapabilities(
            /* [out] */ DXGI_GAMMA_CONTROL_CAPABILITIES *pGammaCaps)
{
  return GetAnyDeviceInstance()->GetGammaControlCapabilities(pGammaCaps);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::SetGammaControl(
            /* [in] */ const DXGI_GAMMA_CONTROL *pArray)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetGammaControl(pArray);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::GetGammaControl(
            /* [out] */ DXGI_GAMMA_CONTROL *pArray)
{
  return GetAnyDeviceInstance()->GetGammaControl(pArray);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::SetDisplaySurface(
            /* [in] */ IDXGISurface *pScanoutSurface)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetDisplaySurface(pScanoutSurface == NULL ? NULL : (static_cast<DXGISurface*>(pScanoutSurface))->GetInstance(i));
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::GetDisplaySurfaceData(
            /* [in] */ IDXGISurface *pDestination)
{
  return GetAnyDeviceInstance()->GetDisplaySurfaceData(pDestination);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIOutput<T>::GetFrameStatistics(
            /* [out] */ DXGI_FRAME_STATISTICS *pStats)
{
  return GetAnyDeviceInstance()->GetFrameStatistics(pStats);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGISwapChain
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGISwapChain()
{
  DXGISwapChain linkFixDXGISwapChain;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISwapChain<T>::Present(
            /* [in] */ UINT SyncInterval,
            /* [in] */ UINT Flags)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->Present(SyncInterval, Flags);
    }
  }

  return result;
}
//---------------------------------------------------------------------
#if 0
template <class T>
HRESULT _DXGISwapChain<T>::GetBuffer(
            /* [in] */ UINT Buffer,
            /* [in] */ REFIID riid,
            /* [out][in] */ void **ppSurface)
{
  return GetAnyDeviceInstance()->GetBuffer(Buffer, riid, ppSurface);
}
#endif
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISwapChain<T>::SetFullscreenState(
            /* [in] */ BOOL Fullscreen,
            /* [in] */ IDXGIOutput *pTarget)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetFullscreenState(Fullscreen, pTarget == NULL ? NULL : (static_cast<DXGIOutput*>(pTarget))->GetInstance(i));
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISwapChain<T>::GetFullscreenState(
            /* [out] */ BOOL *pFullscreen,
            /* [out] */ IDXGIOutput **ppTarget)
{
  return GetAnyDeviceInstance()->GetFullscreenState(pFullscreen, ppTarget);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISwapChain<T>::GetDesc(
            /* [out] */ DXGI_SWAP_CHAIN_DESC *pDesc)
{
  return GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISwapChain<T>::ResizeBuffers(
            /* [in] */ UINT BufferCount,
            /* [in] */ UINT Width,
            /* [in] */ UINT Height,
            /* [in] */ DXGI_FORMAT NewFormat,
            /* [in] */ UINT SwapChainFlags)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISwapChain<T>::ResizeTarget(
            /* [in] */ const DXGI_MODE_DESC *pNewTargetParameters)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->ResizeTarget(pNewTargetParameters);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISwapChain<T>::GetContainingOutput(
            IDXGIOutput **ppOutput)
{
  return GetAnyDeviceInstance()->GetContainingOutput(ppOutput);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISwapChain<T>::GetFrameStatistics(
            /* [out] */ DXGI_FRAME_STATISTICS *pStats)
{
  return GetAnyDeviceInstance()->GetFrameStatistics(pStats);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGISwapChain<T>::GetLastPresentCount(
            /* [out] */ UINT *pLastPresentCount)
{
  return GetAnyDeviceInstance()->GetLastPresentCount(pLastPresentCount);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIFactory
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIFactory()
{
  DXGIFactory linkFixDXGIFactory;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIFactory<T>::EnumAdapters(
            /* [in] */ UINT Adapter,
            /* [out] */ IDXGIAdapter **ppAdapter)
{
  HRESULT result;
  DXGIAdapter * newDXGIAdapter = new DXGIAdapter();
  newDXGIAdapter->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->EnumAdapters(Adapter, newDXGIAdapter->GetInstancePtr(i));
    }
  }

  *ppAdapter = newDXGIAdapter;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIFactory<T>::MakeWindowAssociation(
            HWND WindowHandle,
            UINT Flags)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->MakeWindowAssociation(WindowHandle, Flags);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIFactory<T>::GetWindowAssociation(
            HWND *pWindowHandle)
{
  return GetAnyDeviceInstance()->GetWindowAssociation(pWindowHandle);
}
//---------------------------------------------------------------------
#if 0
template <class T>
HRESULT _DXGIFactory<T>::CreateSwapChain(
            IUnknown *pDevice,
            DXGI_SWAP_CHAIN_DESC *pDesc,
            IDXGISwapChain **ppSwapChain)
{
  HRESULT result;
  DXGISwapChain * newDXGISwapChain = new DXGISwapChain();
  newDXGISwapChain->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateSwapChain(pDevice == NULL ? NULL : (static_cast<Unknown*>(pDevice))->GetInstance(i), pDesc, newDXGISwapChain->GetInstancePtr(i));
    }
  }

  *ppSwapChain = newDXGISwapChain;
  return result;
}
#endif
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIFactory<T>::CreateSoftwareAdapter(
            /* [in] */ HMODULE Module,
            /* [out] */ IDXGIAdapter **ppAdapter)
{
  HRESULT result;
  DXGIAdapter * newDXGIAdapter = new DXGIAdapter();
  newDXGIAdapter->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateSoftwareAdapter(Module, newDXGIAdapter->GetInstancePtr(i));
    }
  }

  *ppAdapter = newDXGIAdapter;
  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIDevice
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIDevice()
{
  DXGIDevice linkFixDXGIDevice;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIDevice<T>::GetAdapter(
            /* [out] */ IDXGIAdapter **pAdapter)
{
  return GetAnyDeviceInstance()->GetAdapter(pAdapter);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIDevice<T>::CreateSurface(
            /* [in] */ const DXGI_SURFACE_DESC *pDesc,
            /* [in] */ UINT NumSurfaces,
            /* [in] */ DXGI_USAGE Usage,
            /* [in] */ const DXGI_SHARED_RESOURCE *pSharedResource,
            /* [out] */ IDXGISurface **ppSurface)
{
  HRESULT result;
  DXGISurface * newDXGISurface = new DXGISurface();
  newDXGISurface->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateSurface(pDesc, NumSurfaces, Usage, pSharedResource, newDXGISurface->GetInstancePtr(i));
    }
  }

  *ppSurface = newDXGISurface;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIDevice<T>::QueryResourceResidency(
            /* [size_is][in] */ IUnknown *const *ppResources,
            /* [size_is][out] */ DXGI_RESIDENCY *pResidencyStatus,
            /* [in] */ UINT NumResources)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->QueryResourceResidency(ppResources == NULL ? NULL : (ConvertArray<Unknown, IUnknown>(ppResources, NumResources, i)), pResidencyStatus, NumResources);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIDevice<T>::SetGPUThreadPriority(
            /* [in] */ INT Priority)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetGPUThreadPriority(Priority);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIDevice<T>::GetGPUThreadPriority(
            /* [retval][out] */ INT *pPriority)
{
  return GetAnyDeviceInstance()->GetGPUThreadPriority(pPriority);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIFactory1
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIFactory1()
{
  DXGIFactory1 linkFixDXGIFactory1;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIFactory1<T>::EnumAdapters1(
            /* [in] */ UINT Adapter,
            /* [out] */ IDXGIAdapter1 **ppAdapter)
{
  HRESULT result;
  DXGIAdapter1 * newDXGIAdapter1 = new DXGIAdapter1();
  newDXGIAdapter1->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->EnumAdapters1(Adapter, newDXGIAdapter1->GetInstancePtr(i));
    }
  }

  *ppAdapter = newDXGIAdapter1;
  return result;
}
//---------------------------------------------------------------------
template <class T>
BOOL _DXGIFactory1<T>::IsCurrent( void )
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->IsCurrent();
    }
  }

  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIAdapter1
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIAdapter1()
{
  DXGIAdapter1 linkFixDXGIAdapter1;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIAdapter1<T>::GetDesc1(
            /* [out] */ DXGI_ADAPTER_DESC1 *pDesc)
{
  return GetAnyDeviceInstance()->GetDesc1(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            DXGIDevice1
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixDXGIDevice1()
{
  DXGIDevice1 linkFixDXGIDevice1;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIDevice1<T>::SetMaximumFrameLatency(
            /* [in] */ UINT MaxLatency)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetMaximumFrameLatency(MaxLatency);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _DXGIDevice1<T>::GetMaximumFrameLatency(
            /* [out] */ UINT *pMaxLatency)
{
  return GetAnyDeviceInstance()->GetMaximumFrameLatency(pMaxLatency);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11DeviceChild
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11DeviceChild()
{
  D3D11DeviceChild linkFixD3D11DeviceChild;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceChild<T>::GetDevice(
            /* [annotation] */ 
            __out  ID3D11Device **ppDevice)
{
  D3D11Device * newD3D11Device = new D3D11Device();
  newD3D11Device->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      (*iter)->GetDevice( newD3D11Device->GetInstancePtr(i));
    }
  }

  *ppDevice = newD3D11Device;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11DeviceChild<T>::GetPrivateData(
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __inout  UINT *pDataSize,
            /* [annotation] */ 
            __out_bcount_opt( *pDataSize )  void *pData)
{
  return GetAnyDeviceInstance()->GetPrivateData(guid, pDataSize, pData);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11DeviceChild<T>::SetPrivateData(
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __in  UINT DataSize,
            /* [annotation] */ 
            __in_bcount_opt( DataSize )  const void *pData)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetPrivateData(guid, DataSize, pData);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11DeviceChild<T>::SetPrivateDataInterface(
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __in_opt  const IUnknown *pData)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetPrivateDataInterface(guid, pData == NULL ? NULL : (static_cast<const Unknown*>(pData))->GetInstance(i));
    }
  }

  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11DepthStencilState
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11DepthStencilState()
{
  D3D11DepthStencilState linkFixD3D11DepthStencilState;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11DepthStencilState<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_DEPTH_STENCIL_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11BlendState
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11BlendState()
{
  D3D11BlendState linkFixD3D11BlendState;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11BlendState<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_BLEND_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11RasterizerState
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11RasterizerState()
{
  D3D11RasterizerState linkFixD3D11RasterizerState;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11RasterizerState<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_RASTERIZER_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Resource
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Resource()
{
  D3D11Resource linkFixD3D11Resource;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11Resource<T>::GetType(
            /* [annotation] */ 
            __out  D3D11_RESOURCE_DIMENSION *pResourceDimension)
{
  GetAnyDeviceInstance()->GetType(pResourceDimension);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11Resource<T>::SetEvictionPriority(
            /* [annotation] */ 
            __in  UINT EvictionPriority)
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->SetEvictionPriority(EvictionPriority);
    }
  }

}
//---------------------------------------------------------------------
template <class T>
UINT _D3D11Resource<T>::GetEvictionPriority( void )
{
  return GetAnyDeviceInstance()->GetEvictionPriority();
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Buffer
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Buffer()
{
  D3D11Buffer linkFixD3D11Buffer;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11Buffer<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_BUFFER_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Texture1D
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Texture1D()
{
  D3D11Texture1D linkFixD3D11Texture1D;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11Texture1D<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_TEXTURE1D_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Texture2D
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Texture2D()
{
  D3D11Texture2D linkFixD3D11Texture2D;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11Texture2D<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_TEXTURE2D_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Texture3D
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Texture3D()
{
  D3D11Texture3D linkFixD3D11Texture3D;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11Texture3D<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_TEXTURE3D_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11View
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11View()
{
  D3D11View linkFixD3D11View;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11View<T>::GetResource(
            /* [annotation] */ 
            __out  ID3D11Resource **ppResource)
{
  D3D11Resource * newD3D11Resource = new D3D11Resource();
  newD3D11Resource->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      (*iter)->GetResource( newD3D11Resource->GetInstancePtr(i));
    }
  }

  *ppResource = newD3D11Resource;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11ShaderResourceView
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11ShaderResourceView()
{
  D3D11ShaderResourceView linkFixD3D11ShaderResourceView;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11ShaderResourceView<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11RenderTargetView
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11RenderTargetView()
{
  D3D11RenderTargetView linkFixD3D11RenderTargetView;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11RenderTargetView<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_RENDER_TARGET_VIEW_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11DepthStencilView
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11DepthStencilView()
{
  D3D11DepthStencilView linkFixD3D11DepthStencilView;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11DepthStencilView<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11UnorderedAccessView
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11UnorderedAccessView()
{
  D3D11UnorderedAccessView linkFixD3D11UnorderedAccessView;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11UnorderedAccessView<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_UNORDERED_ACCESS_VIEW_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11VertexShader
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11VertexShader()
{
  D3D11VertexShader linkFixD3D11VertexShader;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11HullShader
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11HullShader()
{
  D3D11HullShader linkFixD3D11HullShader;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11DomainShader
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11DomainShader()
{
  D3D11DomainShader linkFixD3D11DomainShader;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11GeometryShader
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11GeometryShader()
{
  D3D11GeometryShader linkFixD3D11GeometryShader;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11PixelShader
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11PixelShader()
{
  D3D11PixelShader linkFixD3D11PixelShader;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11ComputeShader
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11ComputeShader()
{
  D3D11ComputeShader linkFixD3D11ComputeShader;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11InputLayout
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11InputLayout()
{
  D3D11InputLayout linkFixD3D11InputLayout;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11SamplerState
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11SamplerState()
{
  D3D11SamplerState linkFixD3D11SamplerState;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11SamplerState<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_SAMPLER_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Asynchronous
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Asynchronous()
{
  D3D11Asynchronous linkFixD3D11Asynchronous;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
UINT _D3D11Asynchronous<T>::GetDataSize( void )
{
  return GetAnyDeviceInstance()->GetDataSize();
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Query
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Query()
{
  D3D11Query linkFixD3D11Query;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11Query<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_QUERY_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Predicate
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Predicate()
{
  D3D11Predicate linkFixD3D11Predicate;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Counter
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Counter()
{
  D3D11Counter linkFixD3D11Counter;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11Counter<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_COUNTER_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11ClassInstance
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11ClassInstance()
{
  D3D11ClassInstance linkFixD3D11ClassInstance;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11ClassInstance<T>::GetClassLinkage(
            /* [annotation] */ 
            __out  ID3D11ClassLinkage **ppLinkage)
{
  D3D11ClassLinkage * newD3D11ClassLinkage = new D3D11ClassLinkage();
  newD3D11ClassLinkage->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      (*iter)->GetClassLinkage( newD3D11ClassLinkage->GetInstancePtr(i));
    }
  }

  *ppLinkage = newD3D11ClassLinkage;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11ClassInstance<T>::GetDesc(
            /* [annotation] */ 
            __out  D3D11_CLASS_INSTANCE_DESC *pDesc)
{
  GetAnyDeviceInstance()->GetDesc(pDesc);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11ClassInstance<T>::GetInstanceName(
            /* [annotation] */ 
            __out_ecount_opt(*pBufferLength)  LPSTR pInstanceName,
            /* [annotation] */ 
            __inout  SIZE_T *pBufferLength)
{
  GetAnyDeviceInstance()->GetInstanceName(pInstanceName, pBufferLength);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11ClassInstance<T>::GetTypeName(
            /* [annotation] */ 
            __out_ecount_opt(*pBufferLength)  LPSTR pTypeName,
            /* [annotation] */ 
            __inout  SIZE_T *pBufferLength)
{
  GetAnyDeviceInstance()->GetTypeName(pTypeName, pBufferLength);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11ClassLinkage
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11ClassLinkage()
{
  D3D11ClassLinkage linkFixD3D11ClassLinkage;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11ClassLinkage<T>::GetClassInstance(
            /* [annotation] */ 
            __in  LPCSTR pClassInstanceName,
            /* [annotation] */ 
            __in  UINT InstanceIndex,
            /* [annotation] */ 
            __out  ID3D11ClassInstance **ppInstance)
{
  HRESULT result;
  D3D11ClassInstance * newD3D11ClassInstance = new D3D11ClassInstance();
  newD3D11ClassInstance->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->GetClassInstance(pClassInstanceName, InstanceIndex, newD3D11ClassInstance->GetInstancePtr(i));
    }
  }

  *ppInstance = newD3D11ClassInstance;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11ClassLinkage<T>::CreateClassInstance(
            /* [annotation] */ 
            __in  LPCSTR pClassTypeName,
            /* [annotation] */ 
            __in  UINT ConstantBufferOffset,
            /* [annotation] */ 
            __in  UINT ConstantVectorOffset,
            /* [annotation] */ 
            __in  UINT TextureOffset,
            /* [annotation] */ 
            __in  UINT SamplerOffset,
            /* [annotation] */ 
            __out  ID3D11ClassInstance **ppInstance)
{
  HRESULT result;
  D3D11ClassInstance * newD3D11ClassInstance = new D3D11ClassInstance();
  newD3D11ClassInstance->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateClassInstance(pClassTypeName, ConstantBufferOffset, ConstantVectorOffset, TextureOffset, SamplerOffset, newD3D11ClassInstance->GetInstancePtr(i));
    }
  }

  *ppInstance = newD3D11ClassInstance;
  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11CommandList
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11CommandList()
{
  D3D11CommandList linkFixD3D11CommandList;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
UINT _D3D11CommandList<T>::GetContextFlags( void )
{
  return GetAnyDeviceInstance()->GetContextFlags();
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11DeviceContext
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11DeviceContext()
{
  D3D11DeviceContext linkFixD3D11DeviceContext;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::VSSetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->VSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->VSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::PSSetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->PSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->PSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::PSSetShader(
            /* [annotation] */ 
            __in_opt  ID3D11PixelShader *pPixelShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->PSSetShader(pPixelShader == NULL ? NULL : (static_cast<D3D11PixelShader*>(pPixelShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->PSSetShader(pPixelShader == NULL ? NULL : (static_cast<D3D11PixelShader*>(pPixelShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::PSSetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->PSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->PSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::VSSetShader(
            /* [annotation] */ 
            __in_opt  ID3D11VertexShader *pVertexShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->VSSetShader(pVertexShader == NULL ? NULL : (static_cast<D3D11VertexShader*>(pVertexShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->VSSetShader(pVertexShader == NULL ? NULL : (static_cast<D3D11VertexShader*>(pVertexShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DrawIndexed(
            /* [annotation] */ 
            __in  UINT IndexCount,
            /* [annotation] */ 
            __in  UINT StartIndexLocation,
            /* [annotation] */ 
            __in  INT BaseVertexLocation)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::Draw(
            /* [annotation] */ 
            __in  UINT VertexCount,
            /* [annotation] */ 
            __in  UINT StartVertexLocation)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->Draw(VertexCount, StartVertexLocation);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->Draw(VertexCount, StartVertexLocation);
      }
    }
 }

}
//---------------------------------------------------------------------
#if 0
template <class T>
HRESULT _D3D11DeviceContext<T>::Map(
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in  UINT Subresource,
            /* [annotation] */ 
            __in  D3D11_MAP MapType,
            /* [annotation] */ 
            __in  UINT MapFlags,
            /* [annotation] */ 
            __out  D3D11_MAPPED_SUBRESOURCE *pMappedResource)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->Map(pResource == NULL ? NULL : (static_cast<D3D11Resource*>(pResource))->GetInstance(i), Subresource, MapType, MapFlags, pMappedResource);
    }
  }

  return result;
}
#endif
//---------------------------------------------------------------------
#if 0
template <class T>
void _D3D11DeviceContext<T>::Unmap(
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in  UINT Subresource)
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->Unmap(pResource == NULL ? NULL : (static_cast<D3D11Resource*>(pResource))->GetInstance(i), Subresource);
    }
  }

}
#endif
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::PSSetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->PSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->PSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::IASetInputLayout(
            /* [annotation] */ 
            __in_opt  ID3D11InputLayout *pInputLayout)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->IASetInputLayout(pInputLayout == NULL ? NULL : (static_cast<D3D11InputLayout*>(pInputLayout))->GetInstance(i));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->IASetInputLayout(pInputLayout == NULL ? NULL : (static_cast<D3D11InputLayout*>(pInputLayout))->GetInstance(i));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::IASetVertexBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppVertexBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  const UINT *pStrides,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  const UINT *pOffsets)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->IASetVertexBuffers(StartSlot, NumBuffers, ppVertexBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppVertexBuffers, NumBuffers, i)), pStrides, pOffsets);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->IASetVertexBuffers(StartSlot, NumBuffers, ppVertexBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppVertexBuffers, NumBuffers, i)), pStrides, pOffsets);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::IASetIndexBuffer(
            /* [annotation] */ 
            __in_opt  ID3D11Buffer *pIndexBuffer,
            /* [annotation] */ 
            __in  DXGI_FORMAT Format,
            /* [annotation] */ 
            __in  UINT Offset)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->IASetIndexBuffer(pIndexBuffer == NULL ? NULL : (static_cast<D3D11Buffer*>(pIndexBuffer))->GetInstance(i), Format, Offset);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->IASetIndexBuffer(pIndexBuffer == NULL ? NULL : (static_cast<D3D11Buffer*>(pIndexBuffer))->GetInstance(i), Format, Offset);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DrawIndexedInstanced(
            /* [annotation] */ 
            __in  UINT IndexCountPerInstance,
            /* [annotation] */ 
            __in  UINT InstanceCount,
            /* [annotation] */ 
            __in  UINT StartIndexLocation,
            /* [annotation] */ 
            __in  INT BaseVertexLocation,
            /* [annotation] */ 
            __in  UINT StartInstanceLocation)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DrawInstanced(
            /* [annotation] */ 
            __in  UINT VertexCountPerInstance,
            /* [annotation] */ 
            __in  UINT InstanceCount,
            /* [annotation] */ 
            __in  UINT StartVertexLocation,
            /* [annotation] */ 
            __in  UINT StartInstanceLocation)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::GSSetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->GSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->GSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::GSSetShader(
            /* [annotation] */ 
            __in_opt  ID3D11GeometryShader *pShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->GSSetShader(pShader == NULL ? NULL : (static_cast<D3D11GeometryShader*>(pShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->GSSetShader(pShader == NULL ? NULL : (static_cast<D3D11GeometryShader*>(pShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::IASetPrimitiveTopology(
            /* [annotation] */ 
            __in  D3D11_PRIMITIVE_TOPOLOGY Topology)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->IASetPrimitiveTopology(Topology);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->IASetPrimitiveTopology(Topology);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::VSSetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->VSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->VSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::VSSetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->VSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->VSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::Begin(
            /* [annotation] */ 
            __in  ID3D11Asynchronous *pAsync)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->Begin(pAsync == NULL ? NULL : (static_cast<D3D11Asynchronous*>(pAsync))->GetInstance(i));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->Begin(pAsync == NULL ? NULL : (static_cast<D3D11Asynchronous*>(pAsync))->GetInstance(i));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::End(
            /* [annotation] */ 
            __in  ID3D11Asynchronous *pAsync)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->End(pAsync == NULL ? NULL : (static_cast<D3D11Asynchronous*>(pAsync))->GetInstance(i));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->End(pAsync == NULL ? NULL : (static_cast<D3D11Asynchronous*>(pAsync))->GetInstance(i));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11DeviceContext<T>::GetData(
            /* [annotation] */ 
            __in  ID3D11Asynchronous *pAsync,
            /* [annotation] */ 
            __out_bcount_opt( DataSize )  void *pData,
            /* [annotation] */ 
            __in  UINT DataSize,
            /* [annotation] */ 
            __in  UINT GetDataFlags)
{
  return GetAnyDeviceInstance()->GetData(pAsync, pData, DataSize, GetDataFlags);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::SetPredication(
            /* [annotation] */ 
            __in_opt  ID3D11Predicate *pPredicate,
            /* [annotation] */ 
            __in  BOOL PredicateValue)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->SetPredication(pPredicate == NULL ? NULL : (static_cast<D3D11Predicate*>(pPredicate))->GetInstance(i), PredicateValue);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->SetPredication(pPredicate == NULL ? NULL : (static_cast<D3D11Predicate*>(pPredicate))->GetInstance(i), PredicateValue);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::GSSetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->GSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->GSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::GSSetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->GSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->GSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::OMSetRenderTargets(
            /* [annotation] */ 
            __in_range( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount_opt(NumViews)  ID3D11RenderTargetView *const *ppRenderTargetViews,
            /* [annotation] */ 
            __in_opt  ID3D11DepthStencilView *pDepthStencilView)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->OMSetRenderTargets(NumViews, ppRenderTargetViews == NULL ? NULL : (ConvertArray<D3D11RenderTargetView, ID3D11RenderTargetView>(ppRenderTargetViews, NumViews, i)), pDepthStencilView == NULL ? NULL : (static_cast<D3D11DepthStencilView*>(pDepthStencilView))->GetInstance(i));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->OMSetRenderTargets(NumViews, ppRenderTargetViews == NULL ? NULL : (ConvertArray<D3D11RenderTargetView, ID3D11RenderTargetView>(ppRenderTargetViews, NumViews, i)), pDepthStencilView == NULL ? NULL : (static_cast<D3D11DepthStencilView*>(pDepthStencilView))->GetInstance(i));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::OMSetRenderTargetsAndUnorderedAccessViews(
            /* [annotation] */ 
            __in  UINT NumRTVs,
            /* [annotation] */ 
            __in_ecount_opt(NumRTVs)  ID3D11RenderTargetView *const *ppRenderTargetViews,
            /* [annotation] */ 
            __in_opt  ID3D11DepthStencilView *pDepthStencilView,
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - 1 )  UINT UAVStartSlot,
            /* [annotation] */ 
            __in  UINT NumUAVs,
            /* [annotation] */ 
            __in_ecount_opt(NumUAVs)  ID3D11UnorderedAccessView *const *ppUnorderedAccessViews,
            /* [annotation] */ 
            __in_ecount_opt(NumUAVs)  const UINT *pUAVInitialCounts)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->OMSetRenderTargetsAndUnorderedAccessViews(NumRTVs, ppRenderTargetViews == NULL ? NULL : (ConvertArray<D3D11RenderTargetView, ID3D11RenderTargetView>(ppRenderTargetViews, NumRTVs, i)), pDepthStencilView == NULL ? NULL : (static_cast<D3D11DepthStencilView*>(pDepthStencilView))->GetInstance(i), UAVStartSlot, NumUAVs, ppUnorderedAccessViews == NULL ? NULL : (ConvertArray<D3D11UnorderedAccessView, ID3D11UnorderedAccessView>(ppUnorderedAccessViews, NumUAVs, i)), pUAVInitialCounts);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->OMSetRenderTargetsAndUnorderedAccessViews(NumRTVs, ppRenderTargetViews == NULL ? NULL : (ConvertArray<D3D11RenderTargetView, ID3D11RenderTargetView>(ppRenderTargetViews, NumRTVs, i)), pDepthStencilView == NULL ? NULL : (static_cast<D3D11DepthStencilView*>(pDepthStencilView))->GetInstance(i), UAVStartSlot, NumUAVs, ppUnorderedAccessViews == NULL ? NULL : (ConvertArray<D3D11UnorderedAccessView, ID3D11UnorderedAccessView>(ppUnorderedAccessViews, NumUAVs, i)), pUAVInitialCounts);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::OMSetBlendState(
            /* [annotation] */ 
            __in_opt  ID3D11BlendState *pBlendState,
            /* [annotation] */ 
            __in_opt  const FLOAT BlendFactor[ 4 ],
            /* [annotation] */ 
            __in  UINT SampleMask)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->OMSetBlendState(pBlendState == NULL ? NULL : (static_cast<D3D11BlendState*>(pBlendState))->GetInstance(i), BlendFactor, SampleMask);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->OMSetBlendState(pBlendState == NULL ? NULL : (static_cast<D3D11BlendState*>(pBlendState))->GetInstance(i), BlendFactor, SampleMask);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::OMSetDepthStencilState(
            /* [annotation] */ 
            __in_opt  ID3D11DepthStencilState *pDepthStencilState,
            /* [annotation] */ 
            __in  UINT StencilRef)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->OMSetDepthStencilState(pDepthStencilState == NULL ? NULL : (static_cast<D3D11DepthStencilState*>(pDepthStencilState))->GetInstance(i), StencilRef);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->OMSetDepthStencilState(pDepthStencilState == NULL ? NULL : (static_cast<D3D11DepthStencilState*>(pDepthStencilState))->GetInstance(i), StencilRef);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::SOSetTargets(
            /* [annotation] */ 
            __in_range( 0, D3D11_SO_BUFFER_SLOT_COUNT)  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount_opt(NumBuffers)  ID3D11Buffer *const *ppSOTargets,
            /* [annotation] */ 
            __in_ecount_opt(NumBuffers)  const UINT *pOffsets)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->SOSetTargets(NumBuffers, ppSOTargets == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppSOTargets, NumBuffers, i)), pOffsets);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->SOSetTargets(NumBuffers, ppSOTargets == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppSOTargets, NumBuffers, i)), pOffsets);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DrawAuto( void )
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DrawAuto();
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DrawAuto();
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DrawIndexedInstancedIndirect(
            /* [annotation] */ 
            __in  ID3D11Buffer *pBufferForArgs,
            /* [annotation] */ 
            __in  UINT AlignedByteOffsetForArgs)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DrawIndexedInstancedIndirect(pBufferForArgs == NULL ? NULL : (static_cast<D3D11Buffer*>(pBufferForArgs))->GetInstance(i), AlignedByteOffsetForArgs);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DrawIndexedInstancedIndirect(pBufferForArgs == NULL ? NULL : (static_cast<D3D11Buffer*>(pBufferForArgs))->GetInstance(i), AlignedByteOffsetForArgs);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DrawInstancedIndirect(
            /* [annotation] */ 
            __in  ID3D11Buffer *pBufferForArgs,
            /* [annotation] */ 
            __in  UINT AlignedByteOffsetForArgs)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DrawInstancedIndirect(pBufferForArgs == NULL ? NULL : (static_cast<D3D11Buffer*>(pBufferForArgs))->GetInstance(i), AlignedByteOffsetForArgs);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DrawInstancedIndirect(pBufferForArgs == NULL ? NULL : (static_cast<D3D11Buffer*>(pBufferForArgs))->GetInstance(i), AlignedByteOffsetForArgs);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::Dispatch(
            /* [annotation] */ 
            __in  UINT ThreadGroupCountX,
            /* [annotation] */ 
            __in  UINT ThreadGroupCountY,
            /* [annotation] */ 
            __in  UINT ThreadGroupCountZ)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DispatchIndirect(
            /* [annotation] */ 
            __in  ID3D11Buffer *pBufferForArgs,
            /* [annotation] */ 
            __in  UINT AlignedByteOffsetForArgs)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DispatchIndirect(pBufferForArgs == NULL ? NULL : (static_cast<D3D11Buffer*>(pBufferForArgs))->GetInstance(i), AlignedByteOffsetForArgs);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DispatchIndirect(pBufferForArgs == NULL ? NULL : (static_cast<D3D11Buffer*>(pBufferForArgs))->GetInstance(i), AlignedByteOffsetForArgs);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::RSSetState(
            /* [annotation] */ 
            __in_opt  ID3D11RasterizerState *pRasterizerState)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->RSSetState(pRasterizerState == NULL ? NULL : (static_cast<D3D11RasterizerState*>(pRasterizerState))->GetInstance(i));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->RSSetState(pRasterizerState == NULL ? NULL : (static_cast<D3D11RasterizerState*>(pRasterizerState))->GetInstance(i));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::RSSetViewports(
            /* [annotation] */ 
            __in_range(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumViewports,
            /* [annotation] */ 
            __in_ecount_opt(NumViewports)  const D3D11_VIEWPORT *pViewports)
{
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->RSSetViewports(NumViewports, pViewports);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::RSSetScissorRects(
            /* [annotation] */ 
            __in_range(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumRects,
            /* [annotation] */ 
            __in_ecount_opt(NumRects)  const D3D11_RECT *pRects)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->RSSetScissorRects(NumRects, pRects);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->RSSetScissorRects(NumRects, pRects);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CopySubresourceRegion(
            /* [annotation] */ 
            __in  ID3D11Resource *pDstResource,
            /* [annotation] */ 
            __in  UINT DstSubresource,
            /* [annotation] */ 
            __in  UINT DstX,
            /* [annotation] */ 
            __in  UINT DstY,
            /* [annotation] */ 
            __in  UINT DstZ,
            /* [annotation] */ 
            __in  ID3D11Resource *pSrcResource,
            /* [annotation] */ 
            __in  UINT SrcSubresource,
            /* [annotation] */ 
            __in_opt  const D3D11_BOX *pSrcBox)
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->CopySubresourceRegion(pDstResource == NULL ? NULL : (static_cast<D3D11Resource*>(pDstResource))->GetInstance(i), DstSubresource, DstX, DstY, DstZ, pSrcResource == NULL ? NULL : (static_cast<D3D11Resource*>(pSrcResource))->GetInstance(i), SrcSubresource, pSrcBox);
    }
  }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CopyResource(
            /* [annotation] */ 
            __in  ID3D11Resource *pDstResource,
            /* [annotation] */ 
            __in  ID3D11Resource *pSrcResource)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->CopyResource(pDstResource == NULL ? NULL : (static_cast<D3D11Resource*>(pDstResource))->GetInstance(i), pSrcResource == NULL ? NULL : (static_cast<D3D11Resource*>(pSrcResource))->GetInstance(i));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->CopyResource(pDstResource == NULL ? NULL : (static_cast<D3D11Resource*>(pDstResource))->GetInstance(i), pSrcResource == NULL ? NULL : (static_cast<D3D11Resource*>(pSrcResource))->GetInstance(i));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::UpdateSubresource(
            /* [annotation] */ 
            __in  ID3D11Resource *pDstResource,
            /* [annotation] */ 
            __in  UINT DstSubresource,
            /* [annotation] */ 
            __in_opt  const D3D11_BOX *pDstBox,
            /* [annotation] */ 
            __in  const void *pSrcData,
            /* [annotation] */ 
            __in  UINT SrcRowPitch,
            /* [annotation] */ 
            __in  UINT SrcDepthPitch)
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->UpdateSubresource(pDstResource == NULL ? NULL : (static_cast<D3D11Resource*>(pDstResource))->GetInstance(i), DstSubresource, pDstBox, pSrcData, SrcRowPitch, SrcDepthPitch);
    }
  }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CopyStructureCount(
            /* [annotation] */ 
            __in  ID3D11Buffer *pDstBuffer,
            /* [annotation] */ 
            __in  UINT DstAlignedByteOffset,
            /* [annotation] */ 
            __in  ID3D11UnorderedAccessView *pSrcView)
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->CopyStructureCount(pDstBuffer == NULL ? NULL : (static_cast<D3D11Buffer*>(pDstBuffer))->GetInstance(i), DstAlignedByteOffset, pSrcView == NULL ? NULL : (static_cast<D3D11UnorderedAccessView*>(pSrcView))->GetInstance(i));
    }
  }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::ClearRenderTargetView(
            /* [annotation] */ 
            __in  ID3D11RenderTargetView *pRenderTargetView,
            /* [annotation] */ 
            __in  const FLOAT ColorRGBA[ 4 ])
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->ClearRenderTargetView(pRenderTargetView == NULL ? NULL : (static_cast<D3D11RenderTargetView*>(pRenderTargetView))->GetInstance(i), ColorRGBA);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->ClearRenderTargetView(pRenderTargetView == NULL ? NULL : (static_cast<D3D11RenderTargetView*>(pRenderTargetView))->GetInstance(i), ColorRGBA);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::ClearUnorderedAccessViewUint(
            /* [annotation] */ 
            __in  ID3D11UnorderedAccessView *pUnorderedAccessView,
            /* [annotation] */ 
            __in  const UINT Values[ 4 ])
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->ClearUnorderedAccessViewUint(pUnorderedAccessView == NULL ? NULL : (static_cast<D3D11UnorderedAccessView*>(pUnorderedAccessView))->GetInstance(i), Values);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->ClearUnorderedAccessViewUint(pUnorderedAccessView == NULL ? NULL : (static_cast<D3D11UnorderedAccessView*>(pUnorderedAccessView))->GetInstance(i), Values);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::ClearUnorderedAccessViewFloat(
            /* [annotation] */ 
            __in  ID3D11UnorderedAccessView *pUnorderedAccessView,
            /* [annotation] */ 
            __in  const FLOAT Values[ 4 ])
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->ClearUnorderedAccessViewFloat(pUnorderedAccessView == NULL ? NULL : (static_cast<D3D11UnorderedAccessView*>(pUnorderedAccessView))->GetInstance(i), Values);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->ClearUnorderedAccessViewFloat(pUnorderedAccessView == NULL ? NULL : (static_cast<D3D11UnorderedAccessView*>(pUnorderedAccessView))->GetInstance(i), Values);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::ClearDepthStencilView(
            /* [annotation] */ 
            __in  ID3D11DepthStencilView *pDepthStencilView,
            /* [annotation] */ 
            __in  UINT ClearFlags,
            /* [annotation] */ 
            __in  FLOAT Depth,
            /* [annotation] */ 
            __in  UINT8 Stencil)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->ClearDepthStencilView(pDepthStencilView == NULL ? NULL : (static_cast<D3D11DepthStencilView*>(pDepthStencilView))->GetInstance(i), ClearFlags, Depth, Stencil);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->ClearDepthStencilView(pDepthStencilView == NULL ? NULL : (static_cast<D3D11DepthStencilView*>(pDepthStencilView))->GetInstance(i), ClearFlags, Depth, Stencil);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::GenerateMips(
            /* [annotation] */ 
            __in  ID3D11ShaderResourceView *pShaderResourceView)
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->GenerateMips(pShaderResourceView == NULL ? NULL : (static_cast<D3D11ShaderResourceView*>(pShaderResourceView))->GetInstance(i));
    }
  }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::SetResourceMinLOD(
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            FLOAT MinLOD)
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->SetResourceMinLOD(pResource == NULL ? NULL : (static_cast<D3D11Resource*>(pResource))->GetInstance(i), MinLOD);
    }
  }

}
//---------------------------------------------------------------------
template <class T>
FLOAT _D3D11DeviceContext<T>::GetResourceMinLOD(
            /* [annotation] */ 
            __in  ID3D11Resource *pResource)
{
  return GetAnyDeviceInstance()->GetResourceMinLOD(pResource);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::ResolveSubresource(
            /* [annotation] */ 
            __in  ID3D11Resource *pDstResource,
            /* [annotation] */ 
            __in  UINT DstSubresource,
            /* [annotation] */ 
            __in  ID3D11Resource *pSrcResource,
            /* [annotation] */ 
            __in  UINT SrcSubresource,
            /* [annotation] */ 
            __in  DXGI_FORMAT Format)
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->ResolveSubresource(pDstResource == NULL ? NULL : (static_cast<D3D11Resource*>(pDstResource))->GetInstance(i), DstSubresource, pSrcResource == NULL ? NULL : (static_cast<D3D11Resource*>(pSrcResource))->GetInstance(i), SrcSubresource, Format);
    }
  }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::ExecuteCommandList(
            /* [annotation] */ 
            __in  ID3D11CommandList *pCommandList,
            BOOL RestoreContextState)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->ExecuteCommandList(pCommandList == NULL ? NULL : (static_cast<D3D11CommandList*>(pCommandList))->GetInstance(i), RestoreContextState);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->ExecuteCommandList(pCommandList == NULL ? NULL : (static_cast<D3D11CommandList*>(pCommandList))->GetInstance(i), RestoreContextState);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::HSSetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->HSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->HSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::HSSetShader(
            /* [annotation] */ 
            __in_opt  ID3D11HullShader *pHullShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->HSSetShader(pHullShader == NULL ? NULL : (static_cast<D3D11HullShader*>(pHullShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->HSSetShader(pHullShader == NULL ? NULL : (static_cast<D3D11HullShader*>(pHullShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::HSSetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->HSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->HSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::HSSetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->HSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->HSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DSSetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DSSetShader(
            /* [annotation] */ 
            __in_opt  ID3D11DomainShader *pDomainShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DSSetShader(pDomainShader == NULL ? NULL : (static_cast<D3D11DomainShader*>(pDomainShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DSSetShader(pDomainShader == NULL ? NULL : (static_cast<D3D11DomainShader*>(pDomainShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DSSetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DSSetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->DSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->DSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CSSetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->CSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->CSSetShaderResources(StartSlot, NumViews, ppShaderResourceViews == NULL ? NULL : (ConvertArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(ppShaderResourceViews, NumViews, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CSSetUnorderedAccessViews(
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - StartSlot )  UINT NumUAVs,
            /* [annotation] */ 
            __in_ecount(NumUAVs)  ID3D11UnorderedAccessView *const *ppUnorderedAccessViews,
            /* [annotation] */ 
            __in_ecount(NumUAVs)  const UINT *pUAVInitialCounts)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->CSSetUnorderedAccessViews(StartSlot, NumUAVs, ppUnorderedAccessViews == NULL ? NULL : (ConvertArray<D3D11UnorderedAccessView, ID3D11UnorderedAccessView>(ppUnorderedAccessViews, NumUAVs, i)), pUAVInitialCounts);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->CSSetUnorderedAccessViews(StartSlot, NumUAVs, ppUnorderedAccessViews == NULL ? NULL : (ConvertArray<D3D11UnorderedAccessView, ID3D11UnorderedAccessView>(ppUnorderedAccessViews, NumUAVs, i)), pUAVInitialCounts);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CSSetShader(
            /* [annotation] */ 
            __in_opt  ID3D11ComputeShader *pComputeShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->CSSetShader(pComputeShader == NULL ? NULL : (static_cast<D3D11ComputeShader*>(pComputeShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->CSSetShader(pComputeShader == NULL ? NULL : (static_cast<D3D11ComputeShader*>(pComputeShader))->GetInstance(i), ppClassInstances == NULL ? NULL : (ConvertArray<D3D11ClassInstance, ID3D11ClassInstance>(ppClassInstances, NumClassInstances, i)), NumClassInstances);
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CSSetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->CSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->CSSetSamplers(StartSlot, NumSamplers, ppSamplers == NULL ? NULL : (ConvertArray<D3D11SamplerState, ID3D11SamplerState>(ppSamplers, NumSamplers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CSSetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers)
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->CSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->CSSetConstantBuffers(StartSlot, NumBuffers, ppConstantBuffers == NULL ? NULL : (ConvertArray<D3D11Buffer, ID3D11Buffer>(ppConstantBuffers, NumBuffers, i)));
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::VSGetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11Buffer * tempConstantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->VSGetConstantBuffers(StartSlot, NumBuffers, tempConstantBuffers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::PSGetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11ShaderResourceView * tempShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->PSGetShaderResources(StartSlot, NumViews, tempShaderResourceViews);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::PSGetShader(
            /* [annotation] */ 
            __out_opt  ID3D11PixelShader **ppPixelShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances)
{
  bool firstLoop = true;                                                                                                                                                          
  D3D11PixelShader * newD3D11PixelShader = new D3D11PixelShader();                                                                                                         
  newD3D11PixelShader->InitInstancesCount(GetInstanceCount());                                                                                                                 
  *ppPixelShader = newD3D11PixelShader;                                                                                                                                      
                                                                                                                                                                                  
  static ID3D11ClassInstance * tempClassInstances[1000];                                                                                                                          
                                                                                                                                                                                  
  ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                            
  ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                             
  for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                                      
  {                                                                                                                                                                               
      if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
      {
        continue;
      }
      ID3D11ClassInstance **ppInstClassInstances = NULL;                                                                                                                             
                                                                                                                                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                     
      {                                                                                                                                                                              
        ppInstClassInstances = tempClassInstances;                                                                                                                                 
      }                                                                                                                                                                              
                                                                                                                                                                                  
    if((*iter) != NULL)                                                                                                                                                           
    {                                                                                                                                                                             
      (*iter)->PSGetShader(newD3D11PixelShader->GetInstancePtr(i), ppInstClassInstances, pNumClassInstances);                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                   
      {                                                                                                                                                                            
          if(firstLoop == true)                                                                                                                                                    
          {                                                                                                                                                                        
              firstLoop = false;                                                                                                                                                  
              CreateMultiDeviceArrayBySingleDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, GetInstanceCount());   
          }                                                                                                                                                                        
          ConvertToMultiDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, i);                                        
       }                                                                                                                                                                            
    }                                                                                                                                                                             
  }                                                                                                                                                                             
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::PSGetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11SamplerState * tempSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->PSGetSamplers(StartSlot, NumSamplers, tempSamplers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::VSGetShader(
            /* [annotation] */ 
            __out_opt  ID3D11VertexShader **ppVertexShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances)
{
  bool firstLoop = true;                                                                                                                                                          
  D3D11VertexShader * newD3D11VertexShader = new D3D11VertexShader();                                                                                                         
  newD3D11VertexShader->InitInstancesCount(GetInstanceCount());                                                                                                                 
  *ppVertexShader = newD3D11VertexShader;                                                                                                                                      
                                                                                                                                                                                  
  static ID3D11ClassInstance * tempClassInstances[1000];                                                                                                                          
                                                                                                                                                                                  
  ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                            
  ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                             
  for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                                      
  {                                                                                                                                                                               
      if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
      {
        continue;
      }
      ID3D11ClassInstance **ppInstClassInstances = NULL;                                                                                                                             
                                                                                                                                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                     
      {                                                                                                                                                                              
        ppInstClassInstances = tempClassInstances;                                                                                                                                 
      }                                                                                                                                                                              
                                                                                                                                                                                  
    if((*iter) != NULL)                                                                                                                                                           
    {                                                                                                                                                                             
      (*iter)->VSGetShader(newD3D11VertexShader->GetInstancePtr(i), ppInstClassInstances, pNumClassInstances);                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                   
      {                                                                                                                                                                            
          if(firstLoop == true)                                                                                                                                                    
          {                                                                                                                                                                        
              firstLoop = false;                                                                                                                                                  
              CreateMultiDeviceArrayBySingleDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, GetInstanceCount());   
          }                                                                                                                                                                        
          ConvertToMultiDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, i);                                        
       }                                                                                                                                                                            
    }                                                                                                                                                                             
  }                                                                                                                                                                             
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::PSGetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11Buffer * tempConstantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->PSGetConstantBuffers(StartSlot, NumBuffers, tempConstantBuffers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::IAGetInputLayout(
            /* [annotation] */ 
            __out  ID3D11InputLayout **ppInputLayout)
{
  D3D11InputLayout * newD3D11InputLayout = new D3D11InputLayout();
  newD3D11InputLayout->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      (*iter)->IAGetInputLayout( newD3D11InputLayout->GetInstancePtr(i));
    }
  }

  *ppInputLayout = newD3D11InputLayout;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::IAGetVertexBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount_opt(NumBuffers)  ID3D11Buffer **ppVertexBuffers,
            /* [annotation] */ 
            __out_ecount_opt(NumBuffers)  UINT *pStrides,
            /* [annotation] */ 
            __out_ecount_opt(NumBuffers)  UINT *pOffsets)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11Buffer * tempVertexBuffers[D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->IAGetVertexBuffers(StartSlot, NumBuffers, tempVertexBuffers, pStrides, pOffsets);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11Buffer, ID3D11Buffer>(tempVertexBuffers, ppVertexBuffers, NumBuffers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11Buffer, ID3D11Buffer>(tempVertexBuffers, ppVertexBuffers, NumBuffers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::IAGetIndexBuffer(
            /* [annotation] */ 
            __out_opt  ID3D11Buffer **pIndexBuffer,
            /* [annotation] */ 
            __out_opt  DXGI_FORMAT *Format,
            /* [annotation] */ 
            __out_opt  UINT *Offset)
{
  D3D11Buffer * newD3D11Buffer = new D3D11Buffer();
  newD3D11Buffer->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      (*iter)->IAGetIndexBuffer(newD3D11Buffer->GetInstancePtr(i), Format, Offset);
    }
  }

  *pIndexBuffer = newD3D11Buffer;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::GSGetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11Buffer * tempConstantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->GSGetConstantBuffers(StartSlot, NumBuffers, tempConstantBuffers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::GSGetShader(
            /* [annotation] */ 
            __out_opt  ID3D11GeometryShader **ppGeometryShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances)
{
  bool firstLoop = true;                                                                                                                                                          
  D3D11GeometryShader * newD3D11GeometryShader = new D3D11GeometryShader();                                                                                                         
  newD3D11GeometryShader->InitInstancesCount(GetInstanceCount());                                                                                                                 
  *ppGeometryShader = newD3D11GeometryShader;                                                                                                                                      
                                                                                                                                                                                  
  static ID3D11ClassInstance * tempClassInstances[1000];                                                                                                                          
                                                                                                                                                                                  
  ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                            
  ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                             
  for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                                      
  {                                                                                                                                                                               
      if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
      {
        continue;
      }
      ID3D11ClassInstance **ppInstClassInstances = NULL;                                                                                                                             
                                                                                                                                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                     
      {                                                                                                                                                                              
        ppInstClassInstances = tempClassInstances;                                                                                                                                 
      }                                                                                                                                                                              
                                                                                                                                                                                  
    if((*iter) != NULL)                                                                                                                                                           
    {                                                                                                                                                                             
      (*iter)->GSGetShader(newD3D11GeometryShader->GetInstancePtr(i), ppInstClassInstances, pNumClassInstances);                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                   
      {                                                                                                                                                                            
          if(firstLoop == true)                                                                                                                                                    
          {                                                                                                                                                                        
              firstLoop = false;                                                                                                                                                  
              CreateMultiDeviceArrayBySingleDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, GetInstanceCount());   
          }                                                                                                                                                                        
          ConvertToMultiDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, i);                                        
       }                                                                                                                                                                            
    }                                                                                                                                                                             
  }                                                                                                                                                                             
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::IAGetPrimitiveTopology(
            /* [annotation] */ 
            __out  D3D11_PRIMITIVE_TOPOLOGY *pTopology)
{
  GetAnyDeviceInstance()->IAGetPrimitiveTopology(pTopology);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::VSGetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11ShaderResourceView * tempShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->VSGetShaderResources(StartSlot, NumViews, tempShaderResourceViews);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::VSGetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11SamplerState * tempSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->VSGetSamplers(StartSlot, NumSamplers, tempSamplers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::GetPredication(
            /* [annotation] */ 
            __out_opt  ID3D11Predicate **ppPredicate,
            /* [annotation] */ 
            __out_opt  BOOL *pPredicateValue)
{
  D3D11Predicate * newD3D11Predicate = new D3D11Predicate();
  newD3D11Predicate->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      (*iter)->GetPredication(newD3D11Predicate->GetInstancePtr(i), pPredicateValue);
    }
  }

  *ppPredicate = newD3D11Predicate;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::GSGetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11ShaderResourceView * tempShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->GSGetShaderResources(StartSlot, NumViews, tempShaderResourceViews);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::GSGetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11SamplerState * tempSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->GSGetSamplers(StartSlot, NumSamplers, tempSamplers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
#if 0
template <class T>
void _D3D11DeviceContext<T>::OMGetRenderTargets(
            /* [annotation] */ 
            __in_range( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount_opt(NumViews)  ID3D11RenderTargetView **ppRenderTargetViews,
            /* [annotation] */ 
            __out_opt  ID3D11DepthStencilView **ppDepthStencilView)
{
  GetAnyDeviceInstance()->OMGetRenderTargets(NumViews, ppRenderTargetViews, ppDepthStencilView);
}
#endif
//---------------------------------------------------------------------
#if 0
template <class T>
void _D3D11DeviceContext<T>::OMGetRenderTargetsAndUnorderedAccessViews(
            /* [annotation] */ 
            __in_range( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumRTVs,
            /* [annotation] */ 
            __out_ecount_opt(NumRTVs)  ID3D11RenderTargetView **ppRenderTargetViews,
            /* [annotation] */ 
            __out_opt  ID3D11DepthStencilView **ppDepthStencilView,
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - 1 )  UINT UAVStartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - UAVStartSlot )  UINT NumUAVs,
            /* [annotation] */ 
            __out_ecount_opt(NumUAVs)  ID3D11UnorderedAccessView **ppUnorderedAccessViews)
{
  GetAnyDeviceInstance()->OMGetRenderTargetsAndUnorderedAccessViews(NumRTVs, ppRenderTargetViews, ppDepthStencilView, UAVStartSlot, NumUAVs, ppUnorderedAccessViews);
}
#endif
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::OMGetBlendState(
            /* [annotation] */ 
            __out_opt  ID3D11BlendState **ppBlendState,
            /* [annotation] */ 
            __out_opt  FLOAT BlendFactor[ 4 ],
            /* [annotation] */ 
            __out_opt  UINT *pSampleMask)
{
  D3D11BlendState * newD3D11BlendState = new D3D11BlendState();
  newD3D11BlendState->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      (*iter)->OMGetBlendState(newD3D11BlendState->GetInstancePtr(i), BlendFactor, pSampleMask);
    }
  }

  *ppBlendState = newD3D11BlendState;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::OMGetDepthStencilState(
            /* [annotation] */ 
            __out_opt  ID3D11DepthStencilState **ppDepthStencilState,
            /* [annotation] */ 
            __out_opt  UINT *pStencilRef)
{
  D3D11DepthStencilState * newD3D11DepthStencilState = new D3D11DepthStencilState();
  newD3D11DepthStencilState->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      (*iter)->OMGetDepthStencilState(newD3D11DepthStencilState->GetInstancePtr(i), pStencilRef);
    }
  }

  *ppDepthStencilState = newD3D11DepthStencilState;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::SOGetTargets(
            /* [annotation] */ 
            __in_range( 0, D3D11_SO_BUFFER_SLOT_COUNT )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppSOTargets)
{
  GetAnyDeviceInstance()->SOGetTargets(NumBuffers, ppSOTargets);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::RSGetState(
            /* [annotation] */ 
            __out  ID3D11RasterizerState **ppRasterizerState)
{
  D3D11RasterizerState * newD3D11RasterizerState = new D3D11RasterizerState();
  newD3D11RasterizerState->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      (*iter)->RSGetState( newD3D11RasterizerState->GetInstancePtr(i));
    }
  }

  *ppRasterizerState = newD3D11RasterizerState;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::RSGetViewports(
            /* [annotation] */ 
            __inout /*_range(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE )*/   UINT *pNumViewports,
            /* [annotation] */ 
            __out_ecount_opt(*pNumViewports)  D3D11_VIEWPORT *pViewports)
{
  GetAnyDeviceInstance()->RSGetViewports(pNumViewports, pViewports);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::RSGetScissorRects(
            /* [annotation] */ 
            __inout /*_range(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE )*/   UINT *pNumRects,
            /* [annotation] */ 
            __out_ecount_opt(*pNumRects)  D3D11_RECT *pRects)
{
  GetAnyDeviceInstance()->RSGetScissorRects(pNumRects, pRects);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::HSGetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11ShaderResourceView * tempShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->HSGetShaderResources(StartSlot, NumViews, tempShaderResourceViews);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::HSGetShader(
            /* [annotation] */ 
            __out_opt  ID3D11HullShader **ppHullShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances)
{
  bool firstLoop = true;                                                                                                                                                          
  D3D11HullShader * newD3D11HullShader = new D3D11HullShader();                                                                                                         
  newD3D11HullShader->InitInstancesCount(GetInstanceCount());                                                                                                                 
  *ppHullShader = newD3D11HullShader;                                                                                                                                      
                                                                                                                                                                                  
  static ID3D11ClassInstance * tempClassInstances[1000];                                                                                                                          
                                                                                                                                                                                  
  ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                            
  ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                             
  for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                                      
  {                                                                                                                                                                               
      if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
      {
        continue;
      }
      ID3D11ClassInstance **ppInstClassInstances = NULL;                                                                                                                             
                                                                                                                                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                     
      {                                                                                                                                                                              
        ppInstClassInstances = tempClassInstances;                                                                                                                                 
      }                                                                                                                                                                              
                                                                                                                                                                                  
    if((*iter) != NULL)                                                                                                                                                           
    {                                                                                                                                                                             
      (*iter)->HSGetShader(newD3D11HullShader->GetInstancePtr(i), ppInstClassInstances, pNumClassInstances);                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                   
      {                                                                                                                                                                            
          if(firstLoop == true)                                                                                                                                                    
          {                                                                                                                                                                        
              firstLoop = false;                                                                                                                                                  
              CreateMultiDeviceArrayBySingleDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, GetInstanceCount());   
          }                                                                                                                                                                        
          ConvertToMultiDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, i);                                        
       }                                                                                                                                                                            
    }                                                                                                                                                                             
  }                                                                                                                                                                             
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::HSGetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11SamplerState * tempSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->HSGetSamplers(StartSlot, NumSamplers, tempSamplers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::HSGetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11Buffer * tempConstantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->HSGetConstantBuffers(StartSlot, NumBuffers, tempConstantBuffers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DSGetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11ShaderResourceView * tempShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->DSGetShaderResources(StartSlot, NumViews, tempShaderResourceViews);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DSGetShader(
            /* [annotation] */ 
            __out_opt  ID3D11DomainShader **ppDomainShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances)
{
  bool firstLoop = true;                                                                                                                                                          
  D3D11DomainShader * newD3D11DomainShader = new D3D11DomainShader();                                                                                                         
  newD3D11DomainShader->InitInstancesCount(GetInstanceCount());                                                                                                                 
  *ppDomainShader = newD3D11DomainShader;                                                                                                                                      
                                                                                                                                                                                  
  static ID3D11ClassInstance * tempClassInstances[1000];                                                                                                                          
                                                                                                                                                                                  
  ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                            
  ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                             
  for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                                      
  {                                                                                                                                                                               
      if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
      {
        continue;
      }
      ID3D11ClassInstance **ppInstClassInstances = NULL;                                                                                                                             
                                                                                                                                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                     
      {                                                                                                                                                                              
        ppInstClassInstances = tempClassInstances;                                                                                                                                 
      }                                                                                                                                                                              
                                                                                                                                                                                  
    if((*iter) != NULL)                                                                                                                                                           
    {                                                                                                                                                                             
      (*iter)->DSGetShader(newD3D11DomainShader->GetInstancePtr(i), ppInstClassInstances, pNumClassInstances);                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                   
      {                                                                                                                                                                            
          if(firstLoop == true)                                                                                                                                                    
          {                                                                                                                                                                        
              firstLoop = false;                                                                                                                                                  
              CreateMultiDeviceArrayBySingleDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, GetInstanceCount());   
          }                                                                                                                                                                        
          ConvertToMultiDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, i);                                        
       }                                                                                                                                                                            
    }                                                                                                                                                                             
  }                                                                                                                                                                             
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DSGetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11SamplerState * tempSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->DSGetSamplers(StartSlot, NumSamplers, tempSamplers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::DSGetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11Buffer * tempConstantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->DSGetConstantBuffers(StartSlot, NumBuffers, tempConstantBuffers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CSGetShaderResources(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11ShaderResourceView * tempShaderResourceViews[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->CSGetShaderResources(StartSlot, NumViews, tempShaderResourceViews);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11ShaderResourceView, ID3D11ShaderResourceView>(tempShaderResourceViews, ppShaderResourceViews, NumViews, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CSGetUnorderedAccessViews(
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - StartSlot )  UINT NumUAVs,
            /* [annotation] */ 
            __out_ecount(NumUAVs)  ID3D11UnorderedAccessView **ppUnorderedAccessViews)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11UnorderedAccessView * tempUnorderedAccessViews[D3D11_PS_CS_UAV_REGISTER_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->CSGetUnorderedAccessViews(StartSlot, NumUAVs, tempUnorderedAccessViews);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11UnorderedAccessView, ID3D11UnorderedAccessView>(tempUnorderedAccessViews, ppUnorderedAccessViews, NumUAVs, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11UnorderedAccessView, ID3D11UnorderedAccessView>(tempUnorderedAccessViews, ppUnorderedAccessViews, NumUAVs, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CSGetShader(
            /* [annotation] */ 
            __out_opt  ID3D11ComputeShader **ppComputeShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances)
{
  bool firstLoop = true;                                                                                                                                                          
  D3D11ComputeShader * newD3D11ComputeShader = new D3D11ComputeShader();                                                                                                         
  newD3D11ComputeShader->InitInstancesCount(GetInstanceCount());                                                                                                                 
  *ppComputeShader = newD3D11ComputeShader;                                                                                                                                      
                                                                                                                                                                                  
  static ID3D11ClassInstance * tempClassInstances[1000];                                                                                                                          
                                                                                                                                                                                  
  ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                            
  ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                             
  for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                                      
  {                                                                                                                                                                               
      if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
      {
        continue;
      }
      ID3D11ClassInstance **ppInstClassInstances = NULL;                                                                                                                             
                                                                                                                                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                     
      {                                                                                                                                                                              
        ppInstClassInstances = tempClassInstances;                                                                                                                                 
      }                                                                                                                                                                              
                                                                                                                                                                                  
    if((*iter) != NULL)                                                                                                                                                           
    {                                                                                                                                                                             
      (*iter)->CSGetShader(newD3D11ComputeShader->GetInstancePtr(i), ppInstClassInstances, pNumClassInstances);                                                                  
      if(ppClassInstances != NULL && pNumClassInstances != NULL)                                                                                                                   
      {                                                                                                                                                                            
          if(firstLoop == true)                                                                                                                                                    
          {                                                                                                                                                                        
              firstLoop = false;                                                                                                                                                  
              CreateMultiDeviceArrayBySingleDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, GetInstanceCount());   
          }                                                                                                                                                                        
          ConvertToMultiDeviceArray<D3D11ClassInstance, ID3D11ClassInstance>(tempClassInstances, ppClassInstances, *pNumClassInstances, i);                                        
       }                                                                                                                                                                            
    }                                                                                                                                                                             
  }                                                                                                                                                                             
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CSGetSamplers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11SamplerState * tempSamplers[D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->CSGetSamplers(StartSlot, NumSamplers, tempSamplers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11SamplerState, ID3D11SamplerState>(tempSamplers, ppSamplers, NumSamplers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::CSGetConstantBuffers(
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers)
{
    bool firstLoop = true;                                                                                                                                                  
    static ID3D11Buffer * tempConstantBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];                                                                           
    ResourcePerDeviceListIter iter = _instances.begin();                                                                                                                    
    ResourcePerDeviceListIter iterE = _instances.end();                                                                                                                     
    for(int i = 0; iter != iterE ;i++, iter++)                                                                                                                              
    {                                                                                                                                                                       
        if(BaseObject::GetActiveDeviceIndex() != -1 && BaseObject::GetActiveDeviceIndex() != i)
        {
            continue;
        }
        if((*iter) != NULL)                                                                                                                                                 
        {                                                                                                                                                                   
            (*iter)->CSGetConstantBuffers(StartSlot, NumBuffers, tempConstantBuffers);                                                                                      
            if(firstLoop == true)                                                                                                                                           
            {                                                                                                                                                               
                firstLoop = false;                                                                                                                                         
                CreateMultiDeviceArrayBySingleDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, GetInstanceCount());               
            }                                                                                                                                                               
            ConvertToMultiDeviceArray<D3D11Buffer, ID3D11Buffer>(tempConstantBuffers, ppConstantBuffers, NumBuffers, i);                                                    
                                                                                                                                                                          
        }                                                                                                                                                                   
    }                                                                                                                                                                       
}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::ClearState( void )
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->ClearState();
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->ClearState();
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
void _D3D11DeviceContext<T>::Flush( void )
{
  int i = BaseObject::GetActiveDeviceIndex();
  if(i != -1)
  {
    ID3D11DeviceContext * activeDeviceInstance = GetActiveDeviceInstance();
    activeDeviceInstance->Flush();
  }
  else
  {
    ResourcePerDeviceListIter iter = _instances.begin();
    ResourcePerDeviceListIter iterE = _instances.end();
    for(int i = 0; iter != iterE ;i++, iter++)
    {
      if((*iter) != NULL)
      {
        (*iter)->Flush();
      }
    }
 }

}
//---------------------------------------------------------------------
template <class T>
D3D11_DEVICE_CONTEXT_TYPE _D3D11DeviceContext<T>::GetType( void )
{
  return GetAnyDeviceInstance()->GetType();
}
//---------------------------------------------------------------------
template <class T>
UINT _D3D11DeviceContext<T>::GetContextFlags( void )
{
  return GetAnyDeviceInstance()->GetContextFlags();
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11DeviceContext<T>::FinishCommandList(
            BOOL RestoreDeferredContextState,
            /* [annotation] */ 
            __out_opt  ID3D11CommandList **ppCommandList)
{
  HRESULT result;
  D3D11CommandList * newD3D11CommandList = new D3D11CommandList();
  newD3D11CommandList->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->FinishCommandList(RestoreDeferredContextState, newD3D11CommandList->GetInstancePtr(i));
    }
  }

  *ppCommandList = newD3D11CommandList;
  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Device
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Device()
{
  D3D11Device linkFixD3D11Device;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateBuffer(
            /* [annotation] */ 
            __in  const D3D11_BUFFER_DESC *pDesc,
            /* [annotation] */ 
            __in_opt  const D3D11_SUBRESOURCE_DATA *pInitialData,
            /* [annotation] */ 
            __out_opt  ID3D11Buffer **ppBuffer)
{
  HRESULT result;
  D3D11Buffer * newD3D11Buffer = new D3D11Buffer();
  newD3D11Buffer->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateBuffer(pDesc, pInitialData, newD3D11Buffer->GetInstancePtr(i));
    }
  }

  *ppBuffer = newD3D11Buffer;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateTexture1D(
            /* [annotation] */ 
            __in  const D3D11_TEXTURE1D_DESC *pDesc,
            /* [annotation] */ 
            __in_xcount_opt(pDesc->MipLevels * pDesc->ArraySize)  const D3D11_SUBRESOURCE_DATA *pInitialData,
            /* [annotation] */ 
            __out_opt  ID3D11Texture1D **ppTexture1D)
{
  HRESULT result;
  D3D11Texture1D * newD3D11Texture1D = new D3D11Texture1D();
  newD3D11Texture1D->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateTexture1D(pDesc, pInitialData, newD3D11Texture1D->GetInstancePtr(i));
    }
  }

  *ppTexture1D = newD3D11Texture1D;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateTexture2D(
            /* [annotation] */ 
            __in  const D3D11_TEXTURE2D_DESC *pDesc,
            /* [annotation] */ 
            __in_xcount_opt(pDesc->MipLevels * pDesc->ArraySize)  const D3D11_SUBRESOURCE_DATA *pInitialData,
            /* [annotation] */ 
            __out_opt  ID3D11Texture2D **ppTexture2D)
{
  HRESULT result;
  D3D11Texture2D * newD3D11Texture2D = new D3D11Texture2D();
  newD3D11Texture2D->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateTexture2D(pDesc, pInitialData, newD3D11Texture2D->GetInstancePtr(i));
    }
  }

  *ppTexture2D = newD3D11Texture2D;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateTexture3D(
            /* [annotation] */ 
            __in  const D3D11_TEXTURE3D_DESC *pDesc,
            /* [annotation] */ 
            __in_xcount_opt(pDesc->MipLevels)  const D3D11_SUBRESOURCE_DATA *pInitialData,
            /* [annotation] */ 
            __out_opt  ID3D11Texture3D **ppTexture3D)
{
  HRESULT result;
  D3D11Texture3D * newD3D11Texture3D = new D3D11Texture3D();
  newD3D11Texture3D->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateTexture3D(pDesc, pInitialData, newD3D11Texture3D->GetInstancePtr(i));
    }
  }

  *ppTexture3D = newD3D11Texture3D;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateShaderResourceView(
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in_opt  const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc,
            /* [annotation] */ 
            __out_opt  ID3D11ShaderResourceView **ppSRView)
{
  HRESULT result;
  D3D11ShaderResourceView * newD3D11ShaderResourceView = new D3D11ShaderResourceView();
  newD3D11ShaderResourceView->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateShaderResourceView(pResource == NULL ? NULL : (static_cast<D3D11Resource*>(pResource))->GetInstance(i), pDesc, newD3D11ShaderResourceView->GetInstancePtr(i));
    }
  }

  *ppSRView = newD3D11ShaderResourceView;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateUnorderedAccessView(
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in_opt  const D3D11_UNORDERED_ACCESS_VIEW_DESC *pDesc,
            /* [annotation] */ 
            __out_opt  ID3D11UnorderedAccessView **ppUAView)
{
  HRESULT result;
  D3D11UnorderedAccessView * newD3D11UnorderedAccessView = new D3D11UnorderedAccessView();
  newD3D11UnorderedAccessView->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateUnorderedAccessView(pResource == NULL ? NULL : (static_cast<D3D11Resource*>(pResource))->GetInstance(i), pDesc, newD3D11UnorderedAccessView->GetInstancePtr(i));
    }
  }

  *ppUAView = newD3D11UnorderedAccessView;
  return result;
}
//---------------------------------------------------------------------
#if 0
template <class T>
HRESULT _D3D11Device<T>::CreateRenderTargetView(
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in_opt  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
            /* [annotation] */ 
            __out_opt  ID3D11RenderTargetView **ppRTView)
{
  HRESULT result;
  D3D11RenderTargetView * newD3D11RenderTargetView = new D3D11RenderTargetView();
  newD3D11RenderTargetView->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateRenderTargetView(pResource == NULL ? NULL : (static_cast<D3D11Resource*>(pResource))->GetInstance(i), pDesc, newD3D11RenderTargetView->GetInstancePtr(i));
    }
  }

  *ppRTView = newD3D11RenderTargetView;
  return result;
}
#endif
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateDepthStencilView(
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in_opt  const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc,
            /* [annotation] */ 
            __out_opt  ID3D11DepthStencilView **ppDepthStencilView)
{
  HRESULT result;
  D3D11DepthStencilView * newD3D11DepthStencilView = new D3D11DepthStencilView();
  newD3D11DepthStencilView->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateDepthStencilView(pResource == NULL ? NULL : (static_cast<D3D11Resource*>(pResource))->GetInstance(i), pDesc, newD3D11DepthStencilView->GetInstancePtr(i));
    }
  }

  *ppDepthStencilView = newD3D11DepthStencilView;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateInputLayout(
            /* [annotation] */ 
            __in_ecount(NumElements)  const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs,
            /* [annotation] */ 
            __in_range( 0, D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT )  UINT NumElements,
            /* [annotation] */ 
            __in  const void *pShaderBytecodeWithInputSignature,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __out_opt  ID3D11InputLayout **ppInputLayout)
{
  HRESULT result;
  D3D11InputLayout * newD3D11InputLayout = new D3D11InputLayout();
  newD3D11InputLayout->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateInputLayout(pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength, newD3D11InputLayout->GetInstancePtr(i));
    }
  }

  *ppInputLayout = newD3D11InputLayout;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateVertexShader(
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11VertexShader **ppVertexShader)
{
  HRESULT result;
  D3D11VertexShader * newD3D11VertexShader = new D3D11VertexShader();
  newD3D11VertexShader->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateVertexShader(pShaderBytecode, BytecodeLength, pClassLinkage == NULL ? NULL : (static_cast<D3D11ClassLinkage*>(pClassLinkage))->GetInstance(i), newD3D11VertexShader->GetInstancePtr(i));
    }
  }

  *ppVertexShader = newD3D11VertexShader;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateGeometryShader(
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11GeometryShader **ppGeometryShader)
{
  HRESULT result;
  D3D11GeometryShader * newD3D11GeometryShader = new D3D11GeometryShader();
  newD3D11GeometryShader->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateGeometryShader(pShaderBytecode, BytecodeLength, pClassLinkage == NULL ? NULL : (static_cast<D3D11ClassLinkage*>(pClassLinkage))->GetInstance(i), newD3D11GeometryShader->GetInstancePtr(i));
    }
  }

  *ppGeometryShader = newD3D11GeometryShader;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateGeometryShaderWithStreamOutput(
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_ecount_opt(NumEntries)  const D3D11_SO_DECLARATION_ENTRY *pSODeclaration,
            /* [annotation] */ 
            __in_range( 0, D3D11_SO_STREAM_COUNT * D3D11_SO_OUTPUT_COMPONENT_COUNT )  UINT NumEntries,
            /* [annotation] */ 
            __in_ecount_opt(NumStrides)  const UINT *pBufferStrides,
            /* [annotation] */ 
            __in_range( 0, D3D11_SO_BUFFER_SLOT_COUNT )  UINT NumStrides,
            /* [annotation] */ 
            __in  UINT RasterizedStream,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11GeometryShader **ppGeometryShader)
{
  HRESULT result;
  D3D11GeometryShader * newD3D11GeometryShader = new D3D11GeometryShader();
  newD3D11GeometryShader->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateGeometryShaderWithStreamOutput(pShaderBytecode, BytecodeLength, pSODeclaration, NumEntries, pBufferStrides, NumStrides, RasterizedStream, pClassLinkage == NULL ? NULL : (static_cast<D3D11ClassLinkage*>(pClassLinkage))->GetInstance(i), newD3D11GeometryShader->GetInstancePtr(i));
    }
  }

  *ppGeometryShader = newD3D11GeometryShader;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreatePixelShader(
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11PixelShader **ppPixelShader)
{
  HRESULT result;
  D3D11PixelShader * newD3D11PixelShader = new D3D11PixelShader();
  newD3D11PixelShader->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreatePixelShader(pShaderBytecode, BytecodeLength, pClassLinkage == NULL ? NULL : (static_cast<D3D11ClassLinkage*>(pClassLinkage))->GetInstance(i), newD3D11PixelShader->GetInstancePtr(i));
    }
  }

  *ppPixelShader = newD3D11PixelShader;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateHullShader(
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11HullShader **ppHullShader)
{
  HRESULT result;
  D3D11HullShader * newD3D11HullShader = new D3D11HullShader();
  newD3D11HullShader->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateHullShader(pShaderBytecode, BytecodeLength, pClassLinkage == NULL ? NULL : (static_cast<D3D11ClassLinkage*>(pClassLinkage))->GetInstance(i), newD3D11HullShader->GetInstancePtr(i));
    }
  }

  *ppHullShader = newD3D11HullShader;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateDomainShader(
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11DomainShader **ppDomainShader)
{
  HRESULT result;
  D3D11DomainShader * newD3D11DomainShader = new D3D11DomainShader();
  newD3D11DomainShader->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateDomainShader(pShaderBytecode, BytecodeLength, pClassLinkage == NULL ? NULL : (static_cast<D3D11ClassLinkage*>(pClassLinkage))->GetInstance(i), newD3D11DomainShader->GetInstancePtr(i));
    }
  }

  *ppDomainShader = newD3D11DomainShader;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateComputeShader(
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11ComputeShader **ppComputeShader)
{
  HRESULT result;
  D3D11ComputeShader * newD3D11ComputeShader = new D3D11ComputeShader();
  newD3D11ComputeShader->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateComputeShader(pShaderBytecode, BytecodeLength, pClassLinkage == NULL ? NULL : (static_cast<D3D11ClassLinkage*>(pClassLinkage))->GetInstance(i), newD3D11ComputeShader->GetInstancePtr(i));
    }
  }

  *ppComputeShader = newD3D11ComputeShader;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateClassLinkage(
            /* [annotation] */ 
            __out  ID3D11ClassLinkage **ppLinkage)
{
  HRESULT result;
  D3D11ClassLinkage * newD3D11ClassLinkage = new D3D11ClassLinkage();
  newD3D11ClassLinkage->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateClassLinkage( newD3D11ClassLinkage->GetInstancePtr(i));
    }
  }

  *ppLinkage = newD3D11ClassLinkage;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateBlendState(
            /* [annotation] */ 
            __in  const D3D11_BLEND_DESC *pBlendStateDesc,
            /* [annotation] */ 
            __out_opt  ID3D11BlendState **ppBlendState)
{
  HRESULT result;
  D3D11BlendState * newD3D11BlendState = new D3D11BlendState();
  newD3D11BlendState->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateBlendState(pBlendStateDesc, newD3D11BlendState->GetInstancePtr(i));
    }
  }

  *ppBlendState = newD3D11BlendState;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateDepthStencilState(
            /* [annotation] */ 
            __in  const D3D11_DEPTH_STENCIL_DESC *pDepthStencilDesc,
            /* [annotation] */ 
            __out_opt  ID3D11DepthStencilState **ppDepthStencilState)
{
  HRESULT result;
  D3D11DepthStencilState * newD3D11DepthStencilState = new D3D11DepthStencilState();
  newD3D11DepthStencilState->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateDepthStencilState(pDepthStencilDesc, newD3D11DepthStencilState->GetInstancePtr(i));
    }
  }

  *ppDepthStencilState = newD3D11DepthStencilState;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateRasterizerState(
            /* [annotation] */ 
            __in  const D3D11_RASTERIZER_DESC *pRasterizerDesc,
            /* [annotation] */ 
            __out_opt  ID3D11RasterizerState **ppRasterizerState)
{
  HRESULT result;
  D3D11RasterizerState * newD3D11RasterizerState = new D3D11RasterizerState();
  newD3D11RasterizerState->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateRasterizerState(pRasterizerDesc, newD3D11RasterizerState->GetInstancePtr(i));
    }
  }

  *ppRasterizerState = newD3D11RasterizerState;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateSamplerState(
            /* [annotation] */ 
            __in  const D3D11_SAMPLER_DESC *pSamplerDesc,
            /* [annotation] */ 
            __out_opt  ID3D11SamplerState **ppSamplerState)
{
  HRESULT result;
  D3D11SamplerState * newD3D11SamplerState = new D3D11SamplerState();
  newD3D11SamplerState->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateSamplerState(pSamplerDesc, newD3D11SamplerState->GetInstancePtr(i));
    }
  }

  *ppSamplerState = newD3D11SamplerState;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateQuery(
            /* [annotation] */ 
            __in  const D3D11_QUERY_DESC *pQueryDesc,
            /* [annotation] */ 
            __out_opt  ID3D11Query **ppQuery)
{
  HRESULT result;
  D3D11Query * newD3D11Query = new D3D11Query();
  newD3D11Query->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateQuery(pQueryDesc, newD3D11Query->GetInstancePtr(i));
    }
  }

  *ppQuery = newD3D11Query;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreatePredicate(
            /* [annotation] */ 
            __in  const D3D11_QUERY_DESC *pPredicateDesc,
            /* [annotation] */ 
            __out_opt  ID3D11Predicate **ppPredicate)
{
  HRESULT result;
  D3D11Predicate * newD3D11Predicate = new D3D11Predicate();
  newD3D11Predicate->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreatePredicate(pPredicateDesc, newD3D11Predicate->GetInstancePtr(i));
    }
  }

  *ppPredicate = newD3D11Predicate;
  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CreateCounter(
            /* [annotation] */ 
            __in  const D3D11_COUNTER_DESC *pCounterDesc,
            /* [annotation] */ 
            __out_opt  ID3D11Counter **ppCounter)
{
  HRESULT result;
  D3D11Counter * newD3D11Counter = new D3D11Counter();
  newD3D11Counter->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateCounter(pCounterDesc, newD3D11Counter->GetInstancePtr(i));
    }
  }

  *ppCounter = newD3D11Counter;
  return result;
}
//---------------------------------------------------------------------
#if 0
template <class T>
HRESULT _D3D11Device<T>::CreateDeferredContext(
            UINT ContextFlags,
            /* [annotation] */ 
            __out_opt  ID3D11DeviceContext **ppDeferredContext)
{
  HRESULT result;
  D3D11DeviceContext * newD3D11DeviceContext = new D3D11DeviceContext();
  newD3D11DeviceContext->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      result = (*iter)->CreateDeferredContext(ContextFlags, newD3D11DeviceContext->GetInstancePtr(i));
    }
  }

  *ppDeferredContext = newD3D11DeviceContext;
  return result;
}
#endif
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::OpenSharedResource(
            /* [annotation] */ 
            __in  HANDLE hResource,
            /* [annotation] */ 
            __in  REFIID ReturnedInterface,
            /* [annotation] */ 
            __out_opt  void **ppResource)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->OpenSharedResource(hResource, ReturnedInterface, ppResource);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CheckFormatSupport(
            /* [annotation] */ 
            __in  DXGI_FORMAT Format,
            /* [annotation] */ 
            __out  UINT *pFormatSupport)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->CheckFormatSupport(Format, pFormatSupport);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CheckMultisampleQualityLevels(
            /* [annotation] */ 
            __in  DXGI_FORMAT Format,
            /* [annotation] */ 
            __in  UINT SampleCount,
            /* [annotation] */ 
            __out  UINT *pNumQualityLevels)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->CheckMultisampleQualityLevels(Format, SampleCount, pNumQualityLevels);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11Device<T>::CheckCounterInfo(
            /* [annotation] */ 
            __out  D3D11_COUNTER_INFO *pCounterInfo)
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->CheckCounterInfo(pCounterInfo);
    }
  }

}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CheckCounter(
            /* [annotation] */ 
            __in  const D3D11_COUNTER_DESC *pDesc,
            /* [annotation] */ 
            __out  D3D11_COUNTER_TYPE *pType,
            /* [annotation] */ 
            __out  UINT *pActiveCounters,
            /* [annotation] */ 
            __out_ecount_opt(*pNameLength)  LPSTR szName,
            /* [annotation] */ 
            __inout_opt  UINT *pNameLength,
            /* [annotation] */ 
            __out_ecount_opt(*pUnitsLength)  LPSTR szUnits,
            /* [annotation] */ 
            __inout_opt  UINT *pUnitsLength,
            /* [annotation] */ 
            __out_ecount_opt(*pDescriptionLength)  LPSTR szDescription,
            /* [annotation] */ 
            __inout_opt  UINT *pDescriptionLength)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->CheckCounter(pDesc, pType, pActiveCounters, szName, pNameLength, szUnits, pUnitsLength, szDescription, pDescriptionLength);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::CheckFeatureSupport(
            D3D11_FEATURE Feature,
            /* [annotation] */ 
            __out_bcount(FeatureSupportDataSize)  void *pFeatureSupportData,
            UINT FeatureSupportDataSize)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::GetPrivateData(
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __inout  UINT *pDataSize,
            /* [annotation] */ 
            __out_bcount_opt(*pDataSize)  void *pData)
{
  return GetAnyDeviceInstance()->GetPrivateData(guid, pDataSize, pData);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::SetPrivateData(
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __in  UINT DataSize,
            /* [annotation] */ 
            __in_bcount_opt(DataSize)  const void *pData)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetPrivateData(guid, DataSize, pData);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::SetPrivateDataInterface(
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __in_opt  const IUnknown *pData)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetPrivateDataInterface(guid, pData == NULL ? NULL : (static_cast<const Unknown*>(pData))->GetInstance(i));
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
D3D_FEATURE_LEVEL _D3D11Device<T>::GetFeatureLevel( void )
{
  return GetAnyDeviceInstance()->GetFeatureLevel();
}
//---------------------------------------------------------------------
template <class T>
UINT _D3D11Device<T>::GetCreationFlags( void )
{
  return GetAnyDeviceInstance()->GetCreationFlags();
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::GetDeviceRemovedReason( void )
{
  return GetAnyDeviceInstance()->GetDeviceRemovedReason();
}
//---------------------------------------------------------------------
#if 0
template <class T>
void _D3D11Device<T>::GetImmediateContext(
            /* [annotation] */ 
            __out  ID3D11DeviceContext **ppImmediateContext)
{
  D3D11DeviceContext * newD3D11DeviceContext = new D3D11DeviceContext();
  newD3D11DeviceContext->InitInstancesCount(GetInstanceCount());

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
      (*iter)->GetImmediateContext( newD3D11DeviceContext->GetInstancePtr(i));
    }
  }

  *ppImmediateContext = newD3D11DeviceContext;
}
#endif
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Device<T>::SetExceptionMode(
            UINT RaiseFlags)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetExceptionMode(RaiseFlags);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
UINT _D3D11Device<T>::GetExceptionMode( void )
{
  return GetAnyDeviceInstance()->GetExceptionMode();
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11Debug
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11Debug()
{
  D3D11Debug linkFixD3D11Debug;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Debug<T>::SetFeatureMask(
            UINT Mask)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetFeatureMask(Mask);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
UINT _D3D11Debug<T>::GetFeatureMask( void )
{
  return GetAnyDeviceInstance()->GetFeatureMask();
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Debug<T>::SetPresentPerRenderOpDelay(
            UINT Milliseconds)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetPresentPerRenderOpDelay(Milliseconds);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
UINT _D3D11Debug<T>::GetPresentPerRenderOpDelay( void )
{
  return GetAnyDeviceInstance()->GetPresentPerRenderOpDelay();
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Debug<T>::SetSwapChain(
            /* [annotation] */ 
            __in_opt  IDXGISwapChain *pSwapChain)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetSwapChain(pSwapChain);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Debug<T>::GetSwapChain(
            /* [annotation] */ 
            __out  IDXGISwapChain **ppSwapChain)
{
  return GetAnyDeviceInstance()->GetSwapChain(ppSwapChain);
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Debug<T>::ValidateContext(
            /* [annotation] */ 
            __in  ID3D11DeviceContext *pContext)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->ValidateContext(pContext == NULL ? NULL : (static_cast<D3D11DeviceContext*>(pContext))->GetInstance(i));
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Debug<T>::ReportLiveDeviceObjects(
            D3D11_RLDO_FLAGS Flags)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->ReportLiveDeviceObjects(Flags);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11Debug<T>::ValidateContextForDispatch(
            /* [annotation] */ 
            __in  ID3D11DeviceContext *pContext)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->ValidateContextForDispatch(pContext == NULL ? NULL : (static_cast<D3D11DeviceContext*>(pContext))->GetInstance(i));
    }
  }

  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11SwitchToRef
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11SwitchToRef()
{
  D3D11SwitchToRef linkFixD3D11SwitchToRef;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
BOOL _D3D11SwitchToRef<T>::SetUseRef(
            BOOL UseRef)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetUseRef(UseRef);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
BOOL _D3D11SwitchToRef<T>::GetUseRef( void )
{
  return GetAnyDeviceInstance()->GetUseRef();
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            D3D11InfoQueue
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LinkFixD3D11InfoQueue()
{
  D3D11InfoQueue linkFixD3D11InfoQueue;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::SetMessageCountLimit(
            /* [annotation] */ 
            __in  UINT64 MessageCountLimit)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetMessageCountLimit(MessageCountLimit);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11InfoQueue<T>::ClearStoredMessages( void )
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->ClearStoredMessages();
    }
  }

}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::GetMessage(
            /* [annotation] */ 
            __in  UINT64 MessageIndex,
            /* [annotation] */ 
            __out_bcount_opt(*pMessageByteLength)  D3D11_MESSAGE *pMessage,
            /* [annotation] */ 
            __inout  SIZE_T *pMessageByteLength)
{
  return GetAnyDeviceInstance()->GetMessage(MessageIndex, pMessage, pMessageByteLength);
}
//---------------------------------------------------------------------
template <class T>
UINT64 _D3D11InfoQueue<T>::GetNumMessagesAllowedByStorageFilter( void )
{
  return GetAnyDeviceInstance()->GetNumMessagesAllowedByStorageFilter();
}
//---------------------------------------------------------------------
template <class T>
UINT64 _D3D11InfoQueue<T>::GetNumMessagesDeniedByStorageFilter( void )
{
  return GetAnyDeviceInstance()->GetNumMessagesDeniedByStorageFilter();
}
//---------------------------------------------------------------------
template <class T>
UINT64 _D3D11InfoQueue<T>::GetNumStoredMessages( void )
{
  return GetAnyDeviceInstance()->GetNumStoredMessages();
}
//---------------------------------------------------------------------
template <class T>
UINT64 _D3D11InfoQueue<T>::GetNumStoredMessagesAllowedByRetrievalFilter( void )
{
  return GetAnyDeviceInstance()->GetNumStoredMessagesAllowedByRetrievalFilter();
}
//---------------------------------------------------------------------
template <class T>
UINT64 _D3D11InfoQueue<T>::GetNumMessagesDiscardedByMessageCountLimit( void )
{
  return GetAnyDeviceInstance()->GetNumMessagesDiscardedByMessageCountLimit();
}
//---------------------------------------------------------------------
template <class T>
UINT64 _D3D11InfoQueue<T>::GetMessageCountLimit( void )
{
  return GetAnyDeviceInstance()->GetMessageCountLimit();
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::AddStorageFilterEntries(
            /* [annotation] */ 
            __in  D3D11_INFO_QUEUE_FILTER *pFilter)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->AddStorageFilterEntries(pFilter);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::GetStorageFilter(
            /* [annotation] */ 
            __out_bcount_opt(*pFilterByteLength)  D3D11_INFO_QUEUE_FILTER *pFilter,
            /* [annotation] */ 
            __inout  SIZE_T *pFilterByteLength)
{
  return GetAnyDeviceInstance()->GetStorageFilter(pFilter, pFilterByteLength);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11InfoQueue<T>::ClearStorageFilter( void )
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->ClearStorageFilter();
    }
  }

}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::PushEmptyStorageFilter( void )
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->PushEmptyStorageFilter();
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::PushCopyOfStorageFilter( void )
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->PushCopyOfStorageFilter();
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::PushStorageFilter(
            /* [annotation] */ 
            __in  D3D11_INFO_QUEUE_FILTER *pFilter)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->PushStorageFilter(pFilter);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11InfoQueue<T>::PopStorageFilter( void )
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->PopStorageFilter();
    }
  }

}
//---------------------------------------------------------------------
template <class T>
UINT _D3D11InfoQueue<T>::GetStorageFilterStackSize( void )
{
  return GetAnyDeviceInstance()->GetStorageFilterStackSize();
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::AddRetrievalFilterEntries(
            /* [annotation] */ 
            __in  D3D11_INFO_QUEUE_FILTER *pFilter)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->AddRetrievalFilterEntries(pFilter);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::GetRetrievalFilter(
            /* [annotation] */ 
            __out_bcount_opt(*pFilterByteLength)  D3D11_INFO_QUEUE_FILTER *pFilter,
            /* [annotation] */ 
            __inout  SIZE_T *pFilterByteLength)
{
  return GetAnyDeviceInstance()->GetRetrievalFilter(pFilter, pFilterByteLength);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11InfoQueue<T>::ClearRetrievalFilter( void )
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->ClearRetrievalFilter();
    }
  }

}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::PushEmptyRetrievalFilter( void )
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->PushEmptyRetrievalFilter();
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::PushCopyOfRetrievalFilter( void )
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->PushCopyOfRetrievalFilter();
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::PushRetrievalFilter(
            /* [annotation] */ 
            __in  D3D11_INFO_QUEUE_FILTER *pFilter)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->PushRetrievalFilter(pFilter);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
void _D3D11InfoQueue<T>::PopRetrievalFilter( void )
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->PopRetrievalFilter();
    }
  }

}
//---------------------------------------------------------------------
template <class T>
UINT _D3D11InfoQueue<T>::GetRetrievalFilterStackSize( void )
{
  return GetAnyDeviceInstance()->GetRetrievalFilterStackSize();
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::AddMessage(
            /* [annotation] */ 
            __in  D3D11_MESSAGE_CATEGORY Category,
            /* [annotation] */ 
            __in  D3D11_MESSAGE_SEVERITY Severity,
            /* [annotation] */ 
            __in  D3D11_MESSAGE_ID ID,
            /* [annotation] */ 
            __in  LPCSTR pDescription)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->AddMessage(Category, Severity, ID, pDescription);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::AddApplicationMessage(
            /* [annotation] */ 
            __in  D3D11_MESSAGE_SEVERITY Severity,
            /* [annotation] */ 
            __in  LPCSTR pDescription)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->AddApplicationMessage(Severity, pDescription);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::SetBreakOnCategory(
            /* [annotation] */ 
            __in  D3D11_MESSAGE_CATEGORY Category,
            /* [annotation] */ 
            __in  BOOL bEnable)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetBreakOnCategory(Category, bEnable);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::SetBreakOnSeverity(
            /* [annotation] */ 
            __in  D3D11_MESSAGE_SEVERITY Severity,
            /* [annotation] */ 
            __in  BOOL bEnable)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetBreakOnSeverity(Severity, bEnable);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
HRESULT _D3D11InfoQueue<T>::SetBreakOnID(
            /* [annotation] */ 
            __in  D3D11_MESSAGE_ID ID,
            /* [annotation] */ 
            __in  BOOL bEnable)
{
  HRESULT result;

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       result =  (*iter)->SetBreakOnID(ID, bEnable);
    }
  }

  return result;
}
//---------------------------------------------------------------------
template <class T>
BOOL _D3D11InfoQueue<T>::GetBreakOnCategory(
            /* [annotation] */ 
            __in  D3D11_MESSAGE_CATEGORY Category)
{
  return GetAnyDeviceInstance()->GetBreakOnCategory(Category);
}
//---------------------------------------------------------------------
template <class T>
BOOL _D3D11InfoQueue<T>::GetBreakOnSeverity(
            /* [annotation] */ 
            __in  D3D11_MESSAGE_SEVERITY Severity)
{
  return GetAnyDeviceInstance()->GetBreakOnSeverity(Severity);
}
//---------------------------------------------------------------------
template <class T>
BOOL _D3D11InfoQueue<T>::GetBreakOnID(
            /* [annotation] */ 
            __in  D3D11_MESSAGE_ID ID)
{
  return GetAnyDeviceInstance()->GetBreakOnID(ID);
}
//---------------------------------------------------------------------
template <class T>
void _D3D11InfoQueue<T>::SetMuteDebugOutput(
            /* [annotation] */ 
            __in  BOOL bMute)
{

  ResourcePerDeviceListIter iter = _instances.begin();
  ResourcePerDeviceListIter iterE = _instances.end();
  for(int i = 0; iter != iterE ;i++, iter++)
  {
    if((*iter) != NULL)
    {
       (*iter)->SetMuteDebugOutput(bMute);
    }
  }

}
//---------------------------------------------------------------------
template <class T>
BOOL _D3D11InfoQueue<T>::GetMuteDebugOutput( void )
{
  return GetAnyDeviceInstance()->GetMuteDebugOutput();
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//            BaseObject
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void * BaseObject::CreateObjectByGuid( 
    /* [in] */ REFIID riid, std::vector<void **>  & instancesPtr, size_t instanceCount)
{
    void * result = NULL; 
  if (IsEqualGUID(riid, IID_IUnknown))
  {
    Unknown * newObj = new Unknown();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIObject))
  {
    DXGIObject * newObj = new DXGIObject();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIDeviceSubObject))
  {
    DXGIDeviceSubObject * newObj = new DXGIDeviceSubObject();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIResource))
  {
    DXGIResource * newObj = new DXGIResource();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIKeyedMutex))
  {
    DXGIKeyedMutex * newObj = new DXGIKeyedMutex();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGISurface))
  {
    DXGISurface * newObj = new DXGISurface();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGISurface1))
  {
    DXGISurface1 * newObj = new DXGISurface1();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIAdapter))
  {
    DXGIAdapter * newObj = new DXGIAdapter();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIOutput))
  {
    DXGIOutput * newObj = new DXGIOutput();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGISwapChain))
  {
    DXGISwapChain * newObj = new DXGISwapChain();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIFactory))
  {
    DXGIFactory * newObj = new DXGIFactory();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIDevice))
  {
    DXGIDevice * newObj = new DXGIDevice();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIFactory1))
  {
    DXGIFactory1 * newObj = new DXGIFactory1();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIAdapter1))
  {
    DXGIAdapter1 * newObj = new DXGIAdapter1();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_IDXGIDevice1))
  {
    DXGIDevice1 * newObj = new DXGIDevice1();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11DeviceChild))
  {
    D3D11DeviceChild * newObj = new D3D11DeviceChild();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11DepthStencilState))
  {
    D3D11DepthStencilState * newObj = new D3D11DepthStencilState();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11BlendState))
  {
    D3D11BlendState * newObj = new D3D11BlendState();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11RasterizerState))
  {
    D3D11RasterizerState * newObj = new D3D11RasterizerState();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Resource))
  {
    D3D11Resource * newObj = new D3D11Resource();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Buffer))
  {
    D3D11Buffer * newObj = new D3D11Buffer();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Texture1D))
  {
    D3D11Texture1D * newObj = new D3D11Texture1D();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Texture2D))
  {
    D3D11Texture2D * newObj = new D3D11Texture2D();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Texture3D))
  {
    D3D11Texture3D * newObj = new D3D11Texture3D();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11View))
  {
    D3D11View * newObj = new D3D11View();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11ShaderResourceView))
  {
    D3D11ShaderResourceView * newObj = new D3D11ShaderResourceView();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11RenderTargetView))
  {
    D3D11RenderTargetView * newObj = new D3D11RenderTargetView();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11DepthStencilView))
  {
    D3D11DepthStencilView * newObj = new D3D11DepthStencilView();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11UnorderedAccessView))
  {
    D3D11UnorderedAccessView * newObj = new D3D11UnorderedAccessView();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11VertexShader))
  {
    D3D11VertexShader * newObj = new D3D11VertexShader();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11HullShader))
  {
    D3D11HullShader * newObj = new D3D11HullShader();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11DomainShader))
  {
    D3D11DomainShader * newObj = new D3D11DomainShader();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11GeometryShader))
  {
    D3D11GeometryShader * newObj = new D3D11GeometryShader();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11PixelShader))
  {
    D3D11PixelShader * newObj = new D3D11PixelShader();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11ComputeShader))
  {
    D3D11ComputeShader * newObj = new D3D11ComputeShader();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11InputLayout))
  {
    D3D11InputLayout * newObj = new D3D11InputLayout();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11SamplerState))
  {
    D3D11SamplerState * newObj = new D3D11SamplerState();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Asynchronous))
  {
    D3D11Asynchronous * newObj = new D3D11Asynchronous();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Query))
  {
    D3D11Query * newObj = new D3D11Query();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Predicate))
  {
    D3D11Predicate * newObj = new D3D11Predicate();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Counter))
  {
    D3D11Counter * newObj = new D3D11Counter();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11ClassInstance))
  {
    D3D11ClassInstance * newObj = new D3D11ClassInstance();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11ClassLinkage))
  {
    D3D11ClassLinkage * newObj = new D3D11ClassLinkage();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11CommandList))
  {
    D3D11CommandList * newObj = new D3D11CommandList();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11DeviceContext))
  {
    D3D11DeviceContext * newObj = new D3D11DeviceContext();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Device))
  {
    D3D11Device * newObj = new D3D11Device();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11Debug))
  {
    D3D11Debug * newObj = new D3D11Debug();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11SwitchToRef))
  {
    D3D11SwitchToRef * newObj = new D3D11SwitchToRef();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
  if (IsEqualGUID(riid, IID_ID3D11InfoQueue))
  {
    D3D11InfoQueue * newObj = new D3D11InfoQueue();
    newObj->InitInstancesCount(instanceCount);
    result = (void *)newObj;
    for(size_t i = 0 ; i < instancesPtr.size() ; i++)
    {
      instancesPtr[i] = newObj->GetInstancePtrAsVoid(i);
    }
  }
    return result;
}
//---------------------------------------------------------------------
} // namespace D3D11MultiDevice
