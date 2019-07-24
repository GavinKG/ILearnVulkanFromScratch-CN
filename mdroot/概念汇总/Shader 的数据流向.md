# Shader 的数据流向

## `Location`

在 Vertex Shader 中：

* `layout(location = x) in`

  流水线顶点阶段输入的数据，一般为顶点位置、法线、UV 坐标等。

  其 `location = `  所指定的值通过流水线设置阶段的 `VkPipelineVertexInputStateCreateInfo::pVertexAttributeDescriptions` 所传入的 `VkVertexInputAttributeDescription` 数组中每一个元素的 `location` 指定。

* `layout(location = x) out`

  流水线顶点阶段输出的数据，一般为变换后的投影坐标、顶点颜色、UV 坐标等。

  其 `location = `  所指定的值和 Fragment Shader 中输入值对应即可，在客户端程序里不用做任何绑定。

在 Fragment Shader 中：

* `layout(location = x) in`

  流水线片元阶段的输入数据，一般为插值后的 UV 坐标，片元颜色等。

  其 `location = `  所指定的值和 Vertex Shader 中输出值对应即可，在客户端程序里不用做任何绑定。

* `layout(location = x) out`

  流水线片元阶段的输出数据，一般为最终颜色，或为一系列延迟着色的 G-buffer attachment。

  其 `location = `  所指定的值和设定每个 subpass 所用到的 `VkSubpassDescription::pColorAttachments` 时传入的 color attachments 的引用数组（ `VkAttachmentReference colorReferences[N]`）索引相同，即该值所对应的输出会被写入该值索引下的 color attachment。

  注意，Vulkan 中的 subpass 只能使用多个 color attachments（即语义层面支持 Multiple Render Targets，完全废弃了 `gl_FragColor`），而没有多个 depth / stencil attachment，而且深度值在执行 Fragment Shader 之前的提前深度测试中已经写完。

## `Binding`

## `push_constants`

## `constant_id`

## 内置变量

