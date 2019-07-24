## Render Pass

流程方法名：`createRenderPass()`

渲染步骤，或被俗称为“趟”（因为翻译都不准确所以这里一律用英文表示），表示在Vulkan中绘制命令和使用资源的划分。每一 pass 绘制一些图像内容，被称为 attachments（曾用名：render targets）。

### Attachments

上述提到的资源可能包含渲染目标（颜色、深度、模板等）和一些输入（上一个subpass的渲染目标）即为资源附件（Attachments），或者可以理解为广义上的一个图像Image。但真正的资源存在 Framebuffer 里，一个 attachments 对象只是对这些资源的一些描述（descriptions）。

首先创建一些资源附件的描述，使用`VkAttachmentDescription `来记录附件的描述信息，包括格式（同SwapChain格式相同），采样数量（这里为1，即不开启多重采样），该附件渲染前后内容（包括颜色深度信息、模板信息）是否保留或清空，渲染前后图像如何布局（Attachment：`VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`，用作SwapChain Image：`VK_IMAGE_LAYOUT_PRESENT_SRC_KHR`，用作贴图：`VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`）

在这个教程中只需要一个存颜色的资源附件（存颜色的图），称为`VkAttachmentDescription colorAttachment`

### 子pass

一个Render Pass可以有多个subpass存在。例如每一个后处理特效都依赖与之前一个后处理的FrameBuffer，所以就会有一系列的pass，而这里把他们设置为subpass并且打包成一个大的pass。

每一个子pass都会引用一个或资源附件，通过填写`VkAttachmentReference colorAttachmentRef`来设置，其中包括使用的资源附件的编号和其的布局（这里使用`VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` ）。

这个编号和fragment shader中的layout是对应的。例如我在shader里面有`layout(location = 0) out vec4 outColor`，那么这里的color attachment的`colorAttachmentRef.attachment`就要为0。

现在开始对子pass进行描述：使用`VkSubpassDescription `，包括子pass在流水线中的绑定点（这里是`VK_PIPELINE_BIND_POINT_GRAPHICS`，将来可以支持通用计算），颜色资源附件的个数和指针引用（这个例子中只设置了一个颜色资源附件，即为上述`colorAttachmentRef`，所以直接传该实例的指针就行）

### 子pass依赖

每个 subpass 的 image layout 依靠 subpass dependencies 来控制并关联。换句话说，这个依赖起到关联subpasses的作用。

需要注意的是，虽然在本例中只创建了一个subpass，但是在这个显式创建的subpass的前后还有隐藏（implicit）的subpass。

填写`VkSubpassDependency`，用来描述显式创建的subpass与之前隐式subpass的依赖关系。其中包括：

- `srcSubpass`：前一个subpass。这里使用`VK_SUBPASS_EXTERNAL`指定这个subpass前一个，也就是那个我们不知道是什么的隐式subpass。
- `dstSubpass`：后面的subpass。这里直接给出我们显式声明的、有且仅有一个的subpass的编号，也就是`0`。
  注意：`srcSubpass`值必须低于`dstSubpass`，否则subpass的依赖图便存在环了
- `dstStageMask`和`dstAccessMask`：告诉subpass在`dstAccessMask`的access操作到来的时候需要等待的`dstStageMask`阶段。这里我们需要在`VK_ACCESS_COLOR_ATTACHMENT_READ_BIT `和（按位或）`VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT`（读写颜色Image）时等待颜色输出阶段，即上述提到的`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`

### 创建Render Pass

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