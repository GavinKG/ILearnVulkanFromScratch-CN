## Staging Buffer

### 用途

由于硬件限制，很多时候提供一片同时能被GPU和CPU读写的内存（both device local and host visible）不是一件容易的事情，这尤其体现在与独立显卡的交互过程中：CPU不得不在背后将所有数据从PCI-E总线发到GPU端，这比起直接访问内存要慢很多。

为了优化这一点，我们开辟一块和原始顶点内容相等的临时缓冲，将其内存设为共享（和旧式顶点缓冲一样），成为staging buffer。在应用了暂存缓冲后，顶点缓冲则可以成为一个使用GPU独占显存的缓冲区，GPU对其的读取速度要比从共享内存快。相比于每次在render loop的绘制过程都从旧式顶点缓冲的慢速内存中读取顶点数据，我们只在程序一开始通过命令队列进行一次转移（transfer）操作从暂存缓冲移动到高速的顶点缓冲，之后每个render loop显卡将都从高速顶点缓冲中取数据进行绘制，省时省力。

但需要注意的是，如果CPU-GPU带宽是瓶颈，可以选择采用staging buffer，但如果顶点数据经常变动（注意是本质上的变动，例如增加减少，坐标的变动可以通过变换矩阵或几何着色器解决）或只面对集成显卡、游戏主机、移动设备（SoC）等图形设备和图形内存（往往与CPU共享）的程序采取staging buffer + transfer queue则更可能是一种冗余的操作。所以说这一章节的内容需要斟酌考虑是否实现在项目中。

### 转移队列

为了实现从暂存缓冲（源，src）将顶点转移到高速顶点缓冲（目标，dst）中，这里需要使用到 transfer queue，也就是支持`VK_QUEUE_TRANSFER_BIT`的queue family。不过很巧的是，所有图像（Graphics）队列都支持转移操作，所以这里直接将转移命令提交到图像（Graphics）队列就好。当然，如果需要绘制的内容时常全部更改，转移操作很频繁，可以考虑专门新建一个transfer queue，这样Vulkan能够更好的安排队列的并行执行，从而优化程序性能。

### 使用暂存缓冲

在引入暂存缓冲时，我们需要：

* 创建一个暂存缓冲，用途为`VK_BUFFER_USAGE_TRANSFER_SRC_BIT`，其属性由于也要充当CPU与GPU沟通的桥梁，所以与之前创建的顶点缓冲一样，为`VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT`。
* 创建一个只GPU可见的顶点缓冲，用途为`VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT`，属性为`VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`。

数据的转移操作也需要通过一个新的command buffer来执行。在本例中，由于转移操作只进行一次，所以这个command buffer只声明一次就需要被废弃掉。为了优化命令缓冲池的内存分配，可以为其专门开辟一个内存池，并使用`VK_COMMAND_POOL_CREATE` flag。

声明一个`copyBuffer`函数，负责两个缓冲之间的拷贝。步骤如下：

- 声明一个 command buffer。
- 填充`VkCommandBufferBeginInfo `结构体（见上），其中由于这个command buffer只提交一次，所以`flags`参数指定为`VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT。`
- 开始录制。
- 开始拷贝缓冲。使用`VkBufferCopy`设置源/目标缓冲的起始偏移量和拷贝大小，并使用`vkCmdCopyBuffer`录入移动命令。
- 停止录制。
- 提交 command buffer：`vkQueueSubmit。`
- 使用`vkQueueWaitIdle`阻塞等待队列执行完毕。
- 使用`vkFreeCommandBuffers`释放掉这个短命的command buffer。

对拷结束之后，在本例中暂存缓冲的用处也就没有了。使用 `vkDestroyBuffer` 和 `vkFreeMemory` 释放掉这个同样短命的 staging buffer。
