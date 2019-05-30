#ifndef __D3D11RENDERTARGET_H__
#define __D3D11RENDERTARGET_H__

#include "OgreD3D11Prerequisites.h"

namespace Ogre
{
    class D3D11RenderTarget
    {
    public:
        virtual ~D3D11RenderTarget() {}

        virtual uint getNumberOfViews() const = 0;
        virtual ID3D11Texture2D* getSurface(uint index = 0) const = 0;
        virtual ID3D11RenderTargetView* getRenderTargetView(uint index = 0) const = 0;

    protected:
        D3D11RenderTarget() {}
    };

} // namespace Ogre

#endif
