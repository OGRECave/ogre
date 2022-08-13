# Instancing User-Guide {#WhatIsInstancing}

@tableofcontents

A common question is why should I use instancing.
The big reason is performance.
There can be 10x improvements or more when used correctly.
Here's a guide on when you should use instancing:

1.  You have *a lot* of Entities that are repeated and based on the same Mesh (i.e. a rock, a building, a tree, loose leaves, enemies, irrelevant crowds or NPCs)
2.  These Entities that repeat a lot also share the same material (or just a few materials, i.e. 3 or 4)
3.  The bottleneck in your game is the CPU.

If these three requirements are all met in your game, chances are instancing is for you.
There will be minimal gains when using instancing on an Entity that repeats very little, or if each instance actually has a different material, or it could run even slower if the Entity never repeats.

If the bottleneck in your game is not CPU (i.e. it's in the GPU) then instancing won't make a noticeable difference.

## Instances per batch {#InstancesPerBatch}

As explained in the previous section, instancing groups all instances into one draw call.
However this is half the truth.
Instancing actually groups a certain number of instances into a batch.
One batch = One draw call.

If the technique is using 80 instances per batch; then rendering 160 instances is going to need 2 draw calls (two batches);
if there are 180 instances, 3 draw calls will be needed (3 batches).

What is a good value for instances-per-batch setting?
That depends on a lot of factors, which you will have to profile.
Normally, increasing the number should improve performance because the system is most likely CPU bottleneck.
However, past certain number, certain trade offs begin to show up:

-   Culling is first performed at batch level, then for HW techniques culling is also done at per instance level.
    If the batch contains too many instances, its AABB will grow too large; thus the hierarchy culling will always pass and Ogre won't be able skip entire batches.
-   If the instance per batch is at 10.000 and the application created 10.001 instances;
    a lot of RAM & VRAM will be wasted because it's set for 20.000 instances;
	HW techniques will spent an excessive amount of CPU time parsing the 9.999 inactive instances;
	and SW techniques will saturate the Bus bandwidth sending null matrices for the inactive instances to the GPU.

The actual value will depend a lot on the application and whether all instances are often on screen or frustum culled and whether the total number of instances can be known at production time (i.e. environment props).
Normally numbers between 80 and 500 work best, but there have been cases where big values like 5.000 actually improved performance.

# Techniques {#InstancingTechniques}

Ogre supports 4 different instancing techniques. Unfortunately, each of them requires a different vertex shader, since their approaches are different.
Also their compatibility and performance varies.

## ShaderBased {#InstancingTechniquesShaderBased}

@copybrief Ogre::InstanceBatchShader
@copydetails Ogre::InstanceBatchShader

Vertex Shader input example:
```cpp
	...
	float4 blendIdx : BLENDINDICES,
	uniform float3x4 worldMatrix3x4Array[80],
	...
```

Vertex position calculation example:
```cpp
	int idx = int(blendIdx[0]);
	float4 worldPos  = float4( mul( worldMatrix3x4Array[idx], position ).xyz, 1.0f );
	oClipPos = mul( viewProjMatrix, worldPos );
	...
```

See `Examples/Instancing/RTSS/Robot` for an example on how to use instancing with the [RTSS: Run Time Shader System](@ref rtss)

See `material Examples/Instancing/ShaderBased` for an example on how to write the vertex shader. (You can find these files in OGRE sources from before v1.12)
Files:
 - ShaderInstancing.material
 - ShaderInstancing.vert (GLSL)
 - ShaderInstancing.cg (Cg, works with HLSL)

## VTF (Software) {#InstancingTechniquesVTFSoftware}

@copybrief Ogre::BaseInstanceBatchVTF
@copydetails Ogre::BaseInstanceBatchVTF

Texture unit example:
```
texture_unit InstancingVTF
{
	filtering none
}
```

Vertex Shader input example:
```
	...
	float4 m01 : TEXCOORD1,
	float4 m23 : TEXCOORD2,
	uniform sampler2D matrixTexture : register(s0),	// s0 if texture_unit InstancingVTF appears first
	...
```

Vertex position calculation example:

@snippet Samples/Media/materials/programs/HLSL_Cg/VTFInstancing.cg world_pos

See `material Examples/Instancing/VTF` for an example on how to write the vertex shader and setup the material.
Files:
 - VTFInstancing.material
 - VTFInstancing.vert (GLSL)
 - VTFInstancing.cg (Cg, also works with HLSL)

## HW VTF {#InstancingTechniquesHWVTF}

This is the same technique as VTF; but implemented through hardware instancing.
It is probably one of the best and most flexible techniques.

The vertex shader has to be slightly different from SW VTF version.

Texture unit example:
```
texture_unit InstancingVTF
{
	filtering none
}
```

Vertex Shader input example:
```
	float4 m03 : TEXCOORD1,
	float2 mOffset : TEXCOORD2,
	uniform sampler2D matrixTexture : register(s1),
```

Vertex position calculation example:

@snippet Samples/Media/materials/programs/HLSL_Cg/HW_VTFInstancing.cg world_pos

See `material Examples/Instancing/HW_VTF` for an example on how to write the vertex shader and setup the material.
Files:
 - HW\_VTFInstancing.material
 - HW\_VTFInstancing.vert (GLSL)
 - HW\_VTFInstancing.cg (Cg, works with HLSL)

### HW VTF LUT {#InstancingTechniquesHW}

LUT is a special feature of HW VTF; which stands for <b>L</b>ook <b>U</b>p <b>T</b>able.
It has been particularly designed for drawing large animated crowds.

The technique is a trick that works by animating a limited number of instances (i.e. 16 animations) storing them in a look up table in the VTF, and then repeating these animations to all instances uniformly, giving the appearance that all instances are independently animated when seen in large crowds.

To enable the use of LUT, `SceneManager::createInstanceManager`'s flags must include the flag `IM_VTFBONEMATRIXLOOKUP` and specify HW VTF as technique.

```cpp
mSceneMgr->createInstanceManager("InstanceMgr","MyMesh.mesh",
			ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
			InstanceManager::HWInstancingVTF,
			numInstancesPerBatch,IM_USEALL|IM_VTFBONEMATRIXLOOKUP );
```

See `material Examples/Instancing/HW_VTF_LUT`.
Files:
 - Same as HW VTF (different macros defined)

## HW Basic {#InstancingTechniquesHWBasic}

@copybrief Ogre::InstanceBatchHW
@copydetails Ogre::InstanceBatchHW

Vertex Shader input example:
```cpp
	...
	float3x4 worldMatrix : TEXCOORD1,
	...
```

Vertex position calculation example:
```cpp
	...
	float4 worldPos = float4( mul( worldMatrix, position ).xyz, 1.0f );
	oClipPos = mul( viewProjMatrix, worldPos );
	...
```
See `Examples/Instancing/RTSS/Robot` for an example on how to use instancing with the [RTSS: Run Time Shader System](@ref rtss)

See `material Examples/Instancing/HWBasic` for an example. (You can find these files in OGRE sources from before v1.12)
Files:
 - HWInstancing.material
 - HWBasicInstancing.vert (GLSL)
 - HWBasicInstancing.cg (Cg, works with HLSL)

# Custom parameters {#InstancingCustomParameters}

Some instancing techniques allow passing custom parameters to vertex shaders.
For example a custom colour in an RTS game to identify player units; a single value for randomly colouring vegetation, light parameters for rendering deferred shading's light volumes (diffuse colour, specular colour, etc)

At the time of writing only HW Basic supports passing the custom parameters.
All other techniques will ignore it.[^8]

To use custom parameters, call `InstanceManager::setNumCustomParams` to tell the number of custom parameters the user will need.
**This number cannot be changed after creating the first batch** (call createInstancedEntity)

Afterwards, it's just a matter of calling `InstancedEntity::setCustomParam` with the param you wish to send.

For HW Basic techniques, the vertex shader will receive the custom param in an extra `TEXCOORD.`

```cpp
 InstanceManager *instanceMgr;//Assumed to be valid ptr
instanceMgr->setNumCustomParams(2);

InstancedEntity *instancedEntity =instanceMgr->createInstancedEntity("myMaterial");
instancedEntity->setCustomParam(0,Vector4(1.0f,1.0f,1.2f,0.0f));
instancedEntity->setCustomParam(1,Vector4(0.2f,0.0f,0.7f,1.0f));
```

# Supporting multiple submeshes {#InstancingMultipleSubmeshes}

Multiple submeshes means different instance managers, because instancing can only be applied to the same submesh.

Nevertheless, it is actually quite easy to support multiple submeshes.
The first step is to create the InstanceManager setting the `subMeshIdx` parameter to the number of submesh you want to use:

```cpp
std::vector<InstanceManager*>instanceManagers;
MeshPtr mesh =MeshManager::getSingleton().load("myMesh.mesh");
for(uint16 i=0;i<mesh->getNumSubMeshes();++i )
{
	InstanceManager *mgr =
		mSceneMgr->createInstanceManager("MyManager"+StringConverter::toString(i ),
					"myMesh.mesh",
				ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
					InstanceManager::HWInstancingVTF,numInstancePerBatch,
					flags,i );
	instanceManagers.push_back(mgr );
}
```

The second step lies in sharing the transform with one of the submeshes (which will be named 'master'; i.e. the first submesh) to improve performance and reduce RAM consumption when creating the Instanced Entities:

```cpp
SceneNode *sceneNode; //Assumed to be valid ptr
std::vector<InstancedEntity*>instancedEntities;
for(size_t i=0;i<instanceManagers.size();++i )
{
	InstancedEntity *ent =instanceManagers[i]->createInstancedEntity("MyMaterial");

	if(i !=0)
		instancedEntities[0]->shareTransformWith(ent );

	sceneNode->attachObject(ent );
	instancedEntities.push_back(ent );
}
```

Note that it is perfectly possible that each `InstancedEntity` based on a different "submesh" uses a different material.
Selecting the same material won't cause the InstanceManagers to get batched together (though the RenderQueue will try to reduce state change reduction, like with any normal Entity).

Because the transform is shared, animating the master InstancedEntity (in this example, `instancedEntity[0]`) will cause all other slave instances to follow the same animation.

To destroy the instanced entities, use the normal procedure:

```cpp
SceneNode *sceneNode; //Assumed to be valid ptr
std::vector<InstancedEntity*>instancedEntities;
for(size_t i=0;i<instanceManagers.size();++i )
{
	instanceManagers[i]->destroyInstancedEntity(instancedEntities[i]);
}
mSceneMgr->getRootSceneNode()->removeAndDestroyChild(sceneNode );
```

# Defragmenting batches {#InstancingDefragmentingBatches}

## What is batch fragmentation? {#InstancingDefragmentingBatchesIntro}

There are two kinds of fragmentation:

1.  "Deletion" Fragmentation is when many instances have been created, spanning multiple batches; *and many of them got later removed* but they were all from different batches.
	If there were 10 instances per batch, 100 instances created, then later 90 removed; it is possible that now there are 10 batches with one instance each (which equals 10 drawcalls); instead of being just 1 batch with 10 instances (which equals 1 drawcall).
2.  "Culling" Fragmentation is also when many instances of different batches are all sparsed across the whole scene.
    If they were defragmented, they would've been put together in the same batch (all instances sorted by proximity to each other should be in the same batch) to take advantage of hierarchy culling optimizations.

Defragmented batches can dramatically improve performance:

Suppose there 50 instances per batch, and 100 batches total (which means 5000 instanced entities of the same mesh with same material), and they're all moving all the time.

Normally, Ogre first updates all instances' position, then their AABBs;
and while at it, computes the AABB for each batch that encloses all of its instances.

When frustum culling, we first cull the batches, then we cull their instances[^9] (that are inside those culled batches).
**This is the typical hierachial culling optimization**.
We then upload the instances transforms to the GPU.

After moving many instances around the whole world, they will make the batch' enclosing AABB bigger and bigger.
Eventually, every batch' AABB will be so large, that wherever the camera looks, all 100 batches will end up passing the frustum culling test; thus having to resort to cull all 5000 instances individually.

## Prevention: Avoiding fragmentation {#InstancingDefragmentingBatchesPrevention}

If you're creating static objects that won't move (i.e. trees), create them sorted by proximity.
This helps both types of fragmentation:

1.  When unloading areas (i.e. open world games), these objects will be removed all together, thus whole batches will no longer have active instances.
2.  Batches and instances are often assigned by order of creation.
    Those instances will belong to the same batch and thus maximizing culling efficiency.

## Cure: Defragmenting on the fly {#InstancingDefragmentingBatchesOnTheFly}

There are cases where preventing fragmentation, for example units in an RTS game.
By design, all units may end up scattering and moving from one extreme of the scene to the other after hours of gameplay; additionally, lots of units may be in an endless loop of creation and destroying, but if the loop for a certain type of unit is broken; it is possible to end up with the kind of "Deletion" Fragmentation too.

For this reason, the function `InstanceManager::defragmentBatches( bool optimizeCulling )` exists.

Using it as simple as calling the function.
**The sample NewInstancing shows how to do this interactively**.
When `optimizeCulling` is true, both types of fragmentation will be attempted to be fixed.
When false, only the "deletion" kind of fragmentation will be fixed.

Take in mind that when `optimizeCulling = true` it takes significantly more time depending on the level of fragmentation and could cause framerate spikes, even stalls.
Do it sparingly and profile the optimal frequency of calling.

# Troubleshooting {#InstancingTroubleshooting}

@par Q: My mesh doesn't show up.
A: Verify you're using the right material, the vertex shader is set correctly, and it matches the instancing technique being used.

@par Q: My animation plays quite differently than when it is an Entity, or previewed in [Ogre Mesh Viewer](https://github.com/OGRECave/ogre-meshviewer).
A: Your rig animation must be using more than one weight per bone.
You need to add support for it in the vertex shader, and make sure you didn't create the instance manager with the flags Ogre::IM_USEONEWEIGHT or Ogre::IM_FORCEONEWEIGHT.

For example, to modify the HW VTF vertex shader, you need to sample the additional matrices from the VTF:

```cpp
float2uv0		:	TEXCOORD0;
// Up to four weights per vertex. Don't use this shader on a model with 3 weights per vertex, or 2 or 1
float4m03_0	:	TEXCOORD1;//m03.w is always 0
float4m03_1	:	TEXCOORD2;
float4m03_2	:	TEXCOORD3;
float4m03_3	:	TEXCOORD4;
float4mWeights	:	TEXCOORD5;

float2mOffset	:	TEXCOORD6;

float3x4worldMatrix[4];
worldMatrix[0][0] =tex2D(matrixTexture, m03_0.xw +mOffset );
worldMatrix[0][1] =tex2D(matrixTexture, m03_0.yw +mOffset );
worldMatrix[0][2] =tex2D(matrixTexture, m03_0.zw +mOffset );

worldMatrix[1][0] =tex2D(matrixTexture, m03_1.xw +mOffset );
worldMatrix[1][1] =tex2D(matrixTexture, m03_1.yw +mOffset );
worldMatrix[1][2] =tex2D(matrixTexture, m03_1.zw +mOffset );

worldMatrix[2][0] =tex2D(matrixTexture, m03_2.xw +mOffset );
worldMatrix[2][1] =tex2D(matrixTexture, m03_2.yw +mOffset );
worldMatrix[2][2] =tex2D(matrixTexture, m03_2.zw +mOffset );

worldMatrix[3][0] =tex2D(matrixTexture, m03_3.xw +mOffset );
worldMatrix[3][1] =tex2D(matrixTexture, m03_3.yw +mOffset );
worldMatrix[3][2] =tex2D(matrixTexture, m03_3.zw +mOffset );

float4 worldPos = float4(mul(worldMatrix[0], inPos ).xyz, 1.0f )* mWeights.x;
worldPos +=float4(mul(worldMatrix[1], inPos ).xyz, 1.0f )* mWeights.y;
worldPos +=float4(mul(worldMatrix[2], inPos ).xyz, 1.0f )* mWeights.z;
worldPos +=float4(mul(worldMatrix[3], inPos ).xyz, 1.0f )* mWeights.w;

float4 worldNor = float4(mul(worldMatrix[0], inNor ).xyz, 1.0f )* mWeights.x;
worldNor +=float4(mul(worldMatrix[1], inNor ).xyz, 1.0f )* mWeights.y;
worldNor +=float4(mul(worldMatrix[2], inNor ).xyz, 1.0f )* mWeights.z;
worldNor +=float4(mul(worldMatrix[3], inNor ).xyz, 1.0f )* mWeights.w;
```

As you can witness, a HW VTF vertex shader with 4 weights per vertex needs a lot of texture fetches.
Fortunately they fit the texture cache very well; nonetheless it's something to keep watching out.

Instancing is meant for rendering large number of objects in a scene.
If you plan on rendering thousands or tens of thousands of animated objects with 4 weights per vertex, don't expect it to be fast; no matter what technique you use to draw them.

Try convincing the art department to lower the animation quality or just use Ogre::IM_FORCEONEWEIGHT for Ogre to do the downgrade for you.
There are many plugins for popular modeling packages (3DS Max, Maya, Blender) out there that help automatizing this task.

@par Q: The instance doesn't show up, or when playing animations the mesh deforms very weirdly or other very visible artifacts occur

A: Your rig uses more than one weight per vertex.
Either create the instance manager with the flag Ogre::IM_FORCEONEWEIGHT, or modify the vertex shader to support the *exact* amount of weights per vertex needed (see previous questions).

@par Q: How do I find how many weights per vertices is using my model?

A: The quickest way is by looking at the type of Ogre::VES_BLEND_WEIGHTS, where `VET_FLOAT<N>` means N weights.

[^8]: In theory all other techniques could implement custom parameters but for performance reasons only HW VTF is well suited to implement it.
      Though it yet remains to be seen whether it should be passed to the shader through the VTF, or through additional TEXCOORDs.

[^9]: Only HW instancing techniques cull per instance.
      SW instancing techniques send all of their instances, zeroing matrices of those instances that are not in the scene.
