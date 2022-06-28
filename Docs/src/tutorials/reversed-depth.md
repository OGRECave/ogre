# Reversed Depth {#reversed-depth}

By default %Ogre is using the standard depth setup, which results in a [hyperbolical depth value distribution](https://developer.nvidia.com/content/depth-precision-visualized).
This means that there is a high depth resolution close to the near plane, while objects far from the near-plane are likely to experience [z-fighting](https://en.wikipedia.org/wiki/Z-fighting).

This typically occurs if you try to render large outdoor scenes, where you have objects very close to the camera, like grass-leaves, as well as objects very far away that all have their separate depth.

To mitigate this problem, %Ogre allows you to use a reversed floating-point Z-Buffer, that results in an approximately linear depth value distribution. To use this, enable the `"Reversed Z-Buffer"` RenderSystem option.

@note currently this is only supported by the D3D11 and GL3Plus RenderSystems

This will make %Ogre use the `[1; 0]` range for depth values instead of the standard `[0; 1]` range.

However, we also have to use a floating-point depth buffer to get any benefit from that.
This is a little bit tricky, as e.g. OpenGL is very restrictive on the main depth-buffer therefore unlikely to allow you to using a floating point buffer there.

Therefore, we will use an off-screen texture for rendering, where we can easily use a floating-point depth buffer and only copy the results to the screen.

For this you can use the following [Compositor script](@ref Compositor-Scripts):

```cpp
// a simple material that only applies the texture
material copy
{
    technique
    {
        pass
        {
            lighting off
            texture_unit
            {
                filtering none
            }
        }
    }
}

compositor OffscreenRender
{
    technique
    {
        // this intermediate texture allows OGRE to attach a float depth buffer
        texture result target_width target_height PF_BYTE_RGBA

        target result
        {
            // this will just render the scene as-is
            input previous
        }

        target_output
        {
            // for the output we only have to copy the "result" texture to screen
            pass render_quad
            {
                material copy
                input 0 result
            }
        }
    }
}
```
If reversed depth is enabled, %Ogre will automatically assign a floating point buffer here.

See @ref Applying-a-Compositor, for how to set that compositor on your main window.

@note if you already use some compositor effects, make sure that `OffscreenRender` is the first compositor in the Ogre::CompositorChain.

As we only render a full-screen quad to our main window, we should tell %Ogre that we do not need a depth buffer for it.
We do this as:
```cpp
Ogre::RenderWindow* rwin = getRenderWindow();
rwin->setDepthBufferPool(Ogre::DepthBuffer::POOL_NO_DEPTH);
```

@note If you are reading depth values in your shader, you can test for the `OGRE_REVERSED_Z` define, to discover whether reversed depth is enabled.
