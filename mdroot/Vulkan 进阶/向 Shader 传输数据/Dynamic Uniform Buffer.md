## Dynamic Uniform Buffer

> Use a single uniform buffer object as a dynamic uniform buffer to draw multiple objects with different matrices from one big uniform buffer object.

之前在创建UBO的时候，每一个物体（当然不会超过两个物体……）都手动分配了一个UBO和关联的内存。这种方法简单直接，但是当物体多的时候，我们可以选择建立一个大的 Uniform Buffer 缓冲存所有的数据，每一个 draw call 传入这个大集合的一部分作为绘制物体的 Uniform Buffer。使用这种方法可以在没有缓冲内存分配器的时候减小分配（因为分配次数有限制），并且由于数据空间局部性，这样做可以优化内存和 Cache 的命中率。

但需要注意的是，Dynamic Uniform Buffer 也是有分配限制的（笑），Vulkan 官方规定这个分配限制必须 >= 8，在笔者的机器上测得的是 `maxDescriptorSetUniformBuffersDynamic = 15`。

### 准备 Buffer

需要注意的是，动态 Uniform Buffer （下称 DUB 🙃） 里面每一个元素的偏移量（对齐）有最小限制，这个限制一般是256B，所以如果你每一个元素装的东西不够256B（比如这个例子中就装了个M变换矩阵，即4x4x4=64Bytes），你就要填充剩下的空间来撑起来256B，当然如果不足了你也要把空间撑到256B的倍数上去。

这个对齐分配的方法 C++ 目前没有统一规范，因此这里写一个抽象函数：

```c++
void* alignedAlloc(size_t size, size_t alignment)
{
	void *data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
	data = _aligned_malloc(size, alignment);
#else 
	int res = posix_memalign(&data, alignment, size);
	if (res != 0)
		data = nullptr;
#endif
	return data;
}
```

在此例中，`size`参数传递对齐大小 `alighment` * 用这个DUB绘制物体的数量。

之后调用通用的那套生成 Buffer 的方法。注意这里的 memory property flags 可以不用传入 `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`，因为我们可能会每次更改这个大 Buffer 的一小部分，所以我们会手动调用 `vkFlushMappedMemoryRanges` 来 flush 这一小部分。参考：

> `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` bit specifies that the host cache management commands [vkFlushMappedMemoryRanges](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkFlushMappedMemoryRanges.html) and [vkInvalidateMappedMemoryRanges](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkInvalidateMappedMemoryRanges.html) are not needed to flush host writes to the device or make device writes visible to the host, respectively.

### 准备 Descriptor Set

在这个例子中，VP变换矩阵打包成一个struct，M变换矩阵单独拿出来，这样客户端应用程序就可以单独为其指定DUB中的一部分了。shader 绑定代码如下：

```GLSL
layout (binding = 0) uniform UboView 
{
	mat4 projection;
	mat4 view;
} uboView;

layout (binding = 1) uniform UboInstance 
{
	mat4 model; 
} uboInstance;
```

注意 shader 中并没有出现任何跟DUB有关的声明，DUB完全是客户端所要顾及的。

设置 descriptor sets 还是要走三部曲：

* Descriptor Pool：这里为所有物体生成**一个**类型为`VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC`的 pool size。
* Descriptor Set Layout：在 Binding 中添加一个绑定在 `VK_SHADER_STAGE_VERTEX_BIT` 上的 `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC`。
* Descriptor Set 的具体关联缓存和绑定环节：在创建完 `descriptorSet` 后写入时，`VkWriteDescriptorSet` 中的 `descriptorType` 同样要指定为 `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC`。

### 绘制命令

在 command buffer 绘制中的 `vkCmdBindDescriptorSets` 函数的最后两个参数适合 Dynamic Uniform Buffer 有关的：

* `dynamicOffsetCount`：set 中有几个 dynamic descriptor？同时也是下一个数组的大小。
* `pDynamicOffsets`：具体 `uint32_t` 偏移量。这个 offset 只会影响 descriptor set 中声明为 dynamic uniform buffer 和 storage buffer 的 descriptor。这个数组对应的 dynamic descriptor 按照 `binding` 号顺序排列。

> If any of the sets being bound include dynamic uniform or storage buffers, then `pDynamicOffsets` includes one element for each array element in each dynamic descriptor type binding in each set. Values are taken from `pDynamicOffsets` in an order such that all entries for set N come before set N+1; within a set, entries are ordered by the binding numbers in the descriptor set layouts; and within a binding array, elements are in order. `dynamicOffsetCount` **must** equal the total number of dynamic descriptors in the sets being bound.

### 更改 Dynamic Uniform Buffer 中的内容

更改的方法和上面介绍的类似，都是先 `vkMapMemory` ，再 `memcpy`，在 `vkUnmapMemory`。但是由于没有声明 `VK_MEMORY_PROPERTY_HOST_COHERENT_BIT` ，所以得手动保持数据的同步（coherent）。在 unmap 之前使用 `vkFlushMappedMemoryRanges` 提交想要改变的 DUB 中的内容。
