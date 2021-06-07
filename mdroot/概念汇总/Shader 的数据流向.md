# Shader 的数据流向

https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)

## `Location`

* **在 Vertex Shader 中：**

  * `layout(location = x) in`

    流水线顶点阶段输入的数据，一般为顶点位置、法线、UV 坐标等顶点属性。

    其 `location = `  所指定的值通过流水线（pipeline）设置阶段的 `VkPipelineVertexInputStateCreateInfo::pVertexAttributeDescriptions` 所传入的 `VkVertexInputAttributeDescription` 数组中每一个元素的 `location` 指定。

  * `layout(location = x) out`

    流水线顶点阶段输出的数据，一般为顶点颜色、UV 坐标等变换后的顶点属性。注意顶点坐标被输出到了内置变量 `gl_Position` 了。

    其 `location = `  所指定的值和 Fragment Shader 中输入值对应即可，在客户端程序里不用做任何绑定。

* **在 Fragment Shader 中：**

  * `layout(location = x) in`

    流水线片元阶段的输入数据，一般为插值后的 UV 坐标，片元颜色等。

    其 `location = `  所指定的值和 Vertex Shader 中输出值对应即可，在客户端程序里不用做任何绑定。

  * `layout(location = x) out`

    流水线片元阶段的输出数据，一般为最终颜色，或为一系列延迟着色的 G-buffer attachment。

    其 `location = `  所指定的值和设定每个 subpass 所用到的 `VkSubpassDescription::pColorAttachments` 时传入的 color attachments 的引用数组（ `VkAttachmentReference colorReferences[N]`）索引相同，即该值所对应的输出会被写入该值索引下的 color attachment。

    注意，虽然感觉上输出颜色值是必需的，但是由于 Vulkan 已经原生支持 MRT，所以在 GLSL 中原本存在的 `gl_FragColor` 已经被废弃。使用 `layout(location = 0) out vec4 fragColor;` 来输出颜色吧。

  

## `Binding`



## `push_constants`



## `constant_id`



## 内置变量

https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)