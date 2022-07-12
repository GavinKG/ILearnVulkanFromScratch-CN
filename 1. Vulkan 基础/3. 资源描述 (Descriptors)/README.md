# 资源描述 (Descriptor) 和资源的绑定 (Binding)



## 概念

从这一章开始，我们要通过客户端向 Shader 中传递一些通用的数据（而并非流水线中传递的变量，如顶点坐标/颜色等）来增加 Shader 的功能，诸如视口参数（变换矩阵、时间、Framebuffer 大小等），纹理贴图和其它几乎任何数据或结构体。**这些向 Shader 提供的，需要被绑定在流水线上的资源**在客户端中被成为 "Descriptor"，即资源描述，或资源描述符。

一个 Descriptor 的种类有很多，常见的例如（这里使用 `VkDescriptorType` 中的枚举名称）：

* `VK_DESCRIPTOR_TYPE_SAMPLER`：采样器，指定图片被读取的方式，之后会提到。
* `VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE`：Shader 中能被采样的纹理对象。
* `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER`：将采样器和能够采样的纹理对象打包起来，成为一个资源描述，之后会提到。
* `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER`：Uniform Buffer Object (UBO)，存储一些 Shader 只读的小批量数据。见下章。
* `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER`：存储缓冲 Shader Storage Buffer Object (SSBO)，允许我们在 Shader 中读/写缓存中的值。
* `VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER`：缓冲纹理 (Buffer Texture)，其中每个元素为 GPU 特定的“像素”格式（例如 `RGBA8_UNorm`），Shader 中读取该“像素”会被解释为可直接使用的标量或向量（例如上述格式对应为 `vec4`）

详细列表可以参考[官方文档](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkDescriptorType.html)。

其实，所有的 Descriptor 大致上分为三类：**采样器、图像和通用数据缓冲（UBO, SSBO）**，而区别就在于用途和随之而来的性能优化。

每一个 shader 可能会用不止一个 descriptor，**将所有 descriptors 打包起来便被叫做 descriptor set**（本章节之后将会简称为 set），称为**资源描述符集**。在渲染时我们不一个一个发资源描述，而是一下发一整个描述符集给流水线传入 shader。同时，每一个描述符集都配套有一个**描述符集布局 Descriptor Set Layout**，可以理解为**资源的布局、接口、协议、约定**，就像一个集装箱会配上一个**内容清单**一样，来说明这一个资源描述集中的资源描述个数和类型等。



## 构建流水线资源绑定"模板"：Pipeline Layout

![img](https://www.oreilly.com/library/view/vulkan-cookbook/9781786468154/assets/B05542-08-03-1.png)

*图出处：https://www.oreilly.com/library/view/vulkan-cookbook/9781786468154/assets/B05542-08-03-1.png*



### 创建 Descriptor Set Layout Binding
*或简称为 Binding / DSL Binding。*

每一个想在 Shader 中使用的资源都需要通过填写 `VkDescriptorSetLayoutBinding` 结构体，**声明一个用来绑定的资源，即 Descriptor**。这里举一个配置给 Vertex Shader 传入变换矩阵的 Uniform Buffer 的例子：

- `binding`：与 shader 中 `binding` 字段的数字一致，这样就让客户端的资源和 Pipeline Layout，即 Shader 中的变量 “绑定”上了，可以看下一章里面的例子。
- `descriptorType `：资源描述类型。这里直接给 `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER`。
- `descriptorCount`：资源描述个数。这里只想用一个 Uniform Buffer，因此设置为 1。大于 1 就表示一个数组（Array of Descriptors），在 Shader 中可以直接计算出索引值并索引该数组获取资源，[详见此处](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_descriptor_indexing.html)。
- `stageFlags`：该资源被引用的着色器阶段，这里由于是给顶点着色器做坐标变换用，所以给 `VK_SHADER_STAGE_VERTEX_BIT`。
- `pImmutableSamplers`：在配置不可变采样器（Layout 创建后不允许更改）时才会用到，这里留空，即 `nullptr`。

结构体定义和使用方法如下：

```cpp
typedef struct VkDescriptorSetLayoutBinding {
    uint32_t              binding;
    VkDescriptorType      descriptorType;
    uint32_t              descriptorCount;
    VkShaderStageFlags    stageFlags;
    const VkSampler*      pImmutableSamplers;
} VkDescriptorSetLayoutBinding;
```

```cpp
VkDescriptorSetLayoutBinding layoutBinding{};

layoutBinding.binding = 0;
layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
layoutBinding.descriptorCount = 1;
layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
layoutBinding.pImmutableSampler = nullptr;
```

注意！此处仅仅是在描述一个 Descriptor，但并没有传入任何具体的资源。



### 创建资源描述集布局 Descriptor Set Layout
*本章节之后将会简称为 layout。*

多个 Descriptor 在一起，将会组成一个 Descriptor Set（资源描述集），而描述这个 Descriptor Set 布局的对象则被称为 Descriptor Set Layout。

接下来我们终于要创建出这个 Layout 了。创建一个 `VkDescriptorSetLayoutCreateInfo`，里面传入上述所有资源描述的`VkDescriptorSetLayoutBinding`数组。之后，通过`vkCreateDescriptorSetLayout`创建出真正需要的 layout 到成员变量 `VkDescriptorSetLayout descriptorSetLayout` 中。不要忘记在 cleanup 阶段通过`vkDestroyDescriptorSetLayout`进行销毁。

```cpp
typedef struct VkDescriptorSetLayoutCreateInfo {
    VkStructureType                        sType;
    const void*                            pNext;
    VkDescriptorSetLayoutCreateFlags       flags;
    uint32_t                               bindingCount;
    const VkDescriptorSetLayoutBinding*    pBindings;    // mentioned above!
} VkDescriptorSetLayoutCreateInfo;
```



### 将 Layout 传入 Pipeline 创建信息中

在进入到流水线创建的 `VkPipelineLayoutCreateInfo` 时（详见上文 **图形流水线 (Graphics Pipeline)** 一章），将刚刚创建好的描述布局传递到该结构体中的 `setLayoutCount` 和 `descriptorSetLayout`。

```cpp
typedef struct VkPipelineLayoutCreateInfo {
    VkStructureType                 sType;
    const void*                     pNext;
    VkPipelineLayoutCreateFlags     flags;
    uint32_t                        setLayoutCount; // !
    const VkDescriptorSetLayout*    pSetLayouts;    // !
    uint32_t                        pushConstantRangeCount;
    const VkPushConstantRange*      pPushConstantRanges;
} VkPipelineLayoutCreateInfo;
```

> **由于 Descriptor Set Layout 是流水线的一部分**，因此声明 layout 的位置需要在创建流水线之前，即在上述 `createGraphPipeline()` 之前。创建流水线的时候需要传入这个 layout 来生成 Pipeline Layout。两种 Layout 都是修饰数据的布局的，别搞混。



## 资源描述池 Descriptor Pool

Descriptor Set 中的 descriptor 并不能直接创建，而是也要通过一个资源描述池 descriptor pool 来创建。一个池包含几个子池（对应英文为 pool size），分别对应着一种 descriptor 类型，例如：

```cpp
VkDescriptorPoolSize poolSize = {};
poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 我需要 Uniform Buffer!
poolSize.descriptorCount = static_cast<uint32_t>(swapChainImages.size()); // 一个 Swap Chain Image 来一个
```

这里创建了一个针对 uniform buffer 的子池，下面会介绍。

所有子池的类型描述好后，通过`VkDescriptorPoolCreateInfo`并传入子池数组来创建资源描述池。同时，这个结构体需要让我们指定最大可能分配 descriptor sets 的数目（原因见下）。之后，通过调用`vkCreateDescriptorPool`创建资源描述池到成员变量`VkDescriptorPool descriptorPool`中。同理，不要忘记使用`vkDestroyDescriptorPool`进行销毁。



## 创建具体资源

### 创建资源描述集 Descriptor Set

终于，到了可以创建出 Descriptor Set 的阶段了。填充 `VkDescriptorSetAllocateInfo ` 结构体，其中包括：

* `descriptorPool`：刚刚创建的池。
* `descriptorSetCount`：descriptor set 的个数
* `pSetLayouts`：所有 layout 的数组。注意**在本例中**，一个资源描述集对应一个 Swap Chain Image，所以这里的数组每个元素都为之前创建出的那一个 `descriptorSetLayout` 的拷贝。

```cpp
typedef struct VkDescriptorSetAllocateInfo {
    VkStructureType                 sType;
    const void*                     pNext;
    VkDescriptorPool                descriptorPool;
    uint32_t                        descriptorSetCount;
    const VkDescriptorSetLayout*    pSetLayouts;
} VkDescriptorSetAllocateInfo;
```

接下来**使用 `vkAllocateDescriptorSets` 创建一个 descriptor set**。注意，这个资源描述池创建方法并没有创建出我们需要保存并销毁的变量，当销毁资源描述池时，资源描述集也会被销毁。

```cpp
VKAPI_ATTR VkResult VKAPI_CALL vkAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets);
```

分配一个 descriptor set 时，Vulkan 从对应子池中取出 layout 中描述的这种 descriptor，然后拼接成一个完整的 set。而之前提到的 command buffer pool 就没有子池这些概念（可能因为命令所占内存是等长的而且不断变化，所以直接使用了一个固定大小的内存分配器？）

其实在本例中分配资源描述池的数量完全卡着需要的资源设置了池和针对某一类资源描述的子池的大小，当分配完所有资源描述集后，池子里面就没有任何空间了。



### 配置每一个 Descriptor 的资源

Descriptor 描述完了，Layout 描述完了，Set 创建出来了，现在就该**配置每一个 Descriptor 的具体资源**（Buffer / Image）了。

对于那些缓冲类型的资源描述，需要用 `VkDescriptorBufferInfo` 结构体来**描述 descriptor 本身的属性**：

* `buffer`：具体的缓冲对象。在这里我们传入对应的 uniform buffer。
* `offset` 和 `range`：缓冲的哪个位置。这里直接设 offset 为 0，range 为 `sizeof(UniformBufferObject)`。或者，由于这里把整个缓冲对象 `VkBuffer` 都用于这一个 UBO 了，所以可以直接在range里面设置 `VK_WHOLE_SIZE`。

```cpp
typedef struct VkDescriptorBufferInfo {
    VkBuffer        buffer;
    VkDeviceSize    offset;
    VkDeviceSize    range;
} VkDescriptorBufferInfo;
```

将该结构体更新到资源描述中需要再填写一个`VkWriteDescriptorSet`，**用来描述 Descriptor 和 Set 的关系**：

* `dstSet`：目标 Descriptor Set。
* `dstBinding`：这个资源描述在资源描述集中的绑定。需要与 Layout 中声明的绑定相同。
* `dstArrayElement`：由于资源描述可以是数组，这里需要指定一个需要更新的数组下标。
* `descriptorCount`：从下标开始更新几个数组元素。

上述两个成员如果不是数组则设为0, 1。

* `descriptorType`：描述类型。传入一个`VkDescriptorType`枚举成员，例如 `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER`。
* `pBufferInfo` 或 `pImageInfo` 或 `pTexelBufferInfo`：**具体资源。**由于上述描述的是缓冲类型，这里只填写`pBufferInfo`为刚刚声明的`VkDescriptorBufferInfo`实例，其余两项填为`nullptr`。之后在介绍“组合图像采样器”的时候，我们会去设置 `pImageInfo`。

```cpp
typedef struct VkWriteDescriptorSet {
    VkStructureType                  sType;
    const void*                      pNext;
    VkDescriptorSet                  dstSet;
    uint32_t                         dstBinding;
    uint32_t                         dstArrayElement;
    uint32_t                         descriptorCount;
    VkDescriptorType                 descriptorType;
    const VkDescriptorImageInfo*     pImageInfo;
    const VkDescriptorBufferInfo*    pBufferInfo;
    const VkBufferView*              pTexelBufferView;
} VkWriteDescriptorSet;
```

之后调用 `vkUpdateDescriptorSets` 方法，将上述所有的配置写入系统托管的对应资源描述集中。这个方法兼容两种类型的数组：这里使用的 `VkWriteDescriptorSet`，和 `VkCopyDescriptorSet`。

```cpp
// [VkWriteDescriptorSet, VkWriteDescriptorSet ... VkWriteDescriptorSet] => vkUpdateDescriptorSets
VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies);
```

注意，之前配置了这么多，这里才**真正**将 descriptor 所使用的 buffer 或 ImageView/Sampler “注册”进 descriptor sets。这里称之为“注册”的原因是，write 进的内容也只是 buffer 或 ImageView/Sampler 的句柄，并没有产生数据的复制，**真正绘制的时候还是使用的原始缓冲中的内容**。因此，如果想更新内容可以**直接对原缓冲动手**，那些所有 Shader 都可能需要的资源（例如时间、分辨率、VP变换矩阵等）可以**只创建一份缓冲**然后注册进多个 descriptor sets 中。

但是需要注意，**不要在“渲染时”更新 descriptor sets**，即不要在 `vkBeginCommandBuffer` 到同步机制告诉我们 Command Buffer 已执行完毕（注意不是 `vkQueueSubmit`），否则会因为没有和绘制指令同步导致数据错误，产生未定义行为。



## 绘制时绑定 Descriptor Set

做了这么多，现在终于可以使用创建出来的 set 了。录制 Command Buffer 时，在对应绘制命令（`vkCmdDraw`）之前，使用 `vkCmdBindDescriptorSets` 将其所需要的 Set 绑定在流水线上：

```c++
vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
```

```c++
void vkCmdBindDescriptorSets(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    firstSet,
    uint32_t                                    descriptorSetCount,
    const VkDescriptorSet*                      pDescriptorSets,
    uint32_t                                    dynamicOffsetCount,
    const uint32_t*                             pDynamicOffsets);
```

注意这里也是通过 `vkCmd` 系列方法，即绑定资源描述集也是一个需要被命令缓冲录下来的命令。后两个参数为 Dynamic Descriptors 的参数，本例先不去管，之后将会在 **Vulkan 进阶** 中 **Dynamic Uniform Buffer** 提及。

一个常见的绘制指令如下：

```c++
vkCmdBindPipeline        // 绑定图形流水线
vkCmdBindDescriptorSets  // 绑定资源          |
vkCmdBindIndexBuffer     // 绑定模型的索引缓冲  | 这些操作将不同类型的数据绑定到了图形流水线的 Bind Points 上。
vkCmdBindVertexBuffer    // 绑定模型的顶点缓冲  |
vkCmdDrawIndexed         // 绘制指令
```




### Descriptor Set 和 Pipeline 的兼容性

我们使用 `vkCmdBindDescriptorSets` 时，并不需要先绑定一个 Pipeline（毕竟该方法只需要传入 Pipeline Layout 即可）。但需要保证 Descriptor Set 和当前绑定的 Pipeline 的[兼容性](https://www.khronos.org/registry/vulkan/specs/1.3-khr-extensions/html/chap14.html#descriptorsets-compatibility)。因此，如果我们的 Descriptor Set 表示的数据能够跨 Pipeline 使用（例如 View 变换矩阵、时间等），我们可以在最开始时绑定这个 Descriptor Set，之后不管 Pipeline 怎么切换（通过 `vkCmdBindPipeline`），只要这个 Set 和所有的 Pipeline 兼容，就可以不重新绑定直接使用。例如下面的绘制伪代码：

```c++
// https://developer.nvidia.com/vulkan-shader-resource-binding

// example for typical loops in rendering
for each view {
  bind view resources          // camera, environment...
  for each shader {
    bind shader pipeline  
    bind shader resources      // shader control values
    for each material {
      bind material resources  // material parameters and textures
      for each object {
        bind object resources  // object transforms
        draw object
      }
    }
  }
}
```

因此，根据兼容性的定义，我们可以把最不易变（例如 View 相关的 Set）放在第 0 个，最易变的（例如逐物体相关的 Set）放在最后一个，以减少 `vkCmdBindDescriptorSets` 的调用次数。



## Shader 中使用

在 GLSL 中，可以通过 `layout(binding = X)`  后加变量名称来指定需要访问的 Descriptor 资源了。例如：

```glsl
layout(binding = 0) uniform Ubo {...} uboA;
layout(binding = 1) uniform sampler2D texSampler
// ...
```



## 多个 Descriptor Sets

可以同时绑定多个资源描述集。除了更改相关的结构体成员和函数参数，在 shader 中也要注明使用的描述集的编号：

```GLSL
layout(set = 0, binding = 0) uniform Ubo { ... } uboA;
// ...
layout(set = 1, binding = 0) uniform Ubo { ... } uboB;
// ...
```

但是需要注意，我们能使用的 Descriptor Sets 是有数量上限的，即 `VkPhysicalDeviceLimits::maxBoundDescriptorSets`。该值在笔者的机器上是 16。



## Tips：关于 Binding 所使用的数字连续性

> https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#descriptorsets-setlayout
>
> The above layout definition allows the descriptor bindings to be specified sparsely such that not all binding numbers between 0 and the maximum binding number need to be specified in the `pBindings` array. Bindings that are not specified have a `descriptorCount` and `stageFlags` of zero, and the value of `descriptorType` is undefined. However, all binding numbers between 0 and the maximum binding number in the [VkDescriptorSetLayoutCreateInfo](https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#VkDescriptorSetLayoutCreateInfo)::`pBindings` array **may** consume memory in the descriptor set layout even if not all descriptor bindings are used, though it **should** not consume additional memory from the descriptor pool.

一个 Layout 中的 Descriptor Binding **不一定非得从 0 开始连续下去**（0, 1, 2...），即**完全可以绑定任意一个不超过 Limits 的数**（0, 3, 7, 200...），只要保证客户端和 Shader 中的绑定 ID 相同即可。但是，驱动实现可能会将从 0 到最高 Binding ID 中间的每个 Descriptor Set Layout Binding 都分配内存，**因此最好还是保持 Binding ID 的紧凑。**

