@node Shadows
@chapter Shadows
Shadows are clearly an important part of rendering a believable scene - they provide a more tangible feel to the objects in the scene, and aid the viewer in understanding the spatial relationship between objects. Unfortunately, shadows are also one of the most challenging aspects of 3D rendering, and they are still very much an active area of research. Whilst there are many techniques to render shadows, none is perfect and they all come with advantages and disadvantages. For this reason, Ogre provides multiple shadow implementations, with plenty of configuration settings, so you can choose which technique is most appropriate for your scene.@*@*

Shadow implementations fall into basically 2 broad categories: @ref{Stencil Shadows} and @ref{Texture-based Shadows}. This describes the method by which the shape of the shadow is generated. In addition, there is more than one way to render the shadow into the scene: @ref{Modulative Shadows}, which darkens the scene in areas of shadow, and @ref{Additive Light Masking} which by contrast builds up light contribution in areas which are not in shadow. You also have the option of @ref{Integrated Texture Shadows} which gives you complete control over texture shadow application, allowing for complex single-pass shadowing shaders. Ogre supports all these combinations.@*@*

@subheading Enabling shadows
Shadows are disabled by default, here's how you turn them on and configure them in the general sense:
@enumerate
@item 
Enable a shadow technique on the SceneManager as the @strong{first} thing you doing your scene setup. It is important that this is done first because the shadow technique can alter the way meshes are loaded. Here's an example:
@example 
mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
@end example
@item
Create one or more lights. Note that not all light types are necessarily supported by all shadow techniques, you should check the sections about each technique to check. Note that if certain lights should not cast shadows, you can turn that off by calling setCastShadows(false) on the light, the default is true.
@item
Disable shadow casting on objects which should not cast shadows. Call setCastShadows(false) on objects you don't want to cast shadows, the default for all objects is to cast shadows.
@item 
Configure shadow far distance. You can limit the distance at which shadows are considered for performance reasons, by calling SceneManager::setShadowFarDistance.
@item 
Turn off the receipt of shadows on materials that should not receive them. You can turn off the receipt of shadows (note, not the casting of shadows - that is done per-object) by calling Material::setReceiveShadows or using the receive_shadows material attribute. This is useful for materials which should be considered self-illuminated for example. Note that transparent materials are typically excluded from receiving and casting shadows, although see the @ref{transparency_casts_shadows} option for exceptions. 
@end enumerate

@subheading Opting out of shadows
By default Ogre treats all non-transparent objects as shadow casters and receivers (depending on the shadow technique they may not be able to be both at once, check the docs for your chosen technique first). You can disable shadows in various ways:
@table @asis
@item Turning off shadow casting on the light
Calling Light::setCastsShadows(false) will mean this light casts no shadows at all.
@item Turn off shadow receipt on a material
Calling Material::setReceiveShadows(false) will prevent any objects using this material from receiving shadows.
@item Turn off shadow casting on individual objects
Calling MovableObject::setCastsShadows(false) will disable shadow casting for this object.
@item Turn off shadows on an entire rendering queue group
Calling RenderQueueGroup::setShadowsEnabled(false) will turn off both shadow casting and receiving on an entire rendering queue group. This is useful because Ogre has to do light setup tasks per group in order to preserve the inter-group ordering. Ogre automatically disables shadows on a number of groups automatically, such as RENDER_QUEUE_BACKGROUND, RENDER_QUEUE_OVERLAY, RENDER_QUEUE_SKIES_EARLY and RENDER_QUEUE_SKIES_LATE. If you choose to use more rendering queues (and by default, you won't be using any more than this plus the 'standard' queue, so ignore this if you don't know what it means!), be aware that each one can incur a light setup cost, and you should disable shadows on the additional ones you use if you can.
@end table

@node Stencil Shadows
@section Stencil Shadows

Stencil shadows are a method by which a 'mask' is created for the screen using a feature called the stencil buffer. This mask can be used to exclude areas of the screen from subsequent renders, and thus it can be used to either include or exclude areas in shadow. They are enabled by calling SceneManager::setShadowTechnique with a parameter of either @code{SHADOWTYPE_STENCIL_ADDITIVE} or @code{SHADOWTYPE_STENCIL_MODULATIVE}. Because the stencil can only mask areas to be either 'enabled' or 'disabled', stencil shadows have 'hard' edges, that is to say clear dividing lines between light and shadow - it is not possible to soften these edges.@*@*

In order to generate the stencil, 'shadow volumes' are rendered by extruding the silhouette of the shadow caster away from the light. Where these shadow volumes intersect other objects (or the caster, since self-shadowing is supported using this technique), the stencil is updated, allowing subsequent operations to differentiate between light and shadow. How exactly this is used to render the shadows depends on whether @ref{Modulative Shadows} or @ref{Additive Light Masking} is being used. Objects can both cast and receive stencil shadows, so self-shadowing is inbuilt. @*@*

The advantage of stencil shadows is that they can do self-shadowing simply on low-end hardware, provided you keep your poly count under control. In contrast doing self-shadowing with texture shadows requires a fairly modern machine (@xref{Texture-based Shadows}). For this reason, you're likely to pick stencil shadows if you need an accurate shadowing solution for an application aimed at older or lower-spec machines.@*@* 

The disadvantages of stencil shadows are numerous though, especially on more modern hardware. Because stencil shadows are a geometric technique, they are inherently more costly the higher the number of polygons you use, meaning you are penalized the more detailed you make your meshes. The fillrate cost, which comes from having to render shadow volumes, also escalates the same way. Since more modern applications are likely to use higher polygon counts, stencil shadows can start to become a bottleneck. In addition, the visual aspects of stencil shadows are pretty primitive - your shadows will always be hard-edged, and you have no possibility of doing clever things with shaders since the stencil is not available for manipulation there. Therefore, if your application is aimed at higher-end machines you should definitely consider switching to texture shadows (@xref{Texture-based Shadows}).

There are a number of issues to consider which are specific to stencil shadows:
@itemize @bullet
@item @ref{CPU Overhead}
@item @ref{Extrusion distance}
@item @ref{Camera far plane positioning}
@item @ref{Mesh edge lists}
@item @ref{The Silhouette Edge}
@item @ref{Be realistic}
@item @ref{Stencil Optimisations Performed By Ogre}
@end itemize

@anchor{CPU Overhead}
@subheading CPU Overhead
Calculating the shadow volume for a mesh can be expensive, and it has to be done on the CPU, it is not a hardware accelerated feature. Therefore, you can find that if you overuse this feature, you can create a CPU bottleneck for your application. Ogre quite aggressively eliminates objects which cannot be casting shadows on the frustum, but there are limits to how much it can do, and large, elongated shadows (e.g. representing a very low sun position) are very difficult to cull efficiently. Try to avoid having too many shadow casters around at once, and avoid long shadows if you can. Also, make use of the 'shadow far distance' parameter on the SceneManager, this can eliminate distant shadow casters from the shadow volume construction and save you some time, at the expense of only having shadows for closer objects. Lastly, make use of Ogre's Level-Of-Detail (LOD) features; you can generate automatically calculated LODs for your meshes in code (see the Mesh API docs) or when using the mesh tools such as OgreXmlConverter and OgreMeshUpgrader. Alternatively, you can assign your own manual LODs by providing alternative mesh files at lower detail levels. Both methods will cause the shadow volume complexity to decrease as the object gets further away, which saves you valuable volume calculation time. 

@anchor{Extrusion distance}
@subheading Extrusion distance
When vertex programs are not available, Ogre can only extrude shadow volumes a finite distance from the object. If an object gets too close to a light, any finite extrusion distance will be inadequate to guarantee all objects will be shadowed properly by this object. Therefore, you are advised not to let shadow casters pass too close to light sources if you can avoid it, unless you can guarantee that your target audience will have vertex program capable hardware (in this case, Ogre extrudes the volume to infinity using a vertex program so the problem does not occur).@*@*
When infinite extrusion is not possible, Ogre uses finite extrusion, either derived from the attenuation range of a light (in the case of a point light or spotlight), or a fixed extrusion distance set in the application in the case of directional lights. To change the directional light extrusion distance, use SceneManager::setShadowDirectionalLightExtrusionDistance. 

@anchor{Camera far plane positioning}
@subheading Camera far plane positioning
Stencil shadow volumes rely very much on not being clipped by the far plane. When you enable stencil shadows, Ogre internally changes the far plane settings of your cameras such that there is no far plane - i.e. it is placed at infinity (Camera::setFarClipDistance(0)). This avoids artifacts caused by clipping the dark caps on shadow volumes, at the expense of a (very) small amount of depth precision. 

@anchor{Mesh edge lists}
@subheading Mesh edge lists
Stencil shadows can only be calculated when an 'edge list' has been built for all the geometry in a mesh. The official exporters and tools automatically build this for you (or have an option to do so), but if you create your own meshes, you must remember to build edge lists for them before using them with stencil shadows - you can do that by using OgreMeshUpgrade or OgreXmlConverter, or by calling Mesh::buildEdgeList before you export or use the mesh. If a mesh doesn't have edge lists, OGRE assumes that it is not supposed to cast stencil shadows.

@anchor{The Silhouette Edge}
@subheading The Silhouette Edge
Stencil shadowing is about finding a silhouette of the mesh, and projecting it away to form a volume. What this means is that there is a definite boundary on the shadow caster between light and shadow; a set of edges where where the triangle on one side is facing toward the light, and one is facing away. This produces a sharp edge around the mesh as the transition occurs. Provided there is little or no other light in the scene, and the mesh has smooth normals to produce a gradual light change in its underlying shading, the silhouette edge can be hidden - this works better the higher the tessellation of the mesh. However, if the scene includes ambient light, then the difference is far more marked. This is especially true when using @ref{Modulative Shadows}, because the light contribution of each shadowed area is not taken into account by this simplified approach, and so using 2 or more lights in a scene using modulative stencil shadows is not advisable; the silhouette edges will be very marked. Additive lights do not suffer from this as badly because each light is masked individually, meaning that it is only ambient light which can show up the silhouette edges.

@anchor{Be realistic}
@subheading Be realistic
Don't expect to be able to throw any scene using any hardware at the stencil shadow algorithm and expect to get perfect, optimum speed results. Shadows are a complex and expensive technique, so you should impose some reasonable limitations on your placing of lights and objects; they're not really that restricting, but you should be aware that this is not a complete free-for-all. 
@itemize @bullet 
@item Try to avoid letting objects pass very close (or even through) lights - it might look nice but it's one of the cases where artifacts can occur on machines not capable of running vertex programs.
@item Be aware that shadow volumes do not respect the 'solidity' of the objects they pass through, and if those objects do not themselves cast shadows (which would hide the effect) then the result will be that you can see shadows on the other side of what should be an occluding object. 
@item Make use of SceneManager::setShadowFarDistance to limit the number of shadow volumes constructed
@item Make use of LOD to reduce shadow volume complexity at distance
@item Avoid very long (dusk and dawn) shadows - they exacerbate other issues such as volume clipping, fillrate, and cause many more objects at a greater distance to require volume construction.  
@end itemize
@*@*

@anchor{Stencil Optimisations Performed By Ogre}
@subheading Stencil Optimisations Performed By Ogre
Despite all that, stencil shadows can look very nice (especially with @ref{Additive Light Masking}) and can be fast if you respect the rules above. In addition, Ogre comes pre-packed with a lot of optimisations which help to make this as quick as possible. This section is more for developers or people interested in knowing something about the 'under the hood' behaviour of Ogre.
@table @asis
@item Vertex program extrusion
As previously mentioned, Ogre performs the extrusion of shadow volumes in hardware on vertex program-capable hardware (e.g. GeForce3, Radeon 8500 or better). This has 2 major benefits; the obvious one being speed, but secondly that vertex programs can extrude points to infinity, which the fixed-function pipeline cannot, at least not without performing all calculations in software. This leads to more robust volumes, and also eliminates more than half the volume triangles on directional lights since all points are projected to a single point at infinity.
@item Scissor test optimisation
Ogre uses a scissor rectangle to limit the effect of point / spot lights when their range does not cover the entire viewport; that means we save fillrate when rendering stencil volumes, especially with distant lights
@item Z-Pass and Z-Fail algorithms
The Z-Fail algorithm, often attributed to John Carmack, is used in Ogre to make sure shadows are robust when the camera passes through the shadow volume. However, the Z-Fail algorithm is more expensive than the traditional Z-Pass; so Ogre detects when Z-Fail is required and only uses it then, Z-Pass is used at all other times.
@item 2-Sided stencilling and stencil wrapping
Ogre supports the 2-Sided stencilling / stencil wrapping extensions, which when supported allow volumes to be rendered in a single pass instead of having to do one pass for back facing tris and another for front-facing tris. This doesn't save fillrate, since the same number of stencil updates are done, but it does save primitive setup and the overhead incurred in the driver every time a render call is made.
@item Aggressive shadow volume culling
Ogre is pretty good at detecting which lights could be affecting the frustum, and from that, which objects could be casting a shadow on the frustum. This means we don't waste time constructing shadow geometry we don't need. Setting the shadow far distance is another important way you can reduce stencil shadow overhead since it culls far away shadow volumes even if they are visible, which is beneficial in practice since you're most interested in shadows for close-up objects.
@end table


@node Texture-based Shadows
@section Texture-based Shadows

Texture shadows involve rendering shadow casters from the point of view of the light into a texture, which is then projected onto shadow receivers. The main advantage of texture shadows as opposed to @ref{Stencil Shadows} is that the overhead of increasing the geometric detail is far lower, since there is no need to perform per-triangle calculations. Most of the work in rendering texture shadows is done by the graphics card, meaning the technique scales well when taking advantage of the latest cards, which are at present outpacing CPUs in terms of their speed of development. In addition, texture shadows are @strong{much} more customisable - you can pull them into shaders to apply as you like (particularly with @ref{Integrated Texture Shadows}, you can perform filtering to create softer shadows or perform other special effects on them. Basically, most modern engines use texture shadows as their primary shadow technique simply because they are more powerful, and the increasing speed of GPUs is rapidly amortizing the fillrate / texture access costs of using them.@*@*

The main disadvantage to texture shadows is that, because they are simply a texture, they have a fixed resolution which means if stretched, the pixellation of the texture can become obvious. There are ways to combat this though:

@table @asis
@item Choosing a projection basis
The simplest projection is just to render the shadow casters from the lights perspective using a regular camera setup. This can look bad though, so there are many other projections which can help to improve the quality from the main camera's perspective. OGRE supports pluggable projection bases via it's ShadowCameraSetup class, and comes with several existing options - @strong{Uniform} (which is the simplest), @strong{Uniform Focussed} (which is still a normal camera projection, except that the camera is focussed into the area that the main viewing camera is looking at), LiSPSM (Light Space Perspective Shadow Mapping - which both focusses and distorts the shadow frustum based on the main view camera) and Plan Optimal (which seeks to optimise the shadow fidelity for a single receiver plane).
@item Filtering
You can also sample the shadow texture multiple times rather than once to soften the shadow edges and improve the appearance. Percentage Closest Filtering (PCF) is the most popular approach, although there are multiple variants depending on the number and pattern of the samples you take. Our shadows demo includes a 5-tap PCF example combined with depth shadow mapping.
@item Using a larger texture
Again as GPUs get faster and gain more memory, you can scale up to take advantage of this. 
@end table
If you combine all 3 of these techniques you can get a very high quality shadow solution.

The other issue is with point lights. Because texture shadows require a render to texture in the direction of the light, omnidirectional lights (point lights) would require 6 renders to totally cover all the directions shadows might be cast. For this reason, Ogre primarily supports directional lights and spotlights for generating texture shadows; you can use point lights but they will only work if off-camera since they are essentially turned into a spotlight shining into your camera frustum for the purposes of texture shadows.@*@*

@subheading Directional Lights
Directional lights in theory shadow the entire scene from an infinitely distant light. Now, since we only have a finite texture which will look very poor quality if stretched over the entire scene, clearly a simplification is required. Ogre places a shadow texture over the area immediately in front of the camera, and moves it as the camera moves (although it rounds this movement to multiples of texels so that the slight 'swimming shadow' effect caused by moving the texture is minimised). The range to which this shadow extends, and the offset used to move it in front of the camera, are configurable (@xref{Configuring Texture Shadows}). At the far edge of the shadow, Ogre fades out the shadow based on other configurable parameters so that the termination of the shadow is softened.

@subheading Spotlights
Spotlights are much easier to represent as renderable shadow textures than directional lights, since they are naturally a frustum. Ogre represents spotlight directly by rendering the shadow from the light position, in the direction of the light cone; the field-of-view of the texture camera is adjusted based on the spotlight falloff angles. In addition, to hide the fact that the shadow texture is square and has definite edges which could show up outside the spotlight, Ogre uses a second texture unit when projecting the shadow onto the scene which fades out the shadow gradually in a projected circle around the spotlight.

@subheading Point Lights
As mentioned above, to support point lights properly would require multiple renders (either 6 for a cubic render or perhaps 2 for a less precise parabolic mapping), so rather than do that we approximate point lights as spotlights, where the configuration is changed on the fly to make the light shine from its position over the whole of the viewing frustum. This is not an ideal setup since it means it can only really work if the point light's position is out of view, and in addition the changing parameterisation can cause some 'swimming' of the texture. Generally we recommend avoiding making point lights cast texture shadows.

@subheading Shadow Casters and Shadow Receivers
To enable texture shadows, use the shadow technique SHADOWTYPE_TEXTURE_MODULATIVE or SHADOWTYPE_TEXTURE_ADDITIVE; as the name suggests this produces @ref{Modulative Shadows} or @ref{Additive Light Masking} respectively. The cheapest and simplest texture shadow techniques do not use depth information, they merely render casters to a texture and render this onto receivers as plain colour - this means self-shadowing is not possible using these methods. This is the default behaviour if you use the automatic, fixed-function compatible (and thus usable on lower end hardware) texture shadow techniques. You can however use shaders-based techniques through custom shadow materials for casters and receivers to perform more complex shadow algorithms, such as depth shadow mapping which does allow self-shadowing. OGRE comes with an example of this in its shadows demo, although it's only usable on Shader Model 2 cards or better. Whilst fixed-function depth shadow mapping is available in OpenGL, it was never standardised in Direct3D so using shaders in custom caster & receiver materials is the only portable way to do it. If you use this approach, call SceneManager::setShadowTextureSelfShadow with a parameter of 'true' to allow texture shadow casters to also be receivers. @*@*
If you're not using depth shadow mapping, OGRE divides shadow casters and receivers into 2 disjoint groups. Simply by turning off shadow casting on an object, you automatically make it a shadow receiver (although this can be disabled by setting the 'receive_shadows' option to 'false' in a material script. Similarly, if an object is set as a shadow caster, it cannot receive shadows. 
@anchor{Configuring Texture Shadows}
@heading Configuring Texture Shadows 
There are a number of settings which will help you configure your texture-based shadows so that they match your requirements.

@itemize bullet
@item @ref{Maximum number of shadow textures}
@item @ref{Shadow texture size}
@item @ref{Shadow far distance}
@item @ref{Shadow texture offset (Directional Lights)}
@item @ref{Shadow fade settings}
@item @ref{Custom shadow camera setups}
@item @ref{Integrated Texture Shadows}
@end itemize

@anchor{Maximum number of shadow textures}
@subheading Maximum number of shadow textures
Shadow textures take up texture memory, and to avoid stalling the rendering pipeline Ogre does not reuse the same shadow texture for multiple lights within the same frame. This means that each light which is to cast shadows must have its own shadow texture. In practice, if you have a lot of lights in your scene you would not wish to incur that sort of texture overhead.@*@*
You can adjust this manually by simply turning off shadow casting for lights you do not wish to cast shadows. In addition, you can set a maximum limit on the number of shadow textures Ogre is allowed to use by calling SceneManager::setShadowTextureCount. Each frame, Ogre determines the lights which could be affecting the frustum, and then allocates the number of shadow textures it is allowed to use to the lights on a first-come-first-served basis. Any additional lights will not cast shadows that frame.
@*@*Note that you can set the number of shadow textures and their size at the same time by using the SceneManager::setShadowTextureSettings method; this is useful because both the individual calls require the potential creation / destruction of texture resources.


@anchor{Shadow texture size}
@subheading Shadow texture size

The size of the textures used for rendering the shadow casters into can be altered; clearly using larger textures will give you better quality shadows, but at the expense of greater memory usage. Changing the texture size is done by calling SceneManager::setShadowTextureSize - textures are assumed to be square and you must specify a texture size that is a power of 2. Be aware that each modulative shadow texture will take size*size*3 bytes of texture memory. @*@*
@strong{Important}: if you use the GL render system your shadow texture size can only be larger (in either dimension) than the size of your primary window surface if the hardware supports the Frame Buffer Object (FBO) or Pixel Buffer Object (PBO) extensions. Most modern cards support this now, but be careful of older cards - you can check the ability of the hardware to manage this through ogreRoot->getRenderSystem()->getCapabilities()->hasCapability(RSC_HWRENDER_TO_TEXTURE). If this returns false, if you create a shadow texture larger in any dimension than the primary surface, the rest of the shadow texture will be blank.


@anchor{Shadow far distance}
@subheading Shadow far distance

This determines the distance at which shadows are terminated; it also determines how far into the distance the texture shadows for directional lights are stretched - by reducing this value, or increasing the texture size, you can improve the quality of shadows from directional lights at the expense of closer shadow termination or increased memory usage, respectively.

@anchor{Shadow texture offset (Directional Lights)}
@subheading Shadow texture offset (Directional Lights)
As mentioned above in the directional lights section, the rendering of shadows for directional lights is an approximation that allows us to use a single render to cover a largish area with shadows. This offset parameter affects how far from the camera position the center of the shadow texture is offset, as a proportion of the shadow far distance. The greater this value, the more of the shadow texture is 'useful' to you since it's ahead of the camera, but also the further you offset it, the more chance there is of accidentally seeing the edge of the shadow texture at more extreme angles. You change this value by calling SceneManager::setShadowDirLightTextureOffset, the default is 0.6.

@anchor{Shadow fade settings}
@subheading Shadow fade settings

Shadows fade out before the shadow far distance so that the termination of shadow is not abrupt. You can configure the start and end points of this fade by calling the SceneManager::setShadowTextureFadeStart and SceneManager::setShadowTextureFadeEnd methods, both take distances as a proportion of the shadow far distance. Because of the inaccuracies caused by using a square texture and a radial fade distance, you cannot use 1.0 as the fade end, if you do you'll see artifacts at the extreme edges. The default values are 0.7 and 0.9, which serve most purposes but you can change them if you like.

@heading Texture shadows and vertex / fragment programs
When rendering shadow casters into a modulative shadow texture, Ogre turns off all textures, and all lighting contributions except for ambient light, which it sets to the colour of the shadow (@ref{Shadow Colour}). For additive shadows, it render the casters into a black & white texture instead. This is enough to render shadow casters for fixed-function material techniques, however where a vertex program is used Ogre doesn't have so much control. If you use a vertex program in the @strong{first pass} of your technique, then you must also tell ogre which vertex program you want it to use when rendering the shadow caster; see @ref{Shadows and Vertex Programs} for full details.


@anchor{Custom shadow camera setups}
@subheading Custom shadow camera setups

As previously mentioned, one of the downsides of texture shadows is that the texture resolution is finite, and it's possible to get aliasing when the size of the shadow texel is larger than a screen pixel, due to the projection of the texture. In order to address this, you can specify alternative projection bases by using or creating subclasses of the ShadowCameraSetup class. The default version is called DefaultShadowCameraSetup and this sets up a simple regular frustum for point and spotlights, and an orthographic frustum for directional lights. There is also a PlaneOptimalShadowCameraSetup class which specialises the projection to a plane, thus giving you much better definition provided your shadow receivers exist mostly in a single plane. Other setup classes (e.g. you might create a perspective or trapezoid shadow mapping version) can be created and plugged in at runtime, either on individual lights or on the SceneManager as a whole.

@anchor{Integrated Texture Shadows}
@subheading Integrated Texture Shadows

Texture shadows have one major advantage over stencil shadows - the data used to represent them can be referenced in regular shaders. Whilst the default texture shadow modes (SHADOWTYPE_TEXTURE_MODULATIVE and SHADOWTYPE_TEXTURE_ADDITIVE) automatically render shadows for you, their disadvantage is that because they are generalised add-ons to your own materials, they tend to take more passes of the scene to use. In addition, you don't have a lot of control over the composition of the shadows.@*@*

Here is where 'integrated' texture shadows step in. Both of the texture shadow types above have alternative versions called SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED and SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED, where instead of rendering the shadows for you, it just creates the texture shadow and then expects you to use that shadow texture as you see fit when rendering receiver objects in the scene. The downside is that you have to take into account shadow receipt in every one of your materials if you use this option - the upside is that you have total control over how the shadow textures are used. The big advantage here is that you can can perform more complex shading, taking into account shadowing, than is possible using the generalised bolt-on approaches, AND you can probably write them in a smaller number of passes, since you know precisely what you need and can combine passes where possible. When you use one of these shadowing approaches, the only difference between additive and modulative is the colour of the casters in the shadow texture (the shadow colour for modulative, black for additive) - the actual calculation of how the texture affects the receivers is of course up to you. No separate modulative pass will be performed, and no splitting of your materials into ambient / per-light / decal etc will occur - absolutely everything is determined by your original material (which may have modulative passes or per-light iteration if you want of course, but it's not required).@*@*

You reference a shadow texture in a material which implements this approach by using the '@ref{content_type} shadow' directive in your @{texture_unit}. It implicitly references a shadow texture based on the number of times you've used this directive in the same pass, and the light_start option or light-based pass iteration, which might start the light index higher than 0.

@node Modulative Shadows
@section Modulative Shadows
Modulative shadows work by darkening an already rendered scene with a fixed colour. First, the scene is rendered normally containing all the objects which will be shadowed, then a modulative pass is done per light, which darkens areas in shadow. Finally, objects which do not receive shadows are rendered.@*@*

There are 2 modulative shadow techniques; stencil-based (@xref{Stencil Shadows} : SHADOWTYPE_STENCIL_MODULATIVE) and texture-based (@xref{Texture-based Shadows} : SHADOWTYPE_TEXTURE_MODULATIVE). Modulative shadows are an inaccurate lighting model, since they darken the areas of shadow uniformly, irrespective of the amount of light which would have fallen on the shadow area anyway. However, they can give fairly attractive results for a much lower overhead than more 'correct' methods like @ref{Additive Light Masking}, and they also combine well with pre-baked static lighting (such as pre-calculated lightmaps), which additive lighting does not. The main thing to consider is that using multiple light sources can result in overly dark shadows (where shadows overlap, which intuitively looks right in fact, but it's not physically correct) and artifacts when using stencil shadows (@xref{The Silhouette Edge}). @*@*

@anchor{Shadow Colour}
@subheading Shadow Colour
The colour which is used to darken the areas in shadow is set by SceneManager::setShadowColour; it defaults to a dark grey (so that the underlying colour still shows through a bit).@*@*

Note that if you're using texture shadows you have the additional option of using @ref{Integrated Texture Shadows} rather than being forced to have a separate pass of the scene to render shadows. In this case the 'modulative' aspect of the shadow technique just affects the colour of the shadow texture. @*@*

@node Additive Light Masking
@section Additive Light Masking
Additive light masking is about rendering the scene many times, each time representing a single light contribution whose influence is masked out in areas of shadow. Each pass is combined with (added to) the previous one such that when all the passes are complete, all the light contribution has correctly accumulated in the scene, and each light has been prevented from affecting areas which it should not be able to because of shadow casters. This is an effective technique which results in very realistic looking lighting, but it comes at a price: more rendering passes.@*@*

As many technical papers (and game marketing) will tell you, rendering realistic lighting like this requires multiple passes. Being a friendly sort of engine, Ogre frees you from most of the hard work though, and will let you use the exact same material definitions whether you use this lighting technique or not (for the most part, see @ref{Pass Classification and Vertex Programs}). In order to do this technique, Ogre automatically categorises the @ref{Passes} you define in your materials into 3 types:
@enumerate
@item ambient 
Passes categorised as 'ambient' include any base pass which is not lit by any particular light, i.e. it occurs even if there is no ambient light in the scene. The ambient pass always happens first, and sets up the initial depth value of the fragments, and the ambient colour if applicable. It also includes any emissive / self illumination contribution. Only textures which affect ambient light (e.g. ambient occlusion maps) should be rendered in this pass.
@item diffuse/specular
Passes categorised as 'diffuse/specular' (or 'per-light') are rendered once per light, and each pass contributes the diffuse and specular colour from that single light as reflected by the diffuse / specular terms in the pass. Areas in shadow from that light are masked and are thus not updated. The resulting masked colour is added to the existing colour in the scene. Again, no textures are used in this pass (except for textures used for lighting calculations such as normal maps).
@item decal
Passes categorised as 'decal' add the final texture colour to the scene, which is modulated by the accumulated light built up from all the ambient and diffuse/specular passes. 
@end enumerate

In practice, @ref{Passes} rarely fall nicely into just one of these categories. For each Technique, Ogre compiles a list of 'Illumination Passes', which are derived from the user defined passes, but can be split, to ensure that the divisions between illumination pass categories can be maintained. For example, if we take a very simple material definition:
@example
material TestIllumination
{
	technique
	{
		pass
		{
			ambient 0.5 0.2 0.2 
			diffuse 1 0 0  
			specular 1 0.8 0.8 15
			texture_unit
			{
				texture grass.png
			}
		}
	}
}
@end example

Ogre will split this into 3 illumination passes, which will be the equivalent of this:
@example
material TestIlluminationSplitIllumination
{
	technique
	{
		// Ambient pass
		pass
		{
			ambient 0.5 0.2 0.2 
			diffuse 0 0 0
			specular 0 0 0
		}

		// Diffuse / specular pass
		pass
		{
			scene_blend add
			iteration once_per_light
			diffuse 1 0 0  
			specular 1 0.8 0.8 15
		}

		// Decal pass
		pass
		{
			scene_blend modulate
			lighting off
			texture_unit
			{
				texture grass.png
			}
		}
	}
}
@end example
So as you can see, even a simple material requires a minimum of 3 passes when using this shadow technique, and in fact it requires (num_lights + 2) passes in the general sense. You can use more passes in your original material and Ogre will cope with that too, but be aware that each pass may turn into multiple ones if it uses more than one type of light contribution (ambient vs diffuse/specular) and / or has texture units. The main nice thing is that you get the full multipass lighting behaviour even if you don't define your materials in terms of it, meaning that your material definitions can remain the same no matter what lighting approach you decide to use.@*@*

@anchor{Manually Categorising Illumination Passes}
@subheading Manually Categorising Illumination Passes
Alternatively, if you want more direct control over the categorisation of your passes, you can use the @ref{illumination_stage} option in your pass to explicitly assign a pass unchanged to an illumination stage. This way you can make sure you know precisely how your material will be rendered under additive lighting conditions.

@anchor{Pass Classification and Vertex Programs}
@subheading Pass Classification and Vertex Programs
Ogre is pretty good at classifying and splitting your passes to ensure that the multipass rendering approach required by additive lighting works correctly without you having to change your material definitions. However, there is one exception; when you use vertex programs, the normal lighting attributes ambient, diffuse, specular etc are not used, because all of that is determined by the vertex program. Ogre has no way of knowing what you're doing inside that vertex program, so you have to tell it.@*@*

In practice this is very easy. Even though your vertex program could be doing a lot of complex, highly customised processing, it can still be classified into one of the 3 types listed above. All you need to do to tell Ogre what you're doing is to use the pass attributes ambient, diffuse, specular and self_illumination, just as if you were not using a vertex program. Sure, these attributes do nothing (as far as rendering is concerned) when you're using vertex programs, but it's the easiest way to indicate to Ogre which light components you're using in your vertex program. Ogre will then classify and potentially split your programmable pass based on this information - it will leave the vertex program as-is (so that any split passes will respect any vertex modification that is being done). @*@*

Note that when classifying a diffuse/specular programmable pass, Ogre checks to see whether you have indicated the pass can be run once per light (iteration once_per_light). If so, the pass is left intact, including it's vertex and fragment programs. However, if this attribute is not included in the pass, Ogre tries to split off the per-light part, and in doing so it will disable the fragment program, since in the absence of the 'iteration once_per_light' attribute it can only assume that the fragment program is performing decal work and hence must not be used per light.@*@*

So clearly, when you use additive light masking as a shadow technique, you need to make sure that programmable passes you use are properly set up so that they can be classified correctly. However, also note that the changes you have to make to ensure the classification is correct does not affect the way the material renders when you choose not to use additive lighting, so the principle that you should be able to use the same material definitions for all lighting scenarios still holds. Here is an example of a programmable material which will be classified correctly by the illumination pass classifier:
@example
// Per-pixel normal mapping Any number of lights, diffuse and specular
material Examples/BumpMapping/MultiLightSpecular
{

	technique
	{
		// Base ambient pass
		pass
		{
			// ambient only, not needed for rendering, but as information
			// to lighting pass categorisation routine
			ambient 1 1 1
			diffuse 0 0 0 
			specular 0 0 0 0
			// Really basic vertex program
			vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
			{
				param_named_auto worldViewProj worldviewproj_matrix
				param_named_auto ambient ambient_light_colour
			}
			
		}
		// Now do the lighting pass
		// NB we don't do decal texture here because this is repeated per light
		pass
		{
			// set ambient off, not needed for rendering, but as information
			// to lighting pass categorisation routine
			ambient 0 0 0 
			// do this for each light
			iteration once_per_light

		
			scene_blend add

			// Vertex program reference
			vertex_program_ref Examples/BumpMapVPSpecular
			{
				param_named_auto lightPosition light_position_object_space 0
				param_named_auto eyePosition camera_position_object_space
				param_named_auto worldViewProj worldviewproj_matrix
			}

			// Fragment program
			fragment_program_ref Examples/BumpMapFPSpecular
			{
				param_named_auto lightDiffuse light_diffuse_colour 0 
				param_named_auto lightSpecular light_specular_colour 0
			}
			
			// Base bump map
			texture_unit
			{
				texture NMBumpsOut.png
				colour_op replace
			}
			// Normalisation cube map
			texture_unit
			{
				cubic_texture nm.png combinedUVW
				tex_coord_set 1
				tex_address_mode clamp
			}
			// Normalisation cube map #2
			texture_unit
			{
				cubic_texture nm.png combinedUVW
				tex_coord_set 1
				tex_address_mode clamp
			}
		}
		
		// Decal pass
		pass
		{
			lighting off
			// Really basic vertex program
			vertex_program_ref Ogre/BasicVertexPrograms/AmbientOneTexture
			{
				param_named_auto worldViewProj worldviewproj_matrix
				param_named ambient float4 1 1 1 1
			}
			scene_blend dest_colour zero
			texture_unit
			{
				texture RustedMetal.jpg 
			}
			
		}
	}
}
@end example

Note that if you're using texture shadows you have the additional option of using @ref{Integrated Texture Shadows} rather than being forced to use this explicit sequence - allowing you to compress the number of passes into a much smaller number at the expense of defining an upper number of shadow casting lights. In this case the 'additive' aspect of the shadow technique just affects the colour of the shadow texture and it's up to you to combine the shadow textures in your receivers however you like. @*@*

@subheading Static Lighting
Despite their power, additive lighting techniques have an additional limitation; they do not combine well with pre-calculated static lighting in the scene. This is because they are based on the principle that shadow is an absence of light, but since static lighting in the scene already includes areas of light and shadow, additive lighting cannot remove light to create new shadows. Therefore, if you use the additive lighting technique you must either use it exclusively as your lighting solution (and you can combine it with per-pixel lighting to create a very impressive dynamic lighting solution), or you must use @ref{Integrated Texture Shadows} to combine the static lighting according to your chosen approach. 


