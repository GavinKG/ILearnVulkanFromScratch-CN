## Push Constants

在之前更新 uniform buffer 章节的时候提到，有时候一些经常变化的小数据，比如说变换矩阵、光源位置等，可以通过流水线中的 push constants 直接传入 shader，其直接从 data path 传入，从而可以避免分配缓存等更重的操作，节省性能开销。

需要注意，一个 shader 只能分配一个 push constants 块，并且这个块的大小极度受限（`maxPushConstantsSize`）。Vulkan 规定大小至少为 128 Bytes，在笔者的机器上为 256 Bytes。

由于 Push constants 是流水线的一部分，所以首先在设置 pipeline layout 阶段（包含 descriptor set layout 和 push constants）就要让流水线知道有 push constants 的存在。使用 `VkPushConstantRange` 声明设置 push constants 的用途和大小。在案例中我们使用其来传递6个光源的位置（占用 4x4x6 = 96 Bytes），所以 shader stage 指定为 `VK_SHADER_STAGE_VERTEX_BIT`。最后在使用 `pipelineLayoutCreateInfo` 提交 Pipeline Layout 的时候将上述结构体传入即可。注意此时只是在勾勒流水线的布局，并没有真正传入数据。

之后，在 command buffer 中使用 `vkCmdPushConstants` 命令来提交真正的 push constants 数据，即可完成传递。

在 shader 中，通过 `layout(push_constant)` 替代 binding 来接收 push constant，在此例中即为：

```GLSL
layout(push_constant) uniform PushConsts {
	vec4 lightPos[lightCount];
} pushConsts;
```

