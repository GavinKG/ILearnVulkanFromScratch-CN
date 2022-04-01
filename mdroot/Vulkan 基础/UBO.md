## Uniform 缓冲 (Uniform Buffer Object)

**原文：https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets。这里我打乱顺序重新组织了一下。**



### 概念

如上所述，资源描述分为很多种，由于这里要实现三维的变换，需要传入 MVP 矩阵，所以我们需要一种特定的资源描述，叫 Uniform Buffer Object，**其对应 Shader 中的 uniform 常量**。之所以称之为 "Uniform"，正是因为这些数据（在没有显式被客户端改变时）**在所有使用这些数据的 Shader 实例中都不会改变，即只读**。

> 与其相对的概念是之前提到过的 SSBO (Shader Storage Buffer Object)，其可以被原子写入。在其他 API、引擎中通常也被称为 UAV。

在 C++ 客户端中，一个我们需要的 UBO 结构体如下所示。注意：glm 代数库中的 `glm::mat4` 类可直接对应上 shader 中 `mat4` 类型，即这两个类型二进制兼容，同时这个结构体满足一样的布局（见下），所以在复制到 buffer 时可以直接用 `memcpy` 函数。

```cpp
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
```

之后，需要在 Shader 中更新对应的接收部分。再复习一下，`binding` 指的是和 UBO 等资源描述符绑定，而 `location` 和 Vertex Buffer 中特定顶点属性（在`VkVertexInputAttributeDescription`中声明）绑定，不要弄混了。

```GLSL
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;
```

在 Shader 中的用法举例如下：

```GLSL
void main() {
    gl_position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
}
```

需要注意，在 OpenGL 时代，可以不用 "类struct" 格式进行定义，而直接裸用变量（`uniform mat4 proj`），但是在 Vulkan 和 SPIR-V 中必须将 uniform 组合成上面的 [“Uniform Block” 结构体](https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL))。



### 创建 Uniform Buffer

> 由于我们每一帧都有可能对这个 Uniform Buffer 进行数据的改变，就如这里的传递 MVP 的做法一样，所以这里没有必要为其声明 Staging Buffer，而相反，场景中的顶点们一旦声明出来可能会有一段时间不进行修改。

在该案例中，由于 uniform buffer 需要在 command buffer 中被引用，而 command buffer 数量对应着 swap chain image 的数量，所以这里创建的 uniform buffer 数量与 swap chain image 的数量一致。

同上，创建两个成员变量 `std::vector<VkBuffer> uniformBuffers` 和 `std::vector<VkDeviceMemory> uniformBuffersMemory` 并 `resize` 为 swap chain image 的数量。

创建 uniform buffer 的时机和创建其它（vertex, index）buffer的时机相同。示例代码如下：

```c++
VkDeviceSize bufferSize = sizeof(UniformBufferObject);
for (size_t i = 0; i < swapChainImages.size(); i++) {
    createBuffer(bufferSize, 
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 uniformBuffers[i],
                 uniformBuffersMemory[i]);
}
```

一旦声明，这些 buffer 就可以不去动它了，直到渲染结束。在`cleanupSwapChain` 时通过使用`vkDestroyBuffer` 和 `vkFreeMemory` 来释放空间。这也意味着当 swap chain 改动时，也要重新建立这些 uniform buffer。

在 draw loop 中，当获取到 swap chain image 时，立刻对该帧的 uniform buffer 进行更新即可，除了写缓冲之外同样不需要做任何工作。在本例中，我们让示例 quad 持续转动。



### 更新 Uniform Buffer

这里就要进入到真正的变换运算环节了。该案例中，首先通过**捕获运行时长**来计算旋转角度，即 `angle = time * anglePerSec`。通过使用标准库中的的高精度时钟`std::chrono::steady_clock`并在每次渲染中获取到该帧与**第一帧**的时间差：

```cpp
static auto startTime = std::chrono::steady_clock::now();
auto currentTime = std::chrono::steady_clock::now();
float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
```

> 注意，不要使用教程中使用的 `high_resolution_clock`，因为其可能被 typedef 为 `system_clock`。

接下来开始计算**该案例中**整套变换的 MVP 矩阵：

* 通过`glm::rotate`（参数为：源变换矩阵（这里就为 $diag(1)$，因为没有更下层的变换了），旋转角度，旋转轴）得出**模型空间 --> 世界空间**变换矩阵。
* 通过`glm::lookAt`（参数为：摄像机位置，观察点位置，”上“定义）得出摄像机的**世界空间 --> 观察空间**变换矩阵。
* 通过`glm::perspective`（参数为：FOV，长宽比 aspect ratio，近裁剪平面 near clip plane，远裁剪平面 far clip plane）得出**观察空间 --> 投影空间**的透视变换矩阵。

示例代码如下：

```c++
ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
```

由于 GLM 是给 OpenGL 设计的，所以其默认的裁剪空间 y 轴是与 Vulkan 相反的，这里使用 `ubo.proj[1][1] *= -1` 再给它反回来。具体区别可以参考 **概念汇总** 一章中的 **不同图形 API 之间的区别** 子章节。

算出所有变换矩阵后，使用和之前一样的方法配置内存映射，并将值 `memcpy` 到映射的内存中：

```c++
void* data;
vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
memcpy(data, &ubo, sizeof(ubo));
vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
```

> 注意，使用 UBO 来更改每帧都在变化的变换矩阵其实并不是一个最优方法。最好的方法是使用 `push_constants` 来将数据量小但变化频繁的 uniform 变量提交到 shader 中，例如本例的变换矩阵们。这里的“数据量小”概念可以通过查看实例中的 `VkPhysicalDeviceLimits::maxPushConstantSize` 成员变量值。Vulkan 要求最小值为 128 字节，而当前主流显卡一般也就有其两倍大（本机的 GTX 1070 和 UHD 630 均为256字节）。通过使用 push constants，向 Shader传递内容可以绕过复杂的 buffer 分配环节，**因为 push constants 其实就是流水线的一部分**。



### 数据对齐

C++ 客户端的 UBO 布局要遵循 Vulkan 的对齐标准才能将有效的信息传递给 Shader，具体定义可以在 Vulkan 官网上查到，这里 GLM 已经帮我们进行二进制兼容了。同时，其数据起始地址要是 16 字节的倍数，所以要使用 `alignas(16)` 对齐，例如：

```
struct UniformBufferObject {
    alignas(16) glm::vec2 foo;
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};
```

GLM 在设计的时候也考虑到了这一点。在 include 之前定义 `GLM_FORCE_DEFAULT_ALIGNED_GENTYPES` 宏来强制 GLM 帮我们进行数据的 16-byte 对齐。但当在遇到嵌套结构体时（Shader也可以对应的用结构体哦）还是需要手动对齐数据。

std140 的对其规范可以从 OpenGL Programming Guide 中找到（图摘自 OReally 官网）：

![](https://www.oreilly.com/library/view/opengl-programming-guide/9780132748445/graphics/app09tab01.jpg)



### 配额限制 (Limits)

虽然本例使用的 UBO 算是相当轻量级的了，但正式工程中在使用 UBO 传数据之前也需要了解一下 UBO 在 Vulkan 规范和系统显卡上的空间/数量配额限制。常用的有下面几个：

* `maxUniformBufferRange`

  在将 UBO "write" 进资源描述时（见上一章），`VkDescriptorBufferInfo` 类型参数中 `range` 的最大值，说白了就是 UBO 最大大小。笔者机器上是 65536 字节。

* `maxPerStageDescriptorUniformBuffers`

  一个 Shader 阶段能 binding 多少个 UBO。笔者机器上是 15 个。

* `maxDescriptorSetUniformBuffers`

  一整个流水线能 binding 多少个 UBO。笔者机器上是 180 个。

* `minUniformBufferOffsetAlignment`

  在将 UBO "write" 进资源描述时，`VkDescriptorBufferInfo` 类型参数中 `offset` 的最小值，说白了就是写资源描述时 UBO 的最少偏移。笔者机器上是 256 字节（0x00000100）。

细节请参阅 `VkPhysicalDeviceLimits` 的官方文档或者使用随 Vulkan SDK 自带的软件 Vulkan Configurator，当然也可以通过写个程序自己查这些，就是累点儿不是么。

  