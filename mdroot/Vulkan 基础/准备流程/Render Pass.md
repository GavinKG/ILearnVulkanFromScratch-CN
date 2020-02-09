## Render Pass

流程方法名：`createRenderPass()`

Render Pass 定义为一个渲染步骤，或者可以理解为一个渲染的大方向，就像一个车间一样。在该步骤中定义了渲染好的东西将会输出到何处，也就是下面的渲染目标 attachments（又称为 render targets，我还是比较喜欢这种旧的叫法），就像一个车间将会把里面的东西输送到一个传送带被打包到同一个集装箱中（举个例子...）。当然怎么样去渲染不归 render pass 管，下一节 pipeline 会提到。例如现在有一个有很多不同材质的球体的场景供渲染，那么这里就可以声明一个 render pass 给所有球用，因为毕竟对于每一个球体，其渲染结果都会输出到同一个 attachment 上，所以流程是一样的。

如果说之前提到的那些准备用的步骤大部分都可以套用于所有程序，那么从这里开始的所有步骤将会根据需要来进行高度定制，甚至在图形引擎中会有一套自动生成这些内容的调度系统。在该教程下，这里将创建一个 render pass 供渲染这个三角形。

### Attachments

上述提到的 attachments 可能包含渲染目标（颜色、深度、模板等）和一些输入（上一个subpass的渲染目标）即为资源附件（Attachments），或者可以理解为广义上的一个图像Image。但真正的资源存在 Framebuffer 里，一个 attachments 对象只是对这些资源的一些描述（descriptions）。

首先创建一些资源附件的描述，使用`VkAttachmentDescription `来记录附件的描述信息，包括格式（同SwapChain格式相同），采样数量（这里为1，即不开启多重采样），该附件渲染前后内容（包括颜色深度信息、模板信息）是否保留或清空，渲染前后图像如何布局。Layout 到底是什么后面会提到。

在这个教程中只需要一个存颜色的资源附件（存颜色的图），定义为 `VkAttachmentDescription colorAttachment`。

### Subpass

一个 Render Pass 可以有多个 subpass 存在。例如，每一个后处理特效都依赖与之前一个特效的 attachment，所以跑完所有特效就会有一系列的 pass，而如果这个特效着色的时候只用到了上一个特效的对应像素，则可以把它们设置为 subpass 并且打包成一个大的 pass。想象两个车间和其中的流水线（对于屏幕空间后处理特效来说，一个车间只有一条流水线，即把之前的图像拿过来，处理完了直接输出），如果下一个车间的所有流水线只用到上面的一个与其对应位置的流水线的产物，那为何不把这两个车间直接对接起来，让后一个车间的流水线拼到前一个车间流水线上，从而避免物品集散的时间损失呢？当然这里说的只是最理想化的情形，对于一些实现了 Tile-based renderer 并且带宽小的GPU（通常为移动GPU）来说这种方法比较有用，而对于桌面GPU来说，可能用subpass和直接用多个pass的性能是差不多的。

每一个子pass都会引用一个或资源附件，通过填写`VkAttachmentReference colorAttachmentRef`来设置，其中包括使用的资源附件的编号和其的布局（这里使用`VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL` ）。

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