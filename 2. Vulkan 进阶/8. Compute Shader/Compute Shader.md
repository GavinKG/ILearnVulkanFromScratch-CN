# Compute Shader

参考 OpenGL wiki：https://www.khronos.org/opengl/wiki/Compute_Shader，介绍此处略去。

不同于一个传统的 Shader，一个 Compute Shader 没有定义好的输入输出（`location`），而是接收一些资源，例如 SSBO，UBO，Texture 等，处理后自己写回去。

Compute Shader 最高工作在三维 **Compute Space**，即一个**抽象的计算空间**，每个对应的空间坐标为一个 **Work Group**。Work Group 的大小需要客户端指定；一个 Work Group 中在执行中会调用一定数量的 Compute Shader，这个数量被成为 **Local Size**，该值是在 Compute Shader 代码内部定义的。同样，Local Size 最高也是三维。

Work Group 之间不应该有任何数据依赖（如果需要互相交换数据效率也很低），而且 Work Group 的并行度和执行顺序没有任何保证，最终只会保证所有的都执行了而已。但每个 Work Group 中根据 Local Size 指定的诸多 Compute Shader 实例是**并行执行的**，同时这些实例可以通过 shared variables 和一些方法互相传递数据。Wiki 中给了个例子：用 8x8 的块来压缩一张二维纹理，Local Size 即可以指定成 8x8x1，用来对一个块进行压缩处理；块和块之间不需要交换数据，即独立压缩，因此 Work Group Size 即为整个图片分辨率 / 块分辨率。

