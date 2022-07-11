##  Subpass 实战：延迟渲染

上面提到了，subpass 在 Deferred Renderer 中能够利用起绝大多数移动 GPU 上带的片上高速缓存，起到很大的优化作用，因此这里使用 subpass 实现一个非常简单的延迟着色过程，将几何和光照解耦合。

需要注意，行业内 Deferred Renderer 真正使用的 G-Buffer 往往有着复杂很多的设计（PBR / Material ID 等等额外的 Buffer），同时在移动设备上也大量使用了  TBDR 做光源剔除，因此此处的 G-Buffer 遵循教程，仅限演示，不具备实际实用意义。

该案例中 subpass 流程图如下：

![img](https://www.saschawillems.de/images/2018-07-19-vulkan-input-attachments-and-sub-passes/subpasses.png)

三个 subpass 功能如下：

1. 渲染 G-buffer，依照上图顺序由三个 color attachment：albedo 颜色，世界空间位置，世界空间法线和一个 depth/stencil attachment 构成。注意 G-buffer 传出的数据一定要在同一空间坐标系中，该案例使用世界空间坐标系。
2. G-buffer Composition：依照 G-buffer 渲染光照并合成到最终颜色 attachment。
3. Transparency：依照 depth attachment 渲染半透明物体，叠加到最终颜色 attachment。

相关数据准备如下：

* Framebuffer：含有 5 个 attachment image view，分别为：最终颜色、位置、法线、albedo、深度；

* Pipeline：三套流水线，对应 3 个 subpass；

* Render Pass：5 个 attachment，3 个 subpass；

* Command Buffer：和 subpass 初步一样，分别针对每一个 subpass 进行 binding 操作，使用 `vkCmdNextSubpass` 切换 subpass；

* Descriptor Sets：

  第一个 subpass 和具体渲染有关，这里只传入 MVP 变换矩阵。

  第二个 subpass 和 G-buffer 合成有关，传入 3 个 input attachments，以及多个点光源的信息数组（位置、颜色、衰减）和视点位置的 UBO。

  第三个 subpass 渲染透明物体，传入 MVP 变换矩阵，1 个 input attachment，以及半透明玻璃所使用的纹理。

* shader：

  * G-buffer VS：正常进行坐标变换即可。输入 MVP 矩阵和顶点数据，除了输出 `gl_Position` 以外还要输出要写入 G-buffer 的 attachments 的内容，分别为位置、法线和漫反射颜色（albedo），为了方便计算光照，坐标系空间均为世界空间。

  * G-buffer TS：这里不做任何渲染操作，但需要把这些元数据写入多个 attachments 中。注意，由于 depth buffer 不能当作 input attachment，所以这里为了方便之后的流程用深度值，直接手动将深度值归一化并写入位置 attachment （索引为 1）的 alpha 通道。

  * 合成 VS：这个覆盖全屏的 quad 在客户端定义，因此这里使用 pass-through shader，即什么都不干。

  * 合成 TS：光照在这里渲染。采样 G-bufffer attachments 并使用一个 for loop 对每一盏灯的贡献颜色进行计算，最后输出最终加一起的颜色。

  * 前向透明 VS：正常进行坐标变换即可。

  * 前向透明 TS：由于在渲染透明物体时不应该对之前渲染不透明物体时的 depth buffer 进行改动（半透明物体后面的半透明物体也要渲染），因此在设置 pipeline 的时候开启了 color blending（`VK_BLEND_OP_ADD`），并且没有在 subpass 中绑定 depth buffer 来避免改动，而是从位置 attachment 的 z 分量获取深度值并手动判断是否 discard：

    ```glsl
    float depth = subpassLoad(samplerPositionDepth).a;
    if ((depth != 0.0) && (linearDepth(gl_FragCoord.z) > depth)) discard;
    ```

    注意该代码中渲染透明物体并没有进行排序。


