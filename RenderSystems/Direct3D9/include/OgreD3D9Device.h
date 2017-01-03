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
#ifndef __D3D9Device_H__
#define __D3D9Device_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreRenderTarget.h"

namespace Ogre {

    class D3D9RenderWindow;
    class D3D9DeviceManager;

    /** High level interface of Direct3D9 Device.
    Provide useful methods for device handling.
    */
    class _OgreD3D9Export D3D9Device : public ResourceAlloc
    {

    // Interface.
    public:
        void                    attachRenderWindow      (D3D9RenderWindow* renderWindow);
        void                    detachRenderWindow      (D3D9RenderWindow* renderWindow);
    
        bool                    acquire                 ();
        
        void                    release                 ();     
        void                    destroy                 ();     
        
        bool                    isDeviceLost            ();             
        IDirect3DDevice9*       getD3D9Device           ();
                    
        UINT                    getAdapterNumber        () const;
        D3DDEVTYPE              getDeviceType           () const;
        bool                    isMultihead             () const;                   
        bool                    isAutoDepthStencil      () const;
        bool                    isFullScreen            () const;
        
        const D3DCAPS9&         getD3D9DeviceCaps       () const;
        D3DFORMAT               getBackBufferFormat     () const;

        bool                    validate                (D3D9RenderWindow* renderWindow);
        void                    invalidate              (D3D9RenderWindow* renderWindow);

        void                    present                 (D3D9RenderWindow* renderWindow);
        
        IDirect3DSurface9*      getDepthBuffer          (D3D9RenderWindow* renderWindow);
        IDirect3DSurface9*      getBackBuffer           (D3D9RenderWindow* renderWindow);

        uint                    getRenderWindowCount    () const;
        D3D9RenderWindow*       getRenderWindow         (uint index);
        uint                    getLastPresentFrame     () const { return mLastPresentFrame; }

        void                    setAdapterOrdinalIndex  (D3D9RenderWindow* renderWindow, uint adapterOrdinalInGroupIndex);
        void                    copyContentsToMemory(D3D9RenderWindow* window, const Box& src, const PixelBox &dst, RenderTarget::FrameBuffer buffer);
        void                    clearDeviceStreams      ();
        int                     getVBlankMissCount      (D3D9RenderWindow* renderWindow);
    
    public:
        D3D9Device  (D3D9DeviceManager* deviceManager,
                     UINT adapterNumber, 
                     HMONITOR hMonitor, 
                     D3DDEVTYPE devType, 
                     DWORD behaviorFlags);
        ~D3D9Device ();

    protected:          
        D3D9DeviceManager*              mDeviceManager;             // The manager of this device instance.
        IDirect3DDevice9*               mDevice;                    // Will hold the device interface.              
        UINT                            mAdapterNumber;             // The adapter that this device belongs to. 
        HMONITOR                        mMonitor;                   // The monitor that this device belongs to.
        D3DDEVTYPE                      mDeviceType;                // Device type. 
        static HWND                     msSharedFocusWindow;        // The shared focus window in case of multiple full screen render windows.
        HWND                            mFocusWindow;               // The focus window this device attached to.            
        DWORD                           mBehaviorFlags;             // The behavior of this device.     
        D3DPRESENT_PARAMETERS*          mPresentationParams;        // Presentation parameters which the device was created with. May be
                                                                    // an array of presentation parameters in case of multi-head device.                
        UINT                            mPresentationParamsCount;   // Number of presentation parameters elements.      
        D3DCAPS9                        mD3D9DeviceCaps;            // Device caps. 
        bool                            mD3D9DeviceCapsValid;       // True if device caps initialized.             
        D3DDEVICE_CREATION_PARAMETERS   mCreationParams;            // Creation parameters.
        uint                            mLastPresentFrame;          // Last frame that this device present method called.
        bool                            mDeviceLost;                // True if device entered lost state.
        D3DPRESENTSTATS                 mPreviousPresentStats;          // We save the previous present stats - so we can detect a "vblank miss"
        bool                            mPreviousPresentStatsIsValid;   // Does mLastPresentStats data is valid (it isn't if when you start or resize the window)
        uint                            mVBlankMissCount;           // Number of times we missed the v sync blank
    
        struct RenderWindowResources
        {
            IDirect3DSwapChain9*    swapChain;                      // Swap chain interface.
            IDirect3DSwapChain9Ex * swapChain9Ex;                   // The 9Ex version of the chain is needed for the v synk blank stats
            uint                    adapterOrdinalInGroupIndex;     // Relative index of the render window in the group.
            uint                    presentParametersIndex;         // Index of present parameter in the shared array of the device.
            IDirect3DSurface9*      backBuffer;                     // The back buffer of the render window.
            IDirect3DSurface9*      depthBuffer;                    // The depth buffer of the render window.
            D3DPRESENT_PARAMETERS   presentParameters;              // Present parameters of the render window.
            bool                    acquired;                       // True if resources acquired.          
        };      
        typedef map<D3D9RenderWindow*, RenderWindowResources*>::type RenderWindowToResourcesMap;
        typedef RenderWindowToResourcesMap::iterator                 RenderWindowToResourcesIterator;

        RenderWindowToResourcesMap mMapRenderWindowToResources;     // Map between render window to resources.


    protected:
        RenderWindowToResourcesIterator getRenderWindowIterator (D3D9RenderWindow* renderWindow);

        bool                    acquire                         (D3D9RenderWindow* renderWindow);
        bool                    reset                           ();
        void                    updatePresentationParameters    ();
        void                    updateRenderWindowsIndices      ();
        
        void                    createD3D9Device                ();
        void                    releaseD3D9Device               ();
        void                    releaseRenderWindowResources    (RenderWindowResources* renderWindowResources);
        void                    acquireRenderWindowResources    (RenderWindowToResourcesIterator it);       
        void                    setupDeviceStates               ();
        void                    notifyDeviceLost                ();

        void                    validateFocusWindow             ();
        void                    validateBackBufferSize          (D3D9RenderWindow* renderWindow);
        bool                    validateDisplayMonitor          (D3D9RenderWindow* renderWindow);
        bool                    validateDeviceState             (D3D9RenderWindow* renderWindow);
        bool                    isSwapChainWindow               (D3D9RenderWindow* renderWindow);
        D3D9RenderWindow*       getPrimaryWindow                ();
        void                    setSharedWindowHandle           (HWND hSharedHWND);

    private:
        friend class D3D9DeviceManager;
        friend class D3D9RenderSystem;
    };
}
#endif
