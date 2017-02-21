
The Command Buffer {#commandbuffer}
==================

The Command Buffer was the response to a very practical problem: Ogre
couldn't assume persistent mapping is supported as this would break
compatibility with DX10-level hardware and DX11 API.

This means that we need to map a buffer to update its contents. But we
can't issue a draw call while a buffer is mapped. Unmapping and mapping
before and after each draw call would be extremely costly; and therefore
a command buffer was needed that would record all the API calls that
have to be made; in order for buffers (i.e. constant and texture buffers
to send data to the shaders) to be mapped only once.

Once we're done updating all buffers, the command buffer is executed,
playing back all the API calls in the right order.

Note that the command buffer is written for high performance, low
footprint; thus its inner workings and interface are very C-like.

@tableofcontents

# Adding a command {#CommandBufferAddCommand}

A C++ template is used to simplify the process; though you are not
forced to use it.

The function "addCommand" creates an uninitialized command, and it's
your job to initialize it to valid values.

The following snippet will record the command to set a new Vao, which
will eventually calling `RenderSystem::_setVertexArrayObject` when it
gets executed:

```cpp
VertexArrayObject*vao=/*...*/;
*mCommandBuffer->addCommand<CbVao>()=CbVao(vao);
```

The following snippet will issue a v1-style indexed draw call:
```cpp
v1::CbDrawCallIndexed*drawCall=
        mCommandBuffer->addCommand<v1::CbDrawCallIndexed>();
*drawCall=v1::CbDrawCallIndexed();
drawCall->operationType    =renderOp.operationType;
drawCall->useGlobalInstancingVertexBufferIsAvailable=
        renderOp.useGlobalInstancingVertexBufferIsAvailable;
drawCall->primCount        =renderOp.indexData->indexCount;
drawCall->instanceCount    =renderOp.numberOfInstances;
drawCall->firstVertexIndex =renderOp.indexData->indexStart;
drawCall->baseInstance     =baseInstance;
```

All the CommandBuffer-related code is under the CommandBuffer folder. By
convention, all commands start with the `Cb` prefix.

The returned pointer should not be kept for long as further calls to
`addCommand` may invalidate the pointer.

# Structure of a command {#CommandBufferCommandStructure}

Each command is, at the time of writting, of a fixed size of 32 bytes.
This value is controlled by `CommandBuffer::COMMAND_FIXED_SIZE`. The
first 2 bytes are reserved for the command type; which leaves you with
30 bytes of actual space.

Beware of storing pointers in the command: In a 32-bit environment each
pointer is 4 bytes. But in a 64-bit environment, each pointer is 8
bytes. Make sure they still fit within the size of the command.

If 32 bytes is not enough, you can try researching into implementing
multi-commands (an action that spans multiple commands and would read
back and forth through the command buffer to obtain all the necessary
data, beware of respecting the header in each command!); or raise the
`COMMAND_FIXED_SIZE` constant.

Keeping the constant size low is vital to achieve good bandwidth and
cache utilization, and hence keep the overhead to a minimum.

# Execution table {#CommandBufferExecutionTable}

The command buffer is implemented via a manual virtual table. The
variable CommandBuffer::CommandBufferExecuteFunc*
CbExecutionTable[MAX_COMMAND_BUFFER+1]` is populated with all the
entries to this vtable. See enum `CbType` for all the possible entries.

Each function accepts a pointer to `this` state and the command
itself.

Commands are stored in structs and its constructors are used to ensure
each command populates the command type value properly.

Example:

To implement a command that will call
`renderSystem->_setHlmsMacroblock( myMacroblock )`; a structure is
defined:

```cpp
struct_OgreExportCbMacroblock:publicCbBase
{
    HlmsMacroblockconst   *block;
    CbMacroblock(constHlmsMacroblock*_block);
};
```

`Block` is the `myMacroblock` we want to set. We derive from `CbBase`
which conveniently has already the 16-bit value defined. Since `CbBase` is
only 2 bytes, take in mind compilers may add padding between the end of
CbBase and the beginning of CbMacroblock unless compiler packing
extensions[^16] are used; which leaves us with an usable space of 28
bytes in 32-bit environments, or 24 bytes in 64-bit. Note that you're
not required to derive from `CbBase` (in case you're pressed with the
command's size). This is just for convenience and consistency.

The constructor will populate all the proper values:

```cpp
CbMacroblock::CbMacroblock(constHlmsMacroblock*_block):
    CbBase(CB_SET_MACROBLOCK),
    block(_block)
{
}
```

The constructor sets the command type to `CB_SET_MACROBLOCK` which is
already part of `CbType`.

Therefore running `*commandBuffer->addCommand<CbMacroblock>() =
CbMacroblock( myMacroblock )`; will just work.

But there's one more thing: We need to implement the actual command!

For that, we declare and define the following function:

```cpp
 
//Declaration
class_OgreExportCommandBuffer
{
    //...
public:
    staticCommandBufferExecuteFuncexecute_setMacroblock;
    //...
};
//Definition
voidCommandBuffer::execute_setMacroblock(CommandBuffer*_this,constCbBase*RESTRICT_ALIAS_cmd)
{
    constCbMacroblock*cmd=static_cast<constCbMacroblock*>(_cmd);
    _this->mRenderSystem->_setHlmsMacroblock(cmd->block);
}	 

```

Last but not least, we have to add the function to the table:

```cpp
CommandBuffer::CommandBufferExecuteFunc*CbExecutionTable[MAX_COMMAND_BUFFER+1]=
{
    //...
    //MAKESURETHEINDEXPOSITIONMATCHESTHEVALUEOFCB_SET_MACROBLOCK
    &CommandBuffer::execute_setMacroblock,
    //...
};	 
```

The comment's remark is very important: You can't add
`execute_setMacroblock` anywhere, you must store it at the same index as
the value of `CB_SET_MACROBLOCK`, so that
`CbExecutionTable[CB_SET_MACROBLOCK]` points to to
`execute_setMacroblock`.

## Hacks and Tricks {#CommandBufferExecutionTableHacks}

Maintaining a manual vtable may seem like a lot of trouble, but the
amount of commands is manageable (don't feel tempted to add one command
for every little thing!) and it has very powerful benefits: You can
manipulate the command type with simple integer arithmetic, thus saving
expensive branch evaluations per object.

For example, Ogre supports indirect buffers when available. But when
indirect buffers aren't supported, not only we have to store the draw
command data in CPU memory rather than GPU memory, but we also have to
use very different draw calls. This is shown in `RenderSystem::_render`
vs `RenderSystem::_renderEmulated`.

The former uses indirect buffer draw calls, the latter doesn't. Both do
the exact same thing.

Usually, during command generation, the RenderQueue could look like
this:

```cpp
for(size_ti=0;i<numRenderables;++i)
{
    if(supportsIndirectBuffers)
        *mCommandBuffer->addCommand<CbDrawCallIndexed>()=CbDrawCallIndexed();
    else
        *mCommandBuffer->addCommand<CbDrawCallIndexedEmulated>()=CbDrawCallIndexedEmulated();
}
```

This involves a branch per renderable. Since there can be literally
hundred of thousands of them, this is costly.

Instead, you can just store the indirectBuffer version and the emulated
together in the vtable, and use integer arithmetic to modify the index
to the table:

```cpp
 
CbDrawCallIndexed::CbDrawCallIndexed(boolsupportsIndirectBuffers):
    CbDrawCall(CB_DRAW_CALL_INDEXED_EMULATED+supportsIndirectBuffers)
{
}	 
```

Then during the command generation in the RenderQueue just write:

```cpp
bool supportsIndirectBuffers = /*...*/;
for(size_ti=0;i<numRenderables;++i)
{
    *mCommandBuffer->addCommand<CbDrawCallIndexed>()=
                            CbDrawCallIndexed( supportsIndirectBuffers );
}
```

It's not just cleaner, it's also more efficient. A cmp, a jmp and a mov
instruction (plus a potential miss-prediction) was just transformed into
a single add.

# Post-processing the command buffer {#CommandBufferPostProcessing}

Another advantage of a command buffer, is that you may have more
information about the rendering sequence only after you have processed
all objects. You could also try to detect redundant state changes (Ogre
doesn't do this though).

For example, both texture buffers require binding them to a shader by
specifying an offset and a size.

The offset is known at the time the texture buffer is needed, but the
size is not. The same texture buffer may be used to store data of many
subsequent Renderables, and thus required size may not be known until
we're done processing all of them.

For example, the Hlms implementations shipped with Ogre populate this
size at the end. For that, it saves the command location with
`CommandBuffer::getCommandOffset` called after adding the command:

```cpp
CbShaderBuffer*shaderBufferCmd=commandBuffer->addCommand<CbShaderBuffer>();
*shaderBufferCmd=CbShaderBuffer(shaderResourceSlot,
                                   texBuffer,offsetStart,0);
size_ttexBufferOffset=commandBuffer->getCommandOffset(shaderBufferCmd);	 
```

The size at this point is set to 0, which is wrong. This will be
corrected once we're done:

```cpp
//Setthebindingsizeoftheoldbindingcommand(ifexists)
CbShaderBuffer*shaderBufferCmd=reinterpret_cast<CbShaderBuffer*>(
            commandBuffer->getCommandFromOffset(texBufferOffset));
shaderBufferCmd->bindSizeBytes=writtenBytes;
```

The function `getCommandFromOffset` will only return a null pointer if the
value from `texBufferOffset` is invalid.

We need to use these functions, as saving the command's pointer is not
safe as it is not guaranteed to be valid after the next `addCommand` call.

[^14]: For example DX11 doesn't allow this level of flexibility. However
    we can still batch all meshes into the same vertex buffer, and in
    the same index buffer; texture buffers are also very flexible.

[^15]: On DX11, we can emulate fences using dummy occlusion queries.

[^16]: Like \#pragma pack
