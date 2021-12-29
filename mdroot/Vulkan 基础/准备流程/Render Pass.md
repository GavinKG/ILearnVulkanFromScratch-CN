## Render Pass

原文：**https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Render_passes**

流程方法名：`createRenderPass()`



Render Pass 定义为一个渲染步骤，或者可以理解为一个渲染的大方向，就像一个车间一样。在该步骤中定义了渲染好的东西将会输出到何处，也就是下面的渲染目标 Render Targets，就像一个车间将会把里面的东西输送到一个传送带被打包到同一个集装箱中（举个例子...）。当然怎么样去渲染不归 render pass 管，下一节 pipeline 会提到。例如现在有一个有很多不同材质的球体的场景供渲染，那么这里就可以声明一个 render pass 给所有球用，因为毕竟对于每一个球体，其渲染结果都会输出到同一个 attachment 上，所以流程是一样的。

> Render Pass 这个概念其实是现代图形 API（[Direct3D 12](https://docs.microsoft.com/en-us/windows/win32/direct3d12/direct3d-12-render-passes), Vulkan, [Metal](https://developer.apple.com/documentation/metal/customizing_render_pass_setup)）的概念。在过去，我们只需要将创建好的 Render Targets 绑定到 Pipeline 即可，例如 DX11 的 `OMSetRenderTargets`，或者 OpenGL 的 `glBindFramebuffer` 等。但对于那些支持片上缓存的硬件渲染器（TBDR）来说，它们在使用 Framebuffer 之前可能会先将显存里的纹理加载（Load）到片上缓存空间中，在渲染完毕之后写回（Store）到显存上，这对于使用 TBDR 的移动设备芯片的带宽是有一定挑战的，并且带宽消耗直接反映在发热上。针对这个问题，我们可以在 Render Pass 上指定 Render Target 是否读入/写回显存。对于那些渲染时不需要考虑这个 Render Target 之前结果的情景来说，可以选择不关心（`VK_ATTACHMENT_LOAD_OP_DONT_CARE`）该纹理在显存中的内容；那些渲染后也不需要写回显存的 Render Target（例如一个临时的深度缓存，其只负责辅助当前 Render Pass 进行深度遮挡），也可以设置为 `VK_ATTACHMENT_STORE_OP_DONT_CARE`，在该 Render Pass 结束后，缓存中的内容将可能被丢弃。当然对于那些均为 `DONT_CARE` 的，只会存在于片上缓存的纹理资源来说，可以将其设置为 Transient Attachments（`TRANSIENT_ATTACHMENT_BIT`），这个话题就不放在 Vulkan 基础章节讨论了。
> 当然，Render Pass 的设计初衷也不仅仅是为了优化 TBDR 的渲染效率。当驱动在渲染前事先知道了 Render Pass 的流程状态，其[可能可以进行一些优化](https://gpuopen.com/learn/vulkan-renderpasses/)。

如果说之前提到的那些准备用的步骤大部分都可以套用于所有程序，那么从这里开始的所有步骤将会根据需要来进行高度定制，甚至在图形引擎中会有一套自动生成这些内容的调度系统。在该教程下，这里将创建一个示例 render pass 供渲染这个三角形。



### Attachments

Attachments 即 Render Target，之后直接使用 Vulkan 的叫法了。Attachments 实在不知道怎么翻译的通俗一点，翻译成“资源附件”又太拗口，这里直接用英文。其代表一个渲染步骤最终要输出的地方。

首先创建一些 Attachment Description。使用 `VkAttachmentDescription` 来记录附件的描述信息，包括格式（同SwapChain格式相同），采样数量（这里为1，即不开启多重采样），该附件渲染前后内容（包括颜色深度信息、模板信息）是否保留或清空，渲染前后图像如何布局。Layout 到底是什么后面会提到。

在这个教程中只需要一个存颜色的 attachment（存颜色的图），定义为 `VkAttachmentDescription colorAttachment`。之后将会用到能够渲染多个 attachments 的 render pass，这种 pass 主要用来实现延迟着色技术（deferred shading / deferred rendering）。



### Subpass

一个 Render Pass 可以有多个 subpass 存在。例如，每一个后处理特效都依赖与之前一个特效的 attachment，所以跑完所有特效就会有一系列的 pass，而如果这个特效着色的时候**只用到了上一个特效的对应像素**，则可以把它们设置为 subpass 并且打包成一个大的 pass。想象两个车间和其中的流水线（对于屏幕空间后处理特效来说，一个车间只有一条流水线，即把之前的图像拿过来，处理完了直接输出），如果下一个车间的所有流水线只用到上面的一个与其对应位置的流水线的产物，那为何不把这两个车间直接对接起来，让后一个车间的流水线拼到前一个车间流水线上，从而避免物品集散的时间损失呢？当然这里说的只是最理想化的情形，对于一些实现了 Tile-based renderer 并且带宽小的GPU（通常为移动GPU）来说这种方法比较有用，而对于桌面GPU来说，可能用subpass和直接用多个pass的性能是差不多的。

每一个子pass都会引用一个 attachment，通过填写`VkAttachmentReference colorAttachmentRef`来设置，其中包括使用的资源附件的编号和其的布局（这里使用`VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` ）。

**这个编号和 Fragment Shader 中的 layout 是对应的。例如我在 shader 里面有 `layout(location = 0) out vec4 outColor`，那么这里的 color attachment 的`colorAttachmentRef.attachment` 就要为 0。**

现在开始对子pass进行描述：使用`VkSubpassDescription `，包括子pass在流水线中的绑定点（这里是`VK_PIPELINE_BIND_POINT_GRAPHICS`，将来可以支持通用计算），颜色资源附件的个数和指针引用（这个例子中只设置了一个颜色资源附件，即为上述`colorAttachmentRef`，所以直接传该实例的指针就行）



### Subpass 依赖

每个 subpass 的 image layout 依靠 subpass dependencies 来控制并关联。换句话说，这个依赖起到关联subpasses的作用。

需要注意的是，虽然在本例中只创建了一个subpass，但是在这个显式创建的subpass的前后还有隐藏（implicit）的subpass。

填写`VkSubpassDependency`，用来描述显式创建的subpass与之前隐式subpass的依赖关系。其中包括：

- `srcSubpass`：前一个subpass。这里使用`VK_SUBPASS_EXTERNAL`指定这个subpass前一个，也就是那个我们不知道是什么的隐式subpass。
- `dstSubpass`：后面的subpass。这里直接给出我们显式声明的、有且仅有一个的subpass的编号，也就是`0`。
  注意：`srcSubpass`值必须低于`dstSubpass`，否则subpass的依赖图便存在环了
- `dstStageMask`和`dstAccessMask`：告诉subpass在`dstAccessMask`的access操作到来的时候需要等待的`dstStageMask`阶段。这里我们需要在`VK_ACCESS_COLOR_ATTACHMENT_READ_BIT `和（按位或）`VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT`（读写颜色Image）时等待颜色输出阶段，即上述提到的`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`

怎样玩转 Subpass 来增加性能将会在 **Vulkan 进阶** 一章中提到。这里因为就需要一个 subpass 所以直接走流程了。



### 创建 Render Pass

声明成员变量`VkRenderPass renderPass`来记录这里要用到的这一个pass。

通过填充`VkRenderPassCreateInfo `来设置：

```c++
VkRenderPassCreateInfo renderPassInfo = {};
renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
renderPassInfo.attachmentCount = 1;
renderPassInfo.pAttachments = &colorAttachment; // "resource description"
renderPassInfo.subpassCount = 1;
renderPassInfo.pSubpasses = &subpass;
renderPassInfo.dependencyCount = 1;
renderPassInfo.pDependencies = &dependency;
```

通过`vkCreateRenderPass`创建一个Render Pass并存在`renderPass`中。

需要注意的是，设置了那么半天 Render Pass，具体 attachment 都是谁并没有进行绑定。可以认为这么多工作只是在描述一个蓝图，而具体蓝图所对应的输入输出**需要在后面录制 Command Buffer 时才真正去进行绑定**。这种描述蓝图的性质也代表了声明出来的 Render Pass 具有通用性，可以适度进行复用。



### 代码实现

```cpp
    void createRenderPass() {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
```

