## Subpass 初步

<https://www.saschawillems.de/blog/2018/07/19/vulkan-input-attachments-and-sub-passes/>

### Subpass 和 Input Attachments

传统的 multi-pass 系统中每个 pass 结束时会渲染出一系列独立的图像，即 attachment image views，供下一个 pass 采样（比如说 combined image sampler）。由于对一个 sampler 采样不受任何限制，所以我们可以在 fragment shader 中对邻近的像素进行采样，从而实现一些复杂的后处理特效（所有的 kernel-based 特效都需要采样临近像素）。当然有些时候，对于一个 framebuffer 来说，一个像素计算好的数据只会被下一个pass的同一个位置的像素利用，例如延迟着色时的 G-buffer 光照计算，以及一些不太复杂的特效，比如说亮度对比度等。

但 multi-pass 也有着性能的问题：一个 pass 渲染好的 image 要被统一存到一个 Framebuffer 中，随后在第二个 pass 中再被分散到不同的 shader 中去，这对于 tile-based renderer 平台是致命的：对于上述提到的那些”只会被同一个位置的像素所利用“的适用情形，这些像素本可以在每个 tile 的高速片上内存直接被下一个 pass 使用，但由于要统一存放在 VRAM 的 framebuffer 中，每个 tile 的片上内存数据一直在被收集-分散-收集-分散，tile memory 几乎起不到任何作用，极大的浪费了性能，加大了数据传输所导致的带宽消耗（在移动端直接带来一个致命问题：发热降频），完全违背移动端 TBR 架构的初衷。

subpass 就解决了这个问题。后一个 subpass 所使用的 attachments 的像素可以直接从前一个 subpass 的 attachments 中的对应位置读取，从而免去了收集的过程。当然需要注意的是，下一个 subpass 不能对临近像素进行采样了，只能使用同一位置的像素信息。这些 attachments 便被称为 input attachments。

> Input attachments are image views that can be used for pixel local load operations inside a fragment shader. This basically means that framebuffer attachments written in one subpass can be read from at the exact same pixel (that they have been written) in subsequent subpasses.

在该案例中，我们在第一个 subpass 渲染出两个 input attachments：一个 color attachment 用来记录颜色，以及一个 depth attachment 用来记录深度。在第二个 subpass 我们将上述两个 input attachments 进行采样，输出处理后的结果到 swap chain image attachment，如下图所示：

![img](https://www.saschawillems.de/images/2018-07-19-vulkan-input-attachments-and-sub-passes/Unbenannt-1-2.png)

### Framebuffer 创建

首先我们要为这三个 attachments 准备 framebuffer。和之前一样，为每一个 swap chain image 准备一个含有三个 image view 的 framebuffer，分别为：

0. Swap chain image view
1. Color Attachment
2. Depth Attachment

### Subpass 声明

之后开始描述这两个subpass，步骤如下（很多都和之前介绍的相同，这里再温习一下）：

* 填充 `std::array<VkAttachmentDescription, 3> attachments{}` 。

* 填充 `std::array<VkSubpassDescription,2> subpassDescriptions{}`。其中每个 subpass description 需要一个 `VkAttachmentReference` （Attachment reference to the Framebuffer）作为其 attachments 的引用。第一个 subpass 需要索引为 1 的 color attachment 以及索引为 2 的 depth attachment，该索引即为**创建 framebuffer 时数组的索引**（`VkFramebufferCreateInfo::pAttachments`）；第二个 subpass 需要索引为 0 的 color attachment，同时需要指定 input attachment：

  ```c++
  VkAttachmentReference inputReferences[2];
  inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
  inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
  subpassDescriptions[1].inputAttachmentCount = 2;
  subpassDescriptions[1].pInputAttachments = inputReferences;
  ```

  注意之后引用 subpass 也将使用索引，其值为 `VkRenderPassCreateInfo::pSubpasses` 数组的索引值。

* 描述 subpass dependencies：`EXTERNAL -> 0 -> 1 -> EXTERNAL`，构建依赖并实现隐式 layout transitions。

### Descriptor Sets

接下来开始描述两套 descriptor sets。由于用到了 input attachments，我们就要在其中加入类型为 `VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT` 的 descriptor。与之前一样，这一套流程为：

* descriptor pool
* descriptor set layout 和 pipeline layout：两套
* descriptor：在这里使用的实际上是 image descriptor `VkDescriptorImageInfo`，但并没有在里面指定 sampler（`VK_NULL_HANDLE`），因为上面原理部分提到了，input attachments 并不能被采样，而只能访问之前同位置的像素。
* descriptor sets：将 color input attachment、depth input attachment 以及控制显示哪个的 UBO write 进 第二个 subpass 用到的那组 descriptor set，第一个 descriptor sets 和往常一样。

注意，在 render pass 设置环节和 descriptor sets 设置环节都有对 input attachments 的设置。

### Pipeline 创建

我们要为这两个 subpass 创建两套不同的 pipeline（当然大多数属性是一样的）。除了需要指定不同的 shader 外，也需要为这两套 pipeline 指定对应的 subpass 和 pipeline layout。

### Command Buffer 的录制

在 Command Buffer 中录制绘图命令时，默认从开启的 render pass （`vkCmdBeginRenderPass` 指定的 pass）中的第 1 个 subpass （索引为 0 ）开始录制。在完成绑定（流水线、顶点索引、descriptor sets）并渲染过后，需要手动调用 `vkCmdNextSubpass` 来根据索引顺序切换到下一个 subpass。第二个subpass只需要录制一个覆盖全屏的 quad 即可（为了使得屏幕上所有像素都有且只有一次被调用 fragment shader）。

### Shader

第一个 subpass 所对应的顶点和片元着色器和之前一样正常绘制即可。

第二个 subpass 所对应的顶点着色器简单 pass-through 即可。在第二个 subpass 的片元着色器中，使用 `input_attachment_index`  编号来使用 input attachment（剩下的属性，诸如 `set` 和 `binding` 和之前一样）。该编号和之前在创建 render pass 阶段的 `subpassDescriptions[1].pInputAttachments` 所传入的数组的索引相同。不要忘记 shader 中的 `subpassInput` 关键字。

```glsl
layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth;
```

注意这里指定了两个看似可以互相替代的编号来索引到一个 input attachment，但不要忘记，在非 TBR 的设备上（比如传统的桌面显卡），input attachments 还是使用了传统的贴图采样。具体原因可以参考 <https://stackoverflow.com/questions/43632903/why-do-input-attachments-need-a-descriptor-set-to-be-bound?rq=1>

之后使用 `subpassLoad` 代替 `texture` 从 input attachment 进行像素颜色值的获取。

```glsl
vec3 color = subpassLoad(inputColor).rgb;
```

