## GPU 粒子系统初步

在该案例中，GPU 粒子系统使用 Compute Shader 参考传入的 UBO 中的信息模拟粒子的行为并将粒子信息存在设备专属的 SSBO 中，并在绘图时直接使用这个 Buffer 作为顶点属性缓冲将顶点绘制出来。在介绍整套系统的流程时，我们首先讨论一种新的缓冲类型：SSBO，进而讨论计算队列和图像队列的同步机制，最终描述该案例的粒子系统架构。



### SSBO

SSBO，即 Shader Storage Buffer Object，一般也直接称之为 Storage Buffer，也是 Buffer 对象的一种，其主要作用是供 Shader 对其进行读写。作为一个一次初始化，多次 GPU 使用的 Buffer，其创建方法和之前涉及到的 Buffer 并没有区别，都是先在本地 staging 再 copy 到这个仅设备可见的 Buffer 中。为了体现出其 Storage Buffer 的特性，在创建这个仅设备可见的 Buffer 时，`usageFlags` 要加上一个 `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT`，当然作为拷贝的目标缓冲，也得有 `VK_BUFFER_USAGE_TRANSFER_DST_BIT`。

和 UBO 一样，SSBO 也是通过 Descriptor 传入 Shader 内部，不要忘记指定类型为 `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER`。但不同于 UBO，SSBO 也有其独特之处：

* 更大：UBO 限制其 Buffer 大小不得超过 16 KB，但 SSBO 可以是 128 MB。具体可以通过查询设备 Limits 获得。
* 可写：UBO 在 Shader 中是 `uniform`，即只读；SSBO 可写，但需要注意数据同步和原子操作，在后续章节将会提到。
* 更慢：UBO 读取速度要比 SSBO 快。

具体细节可以参考 OpenGL Wiki：https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object

在该案例中，这个 Storage Buffer 存放的东西也正好可以被用来当成粒子的顶点属性来使用，即在 Command Buffer 中绑定顶点属性缓冲时会直接用这个 SSBO，因此在创建时的 `usageFlags` 中也要加入 `VK_BUFFER_USAGE_VERTEX_BUFFER_BIT`。这样就实现了 Compute Shader 写一个显存中的 Buffer 直接给图形那边用，CPU 不用参与任何粒子属性的更新，从而实现 GPU 粒子系统。

该案例中 Compute Shader 的 SSBO 声明如下：

```glsl
layout(std140, binding = 0) buffer Pos 
{
   Particle particles[ ];
};
```



### 计算和图形任务的同步

和之前一样，我们要保证在计算这边没算完顶点位置时，Vertex Shader 不去读（？）；Vertex Shader 执行完毕后，计算这边才开始继续改。因此我们要在 Storage Buffer 更改前和更改后都加上 Pipeline Barrier 保证 Buffer 只被一个队列“占有”，满足了这个 Buffer 修改的原子性。

不同于之前案例中采取的写后读（RAW），因为这里图形管线一开始即需要这些顶点属性，如果还保证写后读的话，计算所话费的时间就会使得图形流水线空闲很长时间。这里我们在图形流水线获取顶点时使其获取到上一次计算得到的值（或初始值），在图形流水线执行顶点着色器后面的阶段时让计算这边同时跟进。时序图如下（其中 [x] 和 [o] 代表一对 Barrier / 示信，精简了一些阶段）：

```
Graphics *|-----VS-----[x1]---Rasterize---FS---[x2]     [o2]------VS-----[x1] ...
Compute  *|[x1]        [o1]-----------Compute-----------[x2][x1]         [o1] ...
```

采用之前案例的同步方法，可以看到图形流水线 stall 了很长时间，即使有 Barrier 提示符占空间也比上面慢很多。两个流水线几乎串行交叉执行，失去了 Async Compute 的初衷，并且因为同步还会损失性能。

```
Graphics *|[x]                          [o]-----VS-------Rasterize---FS---    ...
Compute  *|-----------Compute-----------[x]
```



### 粒子系统

该案例视频：https://www.youtube.com/watch?v=7QJOf7N7OdU

下面结合着这个案例说说粒子系统本身。每一个粒子的属性，即 SSBO 每一分块的结构如下：

```
struct Particle {
	glm::vec2 pos;				// 粒子2D位置，直接是 Vertex Shader 输出的裁剪坐标系，每分量范围 [-1, 1]
	glm::vec2 vel;				// 粒子2D运动矢量
	glm::vec4 gradientPos;  	// 粒子颜色
};
```

每一个 Compute Shader 根据 `gl_GlobalInvocationID` 获取到属于自己的属性。因为这个结构体要被客户端、计算部分和图形部分共同使用，因此这里使用 `std140` 布局保证格式统一和硬件无关。这些粒子的数量和每个粒子的位置、颜色都在客户端使用随机数引擎初始化并写入 staging buffer 中随后 copy 到我们的 SSBO 中。

同时，我们也要为粒子共同享有的属性创建出一个 UBO。GPU 粒子系统一般使用 UBO 传递全局粒子属性，例如粒子生命周期、障碍物、重力、引力点位置等。一些比较复杂的属性，例如一个 mesh 障碍物甚至可以通过工具纹理采样器的形式传入 Compute Shader 中。该案例的 UBO 结构如下：

```
struct computeUBO {							
	float deltaT;			// 即帧间时间 delta time，实现帧率无关动画
	float destX;			// 这两个值用来设置全局“引力点”的位置
	float destY;			// 每个粒子根据自身位置和引力点的位置根据万有引力定律计算力和加速度，结合vel算出速度，结合dT算出位移
	int32_t particleCount;  // 全局粒子数量。如果 gl_GlobalInvocationID > 这个值就不让其写 SSBO 防止越界
} ubo;
```



### 绘制流程

结合上该案例中粒子系统的概述和同步方法，绘制流程显得比较简单。构建 Compute Command Buffer 时，不要忘记在 dispatch 命令前后加上 Pipeline Barrier 来保证同步。在 draw loop 中提交到计算队列后，也不要忘记加上 Fence 保证 Compute 提交完毕以后再开启下一轮 draw loop。



### Compute Shader

首先在 Shader 中，别忘了使用 layout qualifier 声明 Local Size，UBO 和 SSBO。由于这里处理的不是图片而是数据，我们将不会使用三维坐标来定位到这个 Compute Shader 想要的数据，而是使用展开的 `gl_GlobalInvocationID` 作为 SSBO 中数组的下标定位到属于自己的粒子属性。之后结合着距离引力点的距离和一些公式计算该粒子的新位置。由于此处粒子位置使用的坐标系即 Vertex Shader 需要输出的剪裁坐标，为了不使得粒子飞出去，也做了一个触边反弹的机制。最终，**直接使用赋值语句**将结果写回 SSBO 对应下标即可。



### 总结

开启 Async Compute，使用 GPU 恐怖的并行运算能力来吞吐 13,107,200‬‬ 个粒子，即千万数量级的粒子进行运算和绘制，在笔者电脑上帧率都能达到很可观的 30 FPS，可见 GPU 粒子系统的潜力。随着图形硬件越来越通用化，很多并行且复杂的粒子行为逻辑（跳转）也可以被移到 GPU 上来做，留给 CPU 更大的运算空间去做游戏逻辑相关的工作。

其实，如果 Compute 和 Graphics 都使用通用队列的话，帧时间也仅仅增加了一点点，几乎可以忽略不记。由于 Compute 和 Graphics 任务分配到硬件时很多时候都在使用硬件的同一部分，再加上图形硬件的乱序执行能力，同时再考虑到队列之间资源同步的开销，Async Compute 的优势需要在非常繁重且特定的计算任务下才能显现出来。具体说明和参考资料可以在概念汇总中“队列”一章找到。