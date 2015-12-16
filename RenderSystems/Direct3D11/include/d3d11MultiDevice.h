#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <d3d11sdklayers.h>

#include "OgreD3D11Prerequisites.h"
#if MULTI_DEVICE_WRAP_OPENGL_INTEROP == 1
#include <OgreGLPrerequisites.h>
#endif


namespace D3D11MultiDevice
{
//---------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////
// D3D11CreateMultiDevice
// ----------------------
//
// Flags
//      Any of those documented for D3D11CreateDeviceAndSwapChain.
// pFeatureLevels
//      Any of those documented for D3D11CreateDeviceAndSwapChain.
// FeatureLevels
//      Size of feature levels array.
// SDKVersion
//      SDK version. Use the D3D11_SDK_VERSION macro.
// ppDevice
//      Pointer to returned interface. May be NULL.
// pFeatureLevel
//      Pointer to returned feature level. May be NULL.
// ppImmediateContext
//      Pointer to returned interface. May be NULL.
//
// Return Values
//  Any of those documented for 
//          CreateDXGIFactory1
//          IDXGIFactory::EnumAdapters
//          IDXGIAdapter::RegisterDriver
//          D3D11CreateDevice
//
///////////////////////////////////////////////////////////////////////////

HRESULT WINAPI D3D11CreateMultiDevice(
    D3D_DRIVER_TYPE DriverType,
    UINT Flags,
    __in_ecount_opt( FeatureLevels ) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
    UINT FeatureLevels,
    UINT SDKVersion,
    __out_opt ID3D11Device** ppDevice,
    __out_opt D3D_FEATURE_LEVEL* pFeatureLevel,
    __out_opt ID3D11DeviceContext** ppImmediateContext
    );

//---------------------------------------------------------------------


// same as CreateDXGIFactory1 but for all the GPUs - for multi device support.
// result will be saved to D3D11MultiDevice::sDxgiFactory and will be used upon calling D3D11CreateMultiDevice

HRESULT WINAPI CreateMultiDeviceDXGIFactory1(IDXGIFactory1 ** ppFactory);

//---------------------------------------------------------------------

// used to copy a buffer to all none active devices so the buffer will be updated on all devices,
// call this function just before "unmap"
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
    );


//---------------------------------------------------------------------

// set the active device by a window handle
void RenderToDeviceByHwnd(HWND val);

//---------------------------------------------------------------------

// set the active device to be all devices - the rendering commands will be
// sent to all GPUs
void RenderToAllDevices();


//---------------------------------------------------------------------

// get internal device (GPU) index by window handle
int GetWindowDeviceIndex(HWND val);


//---------------------------------------------------------------------

class SingleDeviceItem
{
public:
    virtual void AddChild(SingleDeviceItem * child) = 0;
    virtual void RemoveChild(SingleDeviceItem * child) = 0;
    virtual void SetDeviceIdx(int newIdx) = 0;
    virtual void SetParent(SingleDeviceItem * i_parent) = 0;
};

//---------------------------------------------------------------------

class D3D11Device;
class DXGIDevice;
class DXGIFactory;
class D3D11Texture2D;
class DXGIFactory1;
class DXGIAdapter1;
class DXGISwapChain;

//---------------------------------------------------------------------

template <class T>
class SingleDeviceResource : public T, public SingleDeviceItem
{
protected:
    std::set<SingleDeviceItem *> _children;
    SingleDeviceItem * _parent;
    D3D11Device * _d3d11Device;
    DXGIDevice * _dxgiDevice;
    int _deviceIdx;
public:
    SingleDeviceResource();
    virtual ~SingleDeviceResource();
    void SetParent(SingleDeviceItem * i_parent);
    SingleDeviceItem * GetParent();
    void SetD3d11Device(D3D11Device * i_device);
    D3D11Device * GetD3d11Device();
    void SetDxgiDevice(DXGIDevice * i_device);
    DXGIDevice * GetDxgiDevice();
    virtual ULONG STDMETHODCALLTYPE Release( void);
    void RemoveChild(SingleDeviceItem * child);
    void AddChild(SingleDeviceItem * child);

    void LoopChildrenAndSetDeviceIdx(int newIdx);

    int GetDeviceIdx();;

    virtual void SetDeviceIdx(int newIdx);
    virtual void InitDeviceIdx(int newIdx);
};
    
//---------------------------------------------------------------------

  class BaseObject
  {
  private:
      static int s_numberOfDevices;
      static int s_activeDeviceIndex;
      static DXGIFactory1 * s_dxgiFactory;
      static std::vector<DXGIAdapter1 *> s_adapters;
      static std::map<HMONITOR, int> s_monitorToDeviceIndex;
      static std::map<HWND, DXGISwapChain *> s_hwndToSwapChain;
  public:
      static void * CreateObjectByGuid( 
          /* [in] */ REFIID riid, 
          std::vector<void **>  & instancesPtr, 
          size_t instanceCount);

      static int GetNumberOfDevices();
      static void SetNumberOfDevices(int val);
      static int GetActiveDeviceIndex();
      static void SetActiveDeviceIndex(int val);
      static DXGIFactory1 * GetFactory();
      static void SetFactory(DXGIFactory1 * val);
      static std::vector<DXGIAdapter1 *> & GetAdaptersList();
      static int GetMonitorDeviceIndex(HMONITOR val);     
      static void SetMonitorDeviceIndex(HMONITOR mon, int idx);
      static DXGISwapChain * GetSwapChainFromHwnd(HWND val);
      static void SetSwapChainFromHwnd(HWND hwnd, DXGISwapChain * swapChain);
  };

  //---------------------------------------------------------------------

  template <class T>
    class _Unknown : public T
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);

        virtual ULONG STDMETHODCALLTYPE AddRef( void);

        virtual ULONG STDMETHODCALLTYPE Release( void);

        typedef typename std::vector<T *> ResourcePerDeviceList;
        typedef typename std::vector<T *>::iterator ResourcePerDeviceListIter;
        typedef typename std::vector<T *>::const_iterator ResourcePerDeviceListIterConst;

    protected:
        int _baseRef;
        ResourcePerDeviceList  _instances;
    public:        
        _Unknown();
        virtual ~_Unknown();
        T * GetActiveDeviceInstance();
        T * GetAnyDeviceInstance();
        T * GetInstance(int index) const;
        T ** GetInstancePtr(int index);
        void ** GetInstancePtrAsVoid(int index);
        void InitInstancesCount(const size_t count);
        size_t GetInstanceCount();
    };


    class Unknown : public _Unknown<IUnknown> {};


#if MULTI_DEVICE_WRAP_OPENGL_INTEROP == 1
HANDLE WINAPI D3D11MultiDevice_wglDXOpenDeviceNV(void* dxDevice);
BOOL WINAPI D3D11MultiDevice_wglDXCloseDeviceNV(HANDLE hDevice);
BOOL WINAPI  D3D11MultiDevice_wglDXLockObjectsNV(HANDLE hDevice, GLint count, HANDLE* hObjects);
BOOL WINAPI D3D11MultiDevice_wglDXUnlockObjectsNV(HANDLE hDevice, GLint count, HANDLE* hObjects);
BOOL WINAPI D3D11MultiDevice_wglDXObjectAccessNV(HANDLE hObject, GLenum access);
HANDLE WINAPI D3D11MultiDevice_wglDXRegisterObjectNV(HANDLE hDevice, void* dxObject, GLuint name, GLenum type, GLenum access);
BOOL WINAPI D3D11MultiDevice_wglDXSetResourceShareHandleNV(void* dxObject, HANDLE shareHandle);
BOOL WINAPI D3D11MultiDevice_wglDXUnregisterObjectNV(HANDLE hDevice, HANDLE hObject);
#endif


//---------------------------------------------------------------------

    template <class T>
    class _DXGIObject : public _Unknown<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetPrivateData( 
            /* [in] */ REFGUID Name,
            /* [in] */ UINT DataSize,
            /* [in] */ const void *pData);
        virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface( 
            /* [in] */ REFGUID Name,
            /* [in] */ const IUnknown *pUnknown);
        virtual HRESULT STDMETHODCALLTYPE GetPrivateData( 
            /* [in] */ REFGUID Name,
            /* [out][in] */ UINT *pDataSize,
            /* [out] */ void *pData);
        virtual HRESULT STDMETHODCALLTYPE GetParent( 
            /* [in] */ REFIID riid,
            /* [retval][out] */ void **ppParent);
    };

    class DXGIObject : public _DXGIObject<IDXGIObject>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGIDeviceSubObject : public _DXGIObject<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDevice( 
            /* [in] */ REFIID riid,
            /* [retval][out] */ void **ppDevice);
    };

    class DXGIDeviceSubObject : public _DXGIDeviceSubObject<IDXGIDeviceSubObject>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGIResource : public _DXGIDeviceSubObject<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetSharedHandle( 
            /* [out] */ HANDLE *pSharedHandle);
        virtual HRESULT STDMETHODCALLTYPE GetUsage( 
            /* [out] */ DXGI_USAGE *pUsage);
        virtual HRESULT STDMETHODCALLTYPE SetEvictionPriority( 
            /* [in] */ UINT EvictionPriority);
        virtual HRESULT STDMETHODCALLTYPE GetEvictionPriority( 
            /* [retval][out] */ UINT *pEvictionPriority);
    };

    class DXGIResource : public _DXGIResource<IDXGIResource>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGIKeyedMutex : public _DXGIDeviceSubObject<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AcquireSync( 
            /* [in] */ UINT64 Key,
            /* [in] */ DWORD dwMilliseconds);
        virtual HRESULT STDMETHODCALLTYPE ReleaseSync( 
            /* [in] */ UINT64 Key);
    };

    class DXGIKeyedMutex : public _DXGIKeyedMutex<IDXGIKeyedMutex>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGISurface : public _DXGIDeviceSubObject<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDesc( 
            /* [out] */ DXGI_SURFACE_DESC *pDesc);
        virtual HRESULT STDMETHODCALLTYPE Map( 
            /* [out] */ DXGI_MAPPED_RECT *pLockedRect,
            /* [in] */ UINT MapFlags);
        virtual HRESULT STDMETHODCALLTYPE Unmap( void);
    };

    class DXGISurface : public _DXGISurface<IDXGISurface>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGISurface1 : public _DXGISurface<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDC( 
            /* [in] */ BOOL Discard,
            /* [out] */ HDC *phdc);
        virtual HRESULT STDMETHODCALLTYPE ReleaseDC( 
            /* [in] */ RECT *pDirtyRect);
    };

    class DXGISurface1 : public _DXGISurface1<IDXGISurface1>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGIAdapter : public _DXGIObject<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumOutputs( 
            /* [in] */ UINT Output,
            /* [out][in] */ IDXGIOutput **ppOutput);
        virtual HRESULT STDMETHODCALLTYPE GetDesc( 
            /* [out] */ DXGI_ADAPTER_DESC *pDesc);
        virtual HRESULT STDMETHODCALLTYPE CheckInterfaceSupport( 
            /* [in] */ REFGUID InterfaceName,
            /* [out] */ LARGE_INTEGER *pUMDVersion);
    };

    class DXGIAdapter : public _DXGIAdapter<IDXGIAdapter>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGIOutput : public _DXGIObject<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDesc( 
            /* [out] */ DXGI_OUTPUT_DESC *pDesc);
        virtual HRESULT STDMETHODCALLTYPE GetDisplayModeList( 
            /* [in] */ DXGI_FORMAT EnumFormat,
            /* [in] */ UINT Flags,
            /* [out][in] */ UINT *pNumModes,
            /* [out] */ DXGI_MODE_DESC *pDesc);
        virtual HRESULT STDMETHODCALLTYPE FindClosestMatchingMode( 
            /* [in] */ const DXGI_MODE_DESC *pModeToMatch,
            /* [out] */ DXGI_MODE_DESC *pClosestMatch,
            /* [in] */ IUnknown *pConcernedDevice);
        virtual HRESULT STDMETHODCALLTYPE WaitForVBlank( void);
        virtual HRESULT STDMETHODCALLTYPE TakeOwnership( 
            /* [in] */ IUnknown *pDevice,
            BOOL Exclusive);
        virtual void STDMETHODCALLTYPE ReleaseOwnership( void);
        virtual HRESULT STDMETHODCALLTYPE GetGammaControlCapabilities( 
            /* [out] */ DXGI_GAMMA_CONTROL_CAPABILITIES *pGammaCaps);
        virtual HRESULT STDMETHODCALLTYPE SetGammaControl( 
            /* [in] */ const DXGI_GAMMA_CONTROL *pArray);
        virtual HRESULT STDMETHODCALLTYPE GetGammaControl( 
            /* [out] */ DXGI_GAMMA_CONTROL *pArray);
        virtual HRESULT STDMETHODCALLTYPE SetDisplaySurface( 
            /* [in] */ IDXGISurface *pScanoutSurface);
        virtual HRESULT STDMETHODCALLTYPE GetDisplaySurfaceData( 
            /* [in] */ IDXGISurface *pDestination);
        virtual HRESULT STDMETHODCALLTYPE GetFrameStatistics( 
            /* [out] */ DXGI_FRAME_STATISTICS *pStats);
    };

    class DXGIOutput : public _DXGIOutput<IDXGIOutput>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGISwapChain : public SingleDeviceResource<_DXGIDeviceSubObject<T>>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Present( 
            /* [in] */ UINT SyncInterval,
            /* [in] */ UINT Flags);
        virtual HRESULT STDMETHODCALLTYPE GetBuffer( 
            /* [in] */ UINT Buffer,
            /* [in] */ REFIID riid,
            /* [out][in] */ void **ppSurface);
        virtual HRESULT STDMETHODCALLTYPE SetFullscreenState( 
            /* [in] */ BOOL Fullscreen,
            /* [in] */ IDXGIOutput *pTarget);
        virtual HRESULT STDMETHODCALLTYPE GetFullscreenState( 
            /* [out] */ BOOL *pFullscreen,
            /* [out] */ IDXGIOutput **ppTarget);
        virtual HRESULT STDMETHODCALLTYPE GetDesc( 
            /* [out] */ DXGI_SWAP_CHAIN_DESC *pDesc);
        virtual HRESULT STDMETHODCALLTYPE ResizeBuffers( 
            /* [in] */ UINT BufferCount,
            /* [in] */ UINT Width,
            /* [in] */ UINT Height,
            /* [in] */ DXGI_FORMAT NewFormat,
            /* [in] */ UINT SwapChainFlags);
        virtual HRESULT STDMETHODCALLTYPE ResizeTarget( 
            /* [in] */ const DXGI_MODE_DESC *pNewTargetParameters);
        virtual HRESULT STDMETHODCALLTYPE GetContainingOutput( 
            IDXGIOutput **ppOutput);
        virtual HRESULT STDMETHODCALLTYPE GetFrameStatistics( 
            /* [out] */ DXGI_FRAME_STATISTICS *pStats);
        virtual HRESULT STDMETHODCALLTYPE GetLastPresentCount( 
            /* [out] */ UINT *pLastPresentCount);
      virtual void SetDeviceIdx(int newIdx);
    };

    class DXGISwapChain : public _DXGISwapChain<IDXGISwapChain>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGIFactory : public SingleDeviceResource<_DXGIObject<T>>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumAdapters( 
            /* [in] */ UINT Adapter,
            /* [out] */ IDXGIAdapter **ppAdapter);
        virtual HRESULT STDMETHODCALLTYPE MakeWindowAssociation( 
            HWND WindowHandle,
            UINT Flags);
        virtual HRESULT STDMETHODCALLTYPE GetWindowAssociation( 
            HWND *pWindowHandle);
        virtual HRESULT STDMETHODCALLTYPE CreateSwapChain( 
            IUnknown *pDevice,
            DXGI_SWAP_CHAIN_DESC *pDesc,
            IDXGISwapChain **ppSwapChain);
        virtual HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter( 
            /* [in] */ HMODULE Module,
            /* [out] */ IDXGIAdapter **ppAdapter);
      virtual void SetDeviceIdx(int newIdx);
    };

    class DXGIFactory : public _DXGIFactory<IDXGIFactory>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGIDevice : public _DXGIObject<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetAdapter( 
            /* [out] */ IDXGIAdapter **pAdapter);
        virtual HRESULT STDMETHODCALLTYPE CreateSurface( 
            /* [in] */ const DXGI_SURFACE_DESC *pDesc,
            /* [in] */ UINT NumSurfaces,
            /* [in] */ DXGI_USAGE Usage,
            /* [in] */ const DXGI_SHARED_RESOURCE *pSharedResource,
            /* [out] */ IDXGISurface **ppSurface);
        virtual HRESULT STDMETHODCALLTYPE QueryResourceResidency( 
            /* [size_is][in] */ IUnknown *const *ppResources,
            /* [size_is][out] */ DXGI_RESIDENCY *pResidencyStatus,
            /* [in] */ UINT NumResources);
        virtual HRESULT STDMETHODCALLTYPE SetGPUThreadPriority( 
            /* [in] */ INT Priority);
        virtual HRESULT STDMETHODCALLTYPE GetGPUThreadPriority( 
            /* [retval][out] */ INT *pPriority);
    };

    class DXGIDevice : public _DXGIDevice<IDXGIDevice>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGIFactory1 : public _DXGIFactory<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnumAdapters1( 
            /* [in] */ UINT Adapter,
            /* [out] */ IDXGIAdapter1 **ppAdapter);
        virtual BOOL STDMETHODCALLTYPE IsCurrent( void);
    };

    class DXGIFactory1 : public _DXGIFactory1<IDXGIFactory1>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGIAdapter1 : public _DXGIAdapter<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDesc1( 
            /* [out] */ DXGI_ADAPTER_DESC1 *pDesc);
    };

    class DXGIAdapter1 : public _DXGIAdapter1<IDXGIAdapter1>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _DXGIDevice1 : public _DXGIDevice<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetMaximumFrameLatency( 
            /* [in] */ UINT MaxLatency);
        virtual HRESULT STDMETHODCALLTYPE GetMaximumFrameLatency( 
            /* [out] */ UINT *pMaxLatency);
    };

    class DXGIDevice1 : public _DXGIDevice1<IDXGIDevice1>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11DeviceChild : public _Unknown<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDevice( 
            /* [annotation] */ 
            __out  ID3D11Device **ppDevice);
        virtual HRESULT STDMETHODCALLTYPE GetPrivateData( 
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __inout  UINT *pDataSize,
            /* [annotation] */ 
            __out_bcount_opt( *pDataSize )  void *pData);
        virtual HRESULT STDMETHODCALLTYPE SetPrivateData( 
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __in  UINT DataSize,
            /* [annotation] */ 
            __in_bcount_opt( DataSize )  const void *pData);
        virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface( 
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __in_opt  const IUnknown *pData);
    };

    class D3D11DeviceChild : public _D3D11DeviceChild<ID3D11DeviceChild>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11DepthStencilState : public _D3D11DeviceChild<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_DEPTH_STENCIL_DESC *pDesc);
    };

    class D3D11DepthStencilState : public _D3D11DepthStencilState<ID3D11DepthStencilState>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11BlendState : public _D3D11DeviceChild<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_BLEND_DESC *pDesc);
    };

    class D3D11BlendState : public _D3D11BlendState<ID3D11BlendState>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11RasterizerState : public _D3D11DeviceChild<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_RASTERIZER_DESC *pDesc);
    };

    class D3D11RasterizerState : public _D3D11RasterizerState<ID3D11RasterizerState>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Resource : public _D3D11DeviceChild<T>
    {
    protected:
        D3D11_MAP _mapType;
        std::map<UINT, size_t> _SizeOfSubResource;
    public:
        std::map<UINT,std::vector<void *>> & GetMappedSubResources();
        size_t GetSizeOfMappedMemory( __in  UINT Subresource);
        D3D11_MAP GetMapType() const {return _mapType;};
        void SetMapType(const D3D11_MAP val) {_mapType = val;};
        void CopyBufferToSpecificDevice(                    
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
        );
    public:
        virtual void STDMETHODCALLTYPE GetType( 
            /* [annotation] */ 
            __out  D3D11_RESOURCE_DIMENSION *pResourceDimension);
        virtual void STDMETHODCALLTYPE SetEvictionPriority( 
            /* [annotation] */ 
            __in  UINT EvictionPriority);
        virtual UINT STDMETHODCALLTYPE GetEvictionPriority( void);
    };

    class D3D11Resource : public _D3D11Resource<ID3D11Resource>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Buffer : public _D3D11Resource<T>
    {
    public:
        size_t CalcSizeOfMappedMemory(
            /* [annotation] */
            __in  UINT Subresource,
            /* [annotation] */
            __in  UINT RowPitch,
            /* [annotation] */
            __in  UINT DepthPitch);
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_BUFFER_DESC *pDesc);
    };

    class D3D11Buffer : public _D3D11Buffer<ID3D11Buffer>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Texture1D : public _D3D11Resource<T>
    {
    public:
        size_t CalcSizeOfMappedMemory(
            /* [annotation] */
            __in  UINT Subresource,
            /* [annotation] */
            __in  UINT RowPitch,
            /* [annotation] */
            __in  UINT DepthPitch);
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_TEXTURE1D_DESC *pDesc);
    };

    class D3D11Texture1D : public _D3D11Texture1D<ID3D11Texture1D>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Texture2D : public SingleDeviceResource<_D3D11Resource<T>>
    {
    public:
        size_t CalcSizeOfMappedMemory(
            /* [annotation] */
            __in  UINT Subresource,
            /* [annotation] */
            __in  UINT RowPitch,
            /* [annotation] */
            __in  UINT DepthPitch);
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_TEXTURE2D_DESC *pDesc);
      virtual void SetDeviceIdx(int newIdx);
    };

    class D3D11Texture2D : public _D3D11Texture2D<ID3D11Texture2D>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Texture3D : public _D3D11Resource<T>
    {
    public:
        size_t CalcSizeOfMappedMemory(
            /* [annotation] */
            __in  UINT Subresource,
            /* [annotation] */
            __in  UINT RowPitch,
            /* [annotation] */
            __in  UINT DepthPitch);
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_TEXTURE3D_DESC *pDesc);
    };

    class D3D11Texture3D : public _D3D11Texture3D<ID3D11Texture3D>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11View : public _D3D11DeviceChild<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetResource( 
            /* [annotation] */ 
            __out  ID3D11Resource **ppResource);
    };

    class D3D11View : public _D3D11View<ID3D11View>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11ShaderResourceView : public _D3D11View<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc);
    };

    class D3D11ShaderResourceView : public _D3D11ShaderResourceView<ID3D11ShaderResourceView>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11RenderTargetView : public SingleDeviceResource<_D3D11View<T>>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_RENDER_TARGET_VIEW_DESC *pDesc);
      virtual void SetDeviceIdx(int newIdx);
    };

    class D3D11RenderTargetView : public _D3D11RenderTargetView<ID3D11RenderTargetView>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11DepthStencilView : public _D3D11View<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc);
    };

    class D3D11DepthStencilView : public _D3D11DepthStencilView<ID3D11DepthStencilView>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11UnorderedAccessView : public _D3D11View<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_UNORDERED_ACCESS_VIEW_DESC *pDesc);
    };

    class D3D11UnorderedAccessView : public _D3D11UnorderedAccessView<ID3D11UnorderedAccessView>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11VertexShader : public _D3D11DeviceChild<T>
    {
    public:
    };

    class D3D11VertexShader : public _D3D11VertexShader<ID3D11VertexShader>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11HullShader : public _D3D11DeviceChild<T>
    {
    public:
    };

    class D3D11HullShader : public _D3D11HullShader<ID3D11HullShader>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11DomainShader : public _D3D11DeviceChild<T>
    {
    public:
    };

    class D3D11DomainShader : public _D3D11DomainShader<ID3D11DomainShader>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11GeometryShader : public _D3D11DeviceChild<T>
    {
    public:
    };

    class D3D11GeometryShader : public _D3D11GeometryShader<ID3D11GeometryShader>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11PixelShader : public _D3D11DeviceChild<T>
    {
    public:
    };

    class D3D11PixelShader : public _D3D11PixelShader<ID3D11PixelShader>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11ComputeShader : public _D3D11DeviceChild<T>
    {
    public:
    };

    class D3D11ComputeShader : public _D3D11ComputeShader<ID3D11ComputeShader>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11InputLayout : public _D3D11DeviceChild<T>
    {
    public:
    };

    class D3D11InputLayout : public _D3D11InputLayout<ID3D11InputLayout>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11SamplerState : public _D3D11DeviceChild<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_SAMPLER_DESC *pDesc);
    };

    class D3D11SamplerState : public _D3D11SamplerState<ID3D11SamplerState>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Asynchronous : public _D3D11DeviceChild<T>
    {
    public:
        virtual UINT STDMETHODCALLTYPE GetDataSize( void);
    };

    class D3D11Asynchronous : public _D3D11Asynchronous<ID3D11Asynchronous>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Query : public _D3D11Asynchronous<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_QUERY_DESC *pDesc);
    };

    class D3D11Query : public _D3D11Query<ID3D11Query>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Predicate : public _D3D11Query<T>
    {
    public:
    };

    class D3D11Predicate : public _D3D11Predicate<ID3D11Predicate>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Counter : public _D3D11Asynchronous<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_COUNTER_DESC *pDesc);
    };

    class D3D11Counter : public _D3D11Counter<ID3D11Counter>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11ClassInstance : public _D3D11DeviceChild<T>
    {
    public:
        virtual void STDMETHODCALLTYPE GetClassLinkage( 
            /* [annotation] */ 
            __out  ID3D11ClassLinkage **ppLinkage);
        virtual void STDMETHODCALLTYPE GetDesc( 
            /* [annotation] */ 
            __out  D3D11_CLASS_INSTANCE_DESC *pDesc);
        virtual void STDMETHODCALLTYPE GetInstanceName( 
            /* [annotation] */ 
            __out_ecount_opt(*pBufferLength)  LPSTR pInstanceName,
            /* [annotation] */ 
            __inout  SIZE_T *pBufferLength);
        virtual void STDMETHODCALLTYPE GetTypeName( 
            /* [annotation] */ 
            __out_ecount_opt(*pBufferLength)  LPSTR pTypeName,
            /* [annotation] */ 
            __inout  SIZE_T *pBufferLength);
    };

    class D3D11ClassInstance : public _D3D11ClassInstance<ID3D11ClassInstance>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11ClassLinkage : public _D3D11DeviceChild<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetClassInstance( 
            /* [annotation] */ 
            __in  LPCSTR pClassInstanceName,
            /* [annotation] */ 
            __in  UINT InstanceIndex,
            /* [annotation] */ 
            __out  ID3D11ClassInstance **ppInstance);
        virtual HRESULT STDMETHODCALLTYPE CreateClassInstance( 
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
            __out  ID3D11ClassInstance **ppInstance);
    };

    class D3D11ClassLinkage : public _D3D11ClassLinkage<ID3D11ClassLinkage>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11CommandList : public _D3D11DeviceChild<T>
    {
    public:
        virtual UINT STDMETHODCALLTYPE GetContextFlags( void);
    };

    class D3D11CommandList : public _D3D11CommandList<ID3D11CommandList>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11DeviceContext : public _D3D11DeviceChild<T>
    {
        protected:                                                                                                
            ID3D11Device * _device;                                                                               
        public:                                                                                                   
            _D3D11DeviceContext();                                                                                
            ID3D11Device * GetDevice() const { return _device; }                                                  
            void SetDevice(ID3D11Device * val) { _device = val; }                                                 
        virtual void STDMETHODCALLTYPE CopyBufferNoneActiveDevices(                                               
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
        );
    public:
        virtual void STDMETHODCALLTYPE VSSetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers);
        virtual void STDMETHODCALLTYPE PSSetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE PSSetShader( 
            /* [annotation] */ 
            __in_opt  ID3D11PixelShader *pPixelShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances);
        virtual void STDMETHODCALLTYPE PSSetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers);
        virtual void STDMETHODCALLTYPE VSSetShader( 
            /* [annotation] */ 
            __in_opt  ID3D11VertexShader *pVertexShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances);
        virtual void STDMETHODCALLTYPE DrawIndexed( 
            /* [annotation] */ 
            __in  UINT IndexCount,
            /* [annotation] */ 
            __in  UINT StartIndexLocation,
            /* [annotation] */ 
            __in  INT BaseVertexLocation);
        virtual void STDMETHODCALLTYPE Draw( 
            /* [annotation] */ 
            __in  UINT VertexCount,
            /* [annotation] */ 
            __in  UINT StartVertexLocation);
        virtual HRESULT STDMETHODCALLTYPE Map( 
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in  UINT Subresource,
            /* [annotation] */ 
            __in  D3D11_MAP MapType,
            /* [annotation] */ 
            __in  UINT MapFlags,
            /* [annotation] */ 
            __out  D3D11_MAPPED_SUBRESOURCE *pMappedResource);
        virtual void STDMETHODCALLTYPE Unmap( 
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in  UINT Subresource);
        virtual void STDMETHODCALLTYPE PSSetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers);
        virtual void STDMETHODCALLTYPE IASetInputLayout( 
            /* [annotation] */ 
            __in_opt  ID3D11InputLayout *pInputLayout);
        virtual void STDMETHODCALLTYPE IASetVertexBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppVertexBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  const UINT *pStrides,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  const UINT *pOffsets);
        virtual void STDMETHODCALLTYPE IASetIndexBuffer( 
            /* [annotation] */ 
            __in_opt  ID3D11Buffer *pIndexBuffer,
            /* [annotation] */ 
            __in  DXGI_FORMAT Format,
            /* [annotation] */ 
            __in  UINT Offset);
        virtual void STDMETHODCALLTYPE DrawIndexedInstanced( 
            /* [annotation] */ 
            __in  UINT IndexCountPerInstance,
            /* [annotation] */ 
            __in  UINT InstanceCount,
            /* [annotation] */ 
            __in  UINT StartIndexLocation,
            /* [annotation] */ 
            __in  INT BaseVertexLocation,
            /* [annotation] */ 
            __in  UINT StartInstanceLocation);
        virtual void STDMETHODCALLTYPE DrawInstanced( 
            /* [annotation] */ 
            __in  UINT VertexCountPerInstance,
            /* [annotation] */ 
            __in  UINT InstanceCount,
            /* [annotation] */ 
            __in  UINT StartVertexLocation,
            /* [annotation] */ 
            __in  UINT StartInstanceLocation);
        virtual void STDMETHODCALLTYPE GSSetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers);
        virtual void STDMETHODCALLTYPE GSSetShader( 
            /* [annotation] */ 
            __in_opt  ID3D11GeometryShader *pShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances);
        virtual void STDMETHODCALLTYPE IASetPrimitiveTopology( 
            /* [annotation] */ 
            __in  D3D11_PRIMITIVE_TOPOLOGY Topology);
        virtual void STDMETHODCALLTYPE VSSetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE VSSetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers);
        virtual void STDMETHODCALLTYPE Begin( 
            /* [annotation] */ 
            __in  ID3D11Asynchronous *pAsync);
        virtual void STDMETHODCALLTYPE End( 
            /* [annotation] */ 
            __in  ID3D11Asynchronous *pAsync);
        virtual HRESULT STDMETHODCALLTYPE GetData( 
            /* [annotation] */ 
            __in  ID3D11Asynchronous *pAsync,
            /* [annotation] */ 
            __out_bcount_opt( DataSize )  void *pData,
            /* [annotation] */ 
            __in  UINT DataSize,
            /* [annotation] */ 
            __in  UINT GetDataFlags);
        virtual void STDMETHODCALLTYPE SetPredication( 
            /* [annotation] */ 
            __in_opt  ID3D11Predicate *pPredicate,
            /* [annotation] */ 
            __in  BOOL PredicateValue);
        virtual void STDMETHODCALLTYPE GSSetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE GSSetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers);
        virtual void STDMETHODCALLTYPE OMSetRenderTargets( 
            /* [annotation] */ 
            __in_range( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount_opt(NumViews)  ID3D11RenderTargetView *const *ppRenderTargetViews,
            /* [annotation] */ 
            __in_opt  ID3D11DepthStencilView *pDepthStencilView);
        virtual void STDMETHODCALLTYPE OMSetRenderTargetsAndUnorderedAccessViews( 
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
            __in_ecount_opt(NumUAVs)  const UINT *pUAVInitialCounts);
        virtual void STDMETHODCALLTYPE OMSetBlendState( 
            /* [annotation] */ 
            __in_opt  ID3D11BlendState *pBlendState,
            /* [annotation] */ 
            __in_opt  const FLOAT BlendFactor[ 4 ],
            /* [annotation] */ 
            __in  UINT SampleMask);
        virtual void STDMETHODCALLTYPE OMSetDepthStencilState( 
            /* [annotation] */ 
            __in_opt  ID3D11DepthStencilState *pDepthStencilState,
            /* [annotation] */ 
            __in  UINT StencilRef);
        virtual void STDMETHODCALLTYPE SOSetTargets( 
            /* [annotation] */ 
            __in_range( 0, D3D11_SO_BUFFER_SLOT_COUNT)  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount_opt(NumBuffers)  ID3D11Buffer *const *ppSOTargets,
            /* [annotation] */ 
            __in_ecount_opt(NumBuffers)  const UINT *pOffsets);
        virtual void STDMETHODCALLTYPE DrawAuto( void);
        virtual void STDMETHODCALLTYPE DrawIndexedInstancedIndirect( 
            /* [annotation] */ 
            __in  ID3D11Buffer *pBufferForArgs,
            /* [annotation] */ 
            __in  UINT AlignedByteOffsetForArgs);
        virtual void STDMETHODCALLTYPE DrawInstancedIndirect( 
            /* [annotation] */ 
            __in  ID3D11Buffer *pBufferForArgs,
            /* [annotation] */ 
            __in  UINT AlignedByteOffsetForArgs);
        virtual void STDMETHODCALLTYPE Dispatch( 
            /* [annotation] */ 
            __in  UINT ThreadGroupCountX,
            /* [annotation] */ 
            __in  UINT ThreadGroupCountY,
            /* [annotation] */ 
            __in  UINT ThreadGroupCountZ);
        virtual void STDMETHODCALLTYPE DispatchIndirect( 
            /* [annotation] */ 
            __in  ID3D11Buffer *pBufferForArgs,
            /* [annotation] */ 
            __in  UINT AlignedByteOffsetForArgs);
        virtual void STDMETHODCALLTYPE RSSetState( 
            /* [annotation] */ 
            __in_opt  ID3D11RasterizerState *pRasterizerState);
        virtual void STDMETHODCALLTYPE RSSetViewports( 
            /* [annotation] */ 
            __in_range(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumViewports,
            /* [annotation] */ 
            __in_ecount_opt(NumViewports)  const D3D11_VIEWPORT *pViewports);
        virtual void STDMETHODCALLTYPE RSSetScissorRects( 
            /* [annotation] */ 
            __in_range(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)  UINT NumRects,
            /* [annotation] */ 
            __in_ecount_opt(NumRects)  const D3D11_RECT *pRects);
        virtual void STDMETHODCALLTYPE CopySubresourceRegion( 
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
            __in_opt  const D3D11_BOX *pSrcBox);
        virtual void STDMETHODCALLTYPE CopyResource( 
            /* [annotation] */ 
            __in  ID3D11Resource *pDstResource,
            /* [annotation] */ 
            __in  ID3D11Resource *pSrcResource);
        virtual void STDMETHODCALLTYPE UpdateSubresource( 
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
            __in  UINT SrcDepthPitch);
        virtual void STDMETHODCALLTYPE CopyStructureCount( 
            /* [annotation] */ 
            __in  ID3D11Buffer *pDstBuffer,
            /* [annotation] */ 
            __in  UINT DstAlignedByteOffset,
            /* [annotation] */ 
            __in  ID3D11UnorderedAccessView *pSrcView);
        virtual void STDMETHODCALLTYPE ClearRenderTargetView( 
            /* [annotation] */ 
            __in  ID3D11RenderTargetView *pRenderTargetView,
            /* [annotation] */ 
            __in  const FLOAT ColorRGBA[ 4 ]);
        virtual void STDMETHODCALLTYPE ClearUnorderedAccessViewUint( 
            /* [annotation] */ 
            __in  ID3D11UnorderedAccessView *pUnorderedAccessView,
            /* [annotation] */ 
            __in  const UINT Values[ 4 ]);
        virtual void STDMETHODCALLTYPE ClearUnorderedAccessViewFloat( 
            /* [annotation] */ 
            __in  ID3D11UnorderedAccessView *pUnorderedAccessView,
            /* [annotation] */ 
            __in  const FLOAT Values[ 4 ]);
        virtual void STDMETHODCALLTYPE ClearDepthStencilView( 
            /* [annotation] */ 
            __in  ID3D11DepthStencilView *pDepthStencilView,
            /* [annotation] */ 
            __in  UINT ClearFlags,
            /* [annotation] */ 
            __in  FLOAT Depth,
            /* [annotation] */ 
            __in  UINT8 Stencil);
        virtual void STDMETHODCALLTYPE GenerateMips( 
            /* [annotation] */ 
            __in  ID3D11ShaderResourceView *pShaderResourceView);
        virtual void STDMETHODCALLTYPE SetResourceMinLOD( 
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            FLOAT MinLOD);
        virtual FLOAT STDMETHODCALLTYPE GetResourceMinLOD( 
            /* [annotation] */ 
            __in  ID3D11Resource *pResource);
        virtual void STDMETHODCALLTYPE ResolveSubresource( 
            /* [annotation] */ 
            __in  ID3D11Resource *pDstResource,
            /* [annotation] */ 
            __in  UINT DstSubresource,
            /* [annotation] */ 
            __in  ID3D11Resource *pSrcResource,
            /* [annotation] */ 
            __in  UINT SrcSubresource,
            /* [annotation] */ 
            __in  DXGI_FORMAT Format);
        virtual void STDMETHODCALLTYPE ExecuteCommandList( 
            /* [annotation] */ 
            __in  ID3D11CommandList *pCommandList,
            BOOL RestoreContextState);
        virtual void STDMETHODCALLTYPE HSSetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE HSSetShader( 
            /* [annotation] */ 
            __in_opt  ID3D11HullShader *pHullShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances);
        virtual void STDMETHODCALLTYPE HSSetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers);
        virtual void STDMETHODCALLTYPE HSSetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers);
        virtual void STDMETHODCALLTYPE DSSetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE DSSetShader( 
            /* [annotation] */ 
            __in_opt  ID3D11DomainShader *pDomainShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances);
        virtual void STDMETHODCALLTYPE DSSetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers);
        virtual void STDMETHODCALLTYPE DSSetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers);
        virtual void STDMETHODCALLTYPE CSSetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __in_ecount(NumViews)  ID3D11ShaderResourceView *const *ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE CSSetUnorderedAccessViews( 
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - StartSlot )  UINT NumUAVs,
            /* [annotation] */ 
            __in_ecount(NumUAVs)  ID3D11UnorderedAccessView *const *ppUnorderedAccessViews,
            /* [annotation] */ 
            __in_ecount(NumUAVs)  const UINT *pUAVInitialCounts);
        virtual void STDMETHODCALLTYPE CSSetShader( 
            /* [annotation] */ 
            __in_opt  ID3D11ComputeShader *pComputeShader,
            /* [annotation] */ 
            __in_ecount_opt(NumClassInstances)  ID3D11ClassInstance *const *ppClassInstances,
            UINT NumClassInstances);
        virtual void STDMETHODCALLTYPE CSSetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __in_ecount(NumSamplers)  ID3D11SamplerState *const *ppSamplers);
        virtual void STDMETHODCALLTYPE CSSetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __in_ecount(NumBuffers)  ID3D11Buffer *const *ppConstantBuffers);
        virtual void STDMETHODCALLTYPE VSGetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers);
        virtual void STDMETHODCALLTYPE PSGetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE PSGetShader( 
            /* [annotation] */ 
            __out_opt  ID3D11PixelShader **ppPixelShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances);
        virtual void STDMETHODCALLTYPE PSGetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers);
        virtual void STDMETHODCALLTYPE VSGetShader( 
            /* [annotation] */ 
            __out_opt  ID3D11VertexShader **ppVertexShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances);
        virtual void STDMETHODCALLTYPE PSGetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers);
        virtual void STDMETHODCALLTYPE IAGetInputLayout( 
            /* [annotation] */ 
            __out  ID3D11InputLayout **ppInputLayout);
        virtual void STDMETHODCALLTYPE IAGetVertexBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount_opt(NumBuffers)  ID3D11Buffer **ppVertexBuffers,
            /* [annotation] */ 
            __out_ecount_opt(NumBuffers)  UINT *pStrides,
            /* [annotation] */ 
            __out_ecount_opt(NumBuffers)  UINT *pOffsets);
        virtual void STDMETHODCALLTYPE IAGetIndexBuffer( 
            /* [annotation] */ 
            __out_opt  ID3D11Buffer **pIndexBuffer,
            /* [annotation] */ 
            __out_opt  DXGI_FORMAT *Format,
            /* [annotation] */ 
            __out_opt  UINT *Offset);
        virtual void STDMETHODCALLTYPE GSGetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers);
        virtual void STDMETHODCALLTYPE GSGetShader( 
            /* [annotation] */ 
            __out_opt  ID3D11GeometryShader **ppGeometryShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances);
        virtual void STDMETHODCALLTYPE IAGetPrimitiveTopology( 
            /* [annotation] */ 
            __out  D3D11_PRIMITIVE_TOPOLOGY *pTopology);
        virtual void STDMETHODCALLTYPE VSGetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE VSGetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers);
        virtual void STDMETHODCALLTYPE GetPredication( 
            /* [annotation] */ 
            __out_opt  ID3D11Predicate **ppPredicate,
            /* [annotation] */ 
            __out_opt  BOOL *pPredicateValue);
        virtual void STDMETHODCALLTYPE GSGetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE GSGetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers);
        virtual void STDMETHODCALLTYPE OMGetRenderTargets( 
            /* [annotation] */ 
            __in_range( 0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount_opt(NumViews)  ID3D11RenderTargetView **ppRenderTargetViews,
            /* [annotation] */ 
            __out_opt  ID3D11DepthStencilView **ppDepthStencilView);
        virtual void STDMETHODCALLTYPE OMGetRenderTargetsAndUnorderedAccessViews( 
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
            __out_ecount_opt(NumUAVs)  ID3D11UnorderedAccessView **ppUnorderedAccessViews);
        virtual void STDMETHODCALLTYPE OMGetBlendState( 
            /* [annotation] */ 
            __out_opt  ID3D11BlendState **ppBlendState,
            /* [annotation] */ 
            __out_opt  FLOAT BlendFactor[ 4 ],
            /* [annotation] */ 
            __out_opt  UINT *pSampleMask);
        virtual void STDMETHODCALLTYPE OMGetDepthStencilState( 
            /* [annotation] */ 
            __out_opt  ID3D11DepthStencilState **ppDepthStencilState,
            /* [annotation] */ 
            __out_opt  UINT *pStencilRef);
        virtual void STDMETHODCALLTYPE SOGetTargets( 
            /* [annotation] */ 
            __in_range( 0, D3D11_SO_BUFFER_SLOT_COUNT )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppSOTargets);
        virtual void STDMETHODCALLTYPE RSGetState( 
            /* [annotation] */ 
            __out  ID3D11RasterizerState **ppRasterizerState);
        virtual void STDMETHODCALLTYPE RSGetViewports( 
            /* [annotation] */ 
            __inout /*_range(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE )*/   UINT *pNumViewports,
            /* [annotation] */ 
            __out_ecount_opt(*pNumViewports)  D3D11_VIEWPORT *pViewports);
        virtual void STDMETHODCALLTYPE RSGetScissorRects( 
            /* [annotation] */ 
            __inout /*_range(0, D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE )*/   UINT *pNumRects,
            /* [annotation] */ 
            __out_ecount_opt(*pNumRects)  D3D11_RECT *pRects);
        virtual void STDMETHODCALLTYPE HSGetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE HSGetShader( 
            /* [annotation] */ 
            __out_opt  ID3D11HullShader **ppHullShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances);
        virtual void STDMETHODCALLTYPE HSGetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers);
        virtual void STDMETHODCALLTYPE HSGetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers);
        virtual void STDMETHODCALLTYPE DSGetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE DSGetShader( 
            /* [annotation] */ 
            __out_opt  ID3D11DomainShader **ppDomainShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances);
        virtual void STDMETHODCALLTYPE DSGetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers);
        virtual void STDMETHODCALLTYPE DSGetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers);
        virtual void STDMETHODCALLTYPE CSGetShaderResources( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT - StartSlot )  UINT NumViews,
            /* [annotation] */ 
            __out_ecount(NumViews)  ID3D11ShaderResourceView **ppShaderResourceViews);
        virtual void STDMETHODCALLTYPE CSGetUnorderedAccessViews( 
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_PS_CS_UAV_REGISTER_COUNT - StartSlot )  UINT NumUAVs,
            /* [annotation] */ 
            __out_ecount(NumUAVs)  ID3D11UnorderedAccessView **ppUnorderedAccessViews);
        virtual void STDMETHODCALLTYPE CSGetShader( 
            /* [annotation] */ 
            __out_opt  ID3D11ComputeShader **ppComputeShader,
            /* [annotation] */ 
            __out_ecount_opt(*pNumClassInstances)  ID3D11ClassInstance **ppClassInstances,
            /* [annotation] */ 
            __inout_opt  UINT *pNumClassInstances);
        virtual void STDMETHODCALLTYPE CSGetSamplers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT - StartSlot )  UINT NumSamplers,
            /* [annotation] */ 
            __out_ecount(NumSamplers)  ID3D11SamplerState **ppSamplers);
        virtual void STDMETHODCALLTYPE CSGetConstantBuffers( 
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1 )  UINT StartSlot,
            /* [annotation] */ 
            __in_range( 0, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - StartSlot )  UINT NumBuffers,
            /* [annotation] */ 
            __out_ecount(NumBuffers)  ID3D11Buffer **ppConstantBuffers);
        virtual void STDMETHODCALLTYPE ClearState( void);
        virtual void STDMETHODCALLTYPE Flush( void);
        virtual D3D11_DEVICE_CONTEXT_TYPE STDMETHODCALLTYPE GetType( void);
        virtual UINT STDMETHODCALLTYPE GetContextFlags( void);
        virtual HRESULT STDMETHODCALLTYPE FinishCommandList( 
            BOOL RestoreDeferredContextState,
            /* [annotation] */ 
            __out_opt  ID3D11CommandList **ppCommandList);
    };

    class D3D11DeviceContext : public _D3D11DeviceContext<ID3D11DeviceContext>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Device : public _Unknown<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateBuffer( 
            /* [annotation] */ 
            __in  const D3D11_BUFFER_DESC *pDesc,
            /* [annotation] */ 
            __in_opt  const D3D11_SUBRESOURCE_DATA *pInitialData,
            /* [annotation] */ 
            __out_opt  ID3D11Buffer **ppBuffer);
        virtual HRESULT STDMETHODCALLTYPE CreateTexture1D( 
            /* [annotation] */ 
            __in  const D3D11_TEXTURE1D_DESC *pDesc,
            /* [annotation] */ 
            __in_xcount_opt(pDesc->MipLevels * pDesc->ArraySize)  const D3D11_SUBRESOURCE_DATA *pInitialData,
            /* [annotation] */ 
            __out_opt  ID3D11Texture1D **ppTexture1D);
        virtual HRESULT STDMETHODCALLTYPE CreateTexture2D( 
            /* [annotation] */ 
            __in  const D3D11_TEXTURE2D_DESC *pDesc,
            /* [annotation] */ 
            __in_xcount_opt(pDesc->MipLevels * pDesc->ArraySize)  const D3D11_SUBRESOURCE_DATA *pInitialData,
            /* [annotation] */ 
            __out_opt  ID3D11Texture2D **ppTexture2D);
        virtual HRESULT STDMETHODCALLTYPE CreateTexture3D( 
            /* [annotation] */ 
            __in  const D3D11_TEXTURE3D_DESC *pDesc,
            /* [annotation] */ 
            __in_xcount_opt(pDesc->MipLevels)  const D3D11_SUBRESOURCE_DATA *pInitialData,
            /* [annotation] */ 
            __out_opt  ID3D11Texture3D **ppTexture3D);
        virtual HRESULT STDMETHODCALLTYPE CreateShaderResourceView( 
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in_opt  const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc,
            /* [annotation] */ 
            __out_opt  ID3D11ShaderResourceView **ppSRView);
        virtual HRESULT STDMETHODCALLTYPE CreateUnorderedAccessView( 
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in_opt  const D3D11_UNORDERED_ACCESS_VIEW_DESC *pDesc,
            /* [annotation] */ 
            __out_opt  ID3D11UnorderedAccessView **ppUAView);
        virtual HRESULT STDMETHODCALLTYPE CreateRenderTargetView( 
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in_opt  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
            /* [annotation] */ 
            __out_opt  ID3D11RenderTargetView **ppRTView);
        virtual HRESULT STDMETHODCALLTYPE CreateDepthStencilView( 
            /* [annotation] */ 
            __in  ID3D11Resource *pResource,
            /* [annotation] */ 
            __in_opt  const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc,
            /* [annotation] */ 
            __out_opt  ID3D11DepthStencilView **ppDepthStencilView);
        virtual HRESULT STDMETHODCALLTYPE CreateInputLayout( 
            /* [annotation] */ 
            __in_ecount(NumElements)  const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs,
            /* [annotation] */ 
            __in_range( 0, D3D11_IA_VERTEX_INPUT_STRUCTURE_ELEMENT_COUNT )  UINT NumElements,
            /* [annotation] */ 
            __in  const void *pShaderBytecodeWithInputSignature,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __out_opt  ID3D11InputLayout **ppInputLayout);
        virtual HRESULT STDMETHODCALLTYPE CreateVertexShader( 
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11VertexShader **ppVertexShader);
        virtual HRESULT STDMETHODCALLTYPE CreateGeometryShader( 
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11GeometryShader **ppGeometryShader);
        virtual HRESULT STDMETHODCALLTYPE CreateGeometryShaderWithStreamOutput( 
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
            __out_opt  ID3D11GeometryShader **ppGeometryShader);
        virtual HRESULT STDMETHODCALLTYPE CreatePixelShader( 
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11PixelShader **ppPixelShader);
        virtual HRESULT STDMETHODCALLTYPE CreateHullShader( 
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11HullShader **ppHullShader);
        virtual HRESULT STDMETHODCALLTYPE CreateDomainShader( 
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11DomainShader **ppDomainShader);
        virtual HRESULT STDMETHODCALLTYPE CreateComputeShader( 
            /* [annotation] */ 
            __in  const void *pShaderBytecode,
            /* [annotation] */ 
            __in  SIZE_T BytecodeLength,
            /* [annotation] */ 
            __in_opt  ID3D11ClassLinkage *pClassLinkage,
            /* [annotation] */ 
            __out_opt  ID3D11ComputeShader **ppComputeShader);
        virtual HRESULT STDMETHODCALLTYPE CreateClassLinkage( 
            /* [annotation] */ 
            __out  ID3D11ClassLinkage **ppLinkage);
        virtual HRESULT STDMETHODCALLTYPE CreateBlendState( 
            /* [annotation] */ 
            __in  const D3D11_BLEND_DESC *pBlendStateDesc,
            /* [annotation] */ 
            __out_opt  ID3D11BlendState **ppBlendState);
        virtual HRESULT STDMETHODCALLTYPE CreateDepthStencilState( 
            /* [annotation] */ 
            __in  const D3D11_DEPTH_STENCIL_DESC *pDepthStencilDesc,
            /* [annotation] */ 
            __out_opt  ID3D11DepthStencilState **ppDepthStencilState);
        virtual HRESULT STDMETHODCALLTYPE CreateRasterizerState( 
            /* [annotation] */ 
            __in  const D3D11_RASTERIZER_DESC *pRasterizerDesc,
            /* [annotation] */ 
            __out_opt  ID3D11RasterizerState **ppRasterizerState);
        virtual HRESULT STDMETHODCALLTYPE CreateSamplerState( 
            /* [annotation] */ 
            __in  const D3D11_SAMPLER_DESC *pSamplerDesc,
            /* [annotation] */ 
            __out_opt  ID3D11SamplerState **ppSamplerState);
        virtual HRESULT STDMETHODCALLTYPE CreateQuery( 
            /* [annotation] */ 
            __in  const D3D11_QUERY_DESC *pQueryDesc,
            /* [annotation] */ 
            __out_opt  ID3D11Query **ppQuery);
        virtual HRESULT STDMETHODCALLTYPE CreatePredicate( 
            /* [annotation] */ 
            __in  const D3D11_QUERY_DESC *pPredicateDesc,
            /* [annotation] */ 
            __out_opt  ID3D11Predicate **ppPredicate);
        virtual HRESULT STDMETHODCALLTYPE CreateCounter( 
            /* [annotation] */ 
            __in  const D3D11_COUNTER_DESC *pCounterDesc,
            /* [annotation] */ 
            __out_opt  ID3D11Counter **ppCounter);
        virtual HRESULT STDMETHODCALLTYPE CreateDeferredContext( 
            UINT ContextFlags,
            /* [annotation] */ 
            __out_opt  ID3D11DeviceContext **ppDeferredContext);
        virtual HRESULT STDMETHODCALLTYPE OpenSharedResource( 
            /* [annotation] */ 
            __in  HANDLE hResource,
            /* [annotation] */ 
            __in  REFIID ReturnedInterface,
            /* [annotation] */ 
            __out_opt  void **ppResource);
        virtual HRESULT STDMETHODCALLTYPE CheckFormatSupport( 
            /* [annotation] */ 
            __in  DXGI_FORMAT Format,
            /* [annotation] */ 
            __out  UINT *pFormatSupport);
        virtual HRESULT STDMETHODCALLTYPE CheckMultisampleQualityLevels( 
            /* [annotation] */ 
            __in  DXGI_FORMAT Format,
            /* [annotation] */ 
            __in  UINT SampleCount,
            /* [annotation] */ 
            __out  UINT *pNumQualityLevels);
        virtual void STDMETHODCALLTYPE CheckCounterInfo( 
            /* [annotation] */ 
            __out  D3D11_COUNTER_INFO *pCounterInfo);
        virtual HRESULT STDMETHODCALLTYPE CheckCounter( 
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
            __inout_opt  UINT *pDescriptionLength);
        virtual HRESULT STDMETHODCALLTYPE CheckFeatureSupport( 
            D3D11_FEATURE Feature,
            /* [annotation] */ 
            __out_bcount(FeatureSupportDataSize)  void *pFeatureSupportData,
            UINT FeatureSupportDataSize);
        virtual HRESULT STDMETHODCALLTYPE GetPrivateData( 
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __inout  UINT *pDataSize,
            /* [annotation] */ 
            __out_bcount_opt(*pDataSize)  void *pData);
        virtual HRESULT STDMETHODCALLTYPE SetPrivateData( 
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __in  UINT DataSize,
            /* [annotation] */ 
            __in_bcount_opt(DataSize)  const void *pData);
        virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface( 
            /* [annotation] */ 
            __in  REFGUID guid,
            /* [annotation] */ 
            __in_opt  const IUnknown *pData);
        virtual D3D_FEATURE_LEVEL STDMETHODCALLTYPE GetFeatureLevel( void);
        virtual UINT STDMETHODCALLTYPE GetCreationFlags( void);
        virtual HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason( void);
        virtual void STDMETHODCALLTYPE GetImmediateContext( 
            /* [annotation] */ 
            __out  ID3D11DeviceContext **ppImmediateContext);
        virtual HRESULT STDMETHODCALLTYPE SetExceptionMode( 
            UINT RaiseFlags);
        virtual UINT STDMETHODCALLTYPE GetExceptionMode( void);
    };

    class D3D11Device : public _D3D11Device<ID3D11Device>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11Debug : public _Unknown<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetFeatureMask( 
            UINT Mask);
        virtual UINT STDMETHODCALLTYPE GetFeatureMask( void);
        virtual HRESULT STDMETHODCALLTYPE SetPresentPerRenderOpDelay( 
            UINT Milliseconds);
        virtual UINT STDMETHODCALLTYPE GetPresentPerRenderOpDelay( void);
        virtual HRESULT STDMETHODCALLTYPE SetSwapChain( 
            /* [annotation] */ 
            __in_opt  IDXGISwapChain *pSwapChain);
        virtual HRESULT STDMETHODCALLTYPE GetSwapChain( 
            /* [annotation] */ 
            __out  IDXGISwapChain **ppSwapChain);
        virtual HRESULT STDMETHODCALLTYPE ValidateContext( 
            /* [annotation] */ 
            __in  ID3D11DeviceContext *pContext);
        virtual HRESULT STDMETHODCALLTYPE ReportLiveDeviceObjects( 
            D3D11_RLDO_FLAGS Flags);
        virtual HRESULT STDMETHODCALLTYPE ValidateContextForDispatch( 
            /* [annotation] */ 
            __in  ID3D11DeviceContext *pContext);
    };

    class D3D11Debug : public _D3D11Debug<ID3D11Debug>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11SwitchToRef : public _Unknown<T>
    {
    public:
        virtual BOOL STDMETHODCALLTYPE SetUseRef( 
            BOOL UseRef);
        virtual BOOL STDMETHODCALLTYPE GetUseRef( void);
    };

    class D3D11SwitchToRef : public _D3D11SwitchToRef<ID3D11SwitchToRef>
    {
    };

//---------------------------------------------------------------------

    template <class T>
    class _D3D11InfoQueue : public _Unknown<T>
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetMessageCountLimit( 
            /* [annotation] */ 
            __in  UINT64 MessageCountLimit);
        virtual void STDMETHODCALLTYPE ClearStoredMessages( void);
        virtual HRESULT STDMETHODCALLTYPE GetMessage( 
            /* [annotation] */ 
            __in  UINT64 MessageIndex,
            /* [annotation] */ 
            __out_bcount_opt(*pMessageByteLength)  D3D11_MESSAGE *pMessage,
            /* [annotation] */ 
            __inout  SIZE_T *pMessageByteLength);
        virtual UINT64 STDMETHODCALLTYPE GetNumMessagesAllowedByStorageFilter( void);
        virtual UINT64 STDMETHODCALLTYPE GetNumMessagesDeniedByStorageFilter( void);
        virtual UINT64 STDMETHODCALLTYPE GetNumStoredMessages( void);
        virtual UINT64 STDMETHODCALLTYPE GetNumStoredMessagesAllowedByRetrievalFilter( void);
        virtual UINT64 STDMETHODCALLTYPE GetNumMessagesDiscardedByMessageCountLimit( void);
        virtual UINT64 STDMETHODCALLTYPE GetMessageCountLimit( void);
        virtual HRESULT STDMETHODCALLTYPE AddStorageFilterEntries( 
            /* [annotation] */ 
            __in  D3D11_INFO_QUEUE_FILTER *pFilter);
        virtual HRESULT STDMETHODCALLTYPE GetStorageFilter( 
            /* [annotation] */ 
            __out_bcount_opt(*pFilterByteLength)  D3D11_INFO_QUEUE_FILTER *pFilter,
            /* [annotation] */ 
            __inout  SIZE_T *pFilterByteLength);
        virtual void STDMETHODCALLTYPE ClearStorageFilter( void);
        virtual HRESULT STDMETHODCALLTYPE PushEmptyStorageFilter( void);
        virtual HRESULT STDMETHODCALLTYPE PushCopyOfStorageFilter( void);
        virtual HRESULT STDMETHODCALLTYPE PushStorageFilter( 
            /* [annotation] */ 
            __in  D3D11_INFO_QUEUE_FILTER *pFilter);
        virtual void STDMETHODCALLTYPE PopStorageFilter( void);
        virtual UINT STDMETHODCALLTYPE GetStorageFilterStackSize( void);
        virtual HRESULT STDMETHODCALLTYPE AddRetrievalFilterEntries( 
            /* [annotation] */ 
            __in  D3D11_INFO_QUEUE_FILTER *pFilter);
        virtual HRESULT STDMETHODCALLTYPE GetRetrievalFilter( 
            /* [annotation] */ 
            __out_bcount_opt(*pFilterByteLength)  D3D11_INFO_QUEUE_FILTER *pFilter,
            /* [annotation] */ 
            __inout  SIZE_T *pFilterByteLength);
        virtual void STDMETHODCALLTYPE ClearRetrievalFilter( void);
        virtual HRESULT STDMETHODCALLTYPE PushEmptyRetrievalFilter( void);
        virtual HRESULT STDMETHODCALLTYPE PushCopyOfRetrievalFilter( void);
        virtual HRESULT STDMETHODCALLTYPE PushRetrievalFilter( 
            /* [annotation] */ 
            __in  D3D11_INFO_QUEUE_FILTER *pFilter);
        virtual void STDMETHODCALLTYPE PopRetrievalFilter( void);
        virtual UINT STDMETHODCALLTYPE GetRetrievalFilterStackSize( void);
        virtual HRESULT STDMETHODCALLTYPE AddMessage( 
            /* [annotation] */ 
            __in  D3D11_MESSAGE_CATEGORY Category,
            /* [annotation] */ 
            __in  D3D11_MESSAGE_SEVERITY Severity,
            /* [annotation] */ 
            __in  D3D11_MESSAGE_ID ID,
            /* [annotation] */ 
            __in  LPCSTR pDescription);
        virtual HRESULT STDMETHODCALLTYPE AddApplicationMessage( 
            /* [annotation] */ 
            __in  D3D11_MESSAGE_SEVERITY Severity,
            /* [annotation] */ 
            __in  LPCSTR pDescription);
        virtual HRESULT STDMETHODCALLTYPE SetBreakOnCategory( 
            /* [annotation] */ 
            __in  D3D11_MESSAGE_CATEGORY Category,
            /* [annotation] */ 
            __in  BOOL bEnable);
        virtual HRESULT STDMETHODCALLTYPE SetBreakOnSeverity( 
            /* [annotation] */ 
            __in  D3D11_MESSAGE_SEVERITY Severity,
            /* [annotation] */ 
            __in  BOOL bEnable);
        virtual HRESULT STDMETHODCALLTYPE SetBreakOnID( 
            /* [annotation] */ 
            __in  D3D11_MESSAGE_ID ID,
            /* [annotation] */ 
            __in  BOOL bEnable);
        virtual BOOL STDMETHODCALLTYPE GetBreakOnCategory( 
            /* [annotation] */ 
            __in  D3D11_MESSAGE_CATEGORY Category);
        virtual BOOL STDMETHODCALLTYPE GetBreakOnSeverity( 
            /* [annotation] */ 
            __in  D3D11_MESSAGE_SEVERITY Severity);
        virtual BOOL STDMETHODCALLTYPE GetBreakOnID( 
            /* [annotation] */ 
            __in  D3D11_MESSAGE_ID ID);
        virtual void STDMETHODCALLTYPE SetMuteDebugOutput( 
            /* [annotation] */ 
            __in  BOOL bMute);
        virtual BOOL STDMETHODCALLTYPE GetMuteDebugOutput( void);
    };

    class D3D11InfoQueue : public _D3D11InfoQueue<ID3D11InfoQueue>
    {
    };
} // namespace D3D11MultiDevice
