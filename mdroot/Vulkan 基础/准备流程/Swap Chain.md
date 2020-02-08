## Swap Chain

流程方法名：`createSwapChain()`

> the general purpose of the swap chain is to synchronize the presentation of images with the refresh rate of the screen.

之前提到了，整个流程涉及到两个工序：渲染和呈现，分别由渲染器和呈现引擎（presentation engine）所控制。渲染器负责做计算来渲染图像，而呈现引擎则负责在每一个显示器的 Vblank 向显示器或者另外一个程序的缓冲区（例如 DWM，然后其可能在下一个 vblank 再把你的画面输出到显示设备上）输送渲染好的画面，然后把这个画面（已经没用了）来回收，类似一个环状队列。想象一个工厂的生产和装箱，生产好的物品放在箱子里运输出去，里面东西用完了再把箱子收回来。渲染器和呈现引擎通过一个队列来交换数据，这个队列就叫做 swap chain。

呈现器的实现与操作系统的实现高度相关。Swap Chain 里面的缓冲区可以有多个，但数量是固定的，不可动态插入/删除。

具体解释可以看 **概念汇总** 一章。

### 配置参数

到这里，Swap Chain肯定被设备所支持，所以接下来的任务就是要找出对于**这个教程**最合适的参数：

* 表面格式

  现在要从之前验证设备时获取到的众多`VkSurfaceFormatKHR`中获取到最合适的一种。这里选择`VK_FORMAT_B8G8R8A8_UNORM`作为教程所使用的色彩深度（8-bit BGRA 编码，每个通道使用 IEEE UNORM float），以及`VK_COLOR_SPACE_SRGB_NONLINEAR_KHR` 作为色彩空间（sRGB）。

  如果在之前`vkGetPhysicalDeviceSurfaceFormatsKHR`的时候得到的`VkSurfaceFormatKH.format`结果是`VK_FORMAT_UNDEFINED`，那么恭喜，显卡没有任何限制，可以直接选，否则就要从给出的格式中挑选出上述需要的格式。

* 显示模式

  显示模式决定了呈现引擎如何将 Swap Chain 中的 Image 轮换着在屏幕上显示。

  现在要从之前验证设备时获取到的众多`VkPresentModeKHR`中获取到最合适的一种。其中可选择的有：

  * `VK_PRESENT_MODE_IMMEDIATE_KHR`：不加缓冲，画面渲染好了直接开始输送，可能会因为还没到 Vblank 就输送了一张新的图像导致同一时间有两张图像被呈现，导致“撕裂”。有些 GPU 和操作系统不支持这个模式。
  * `VK_PRESENT_MODE_FIFO_KHR`：类似于其它 API 的 Double Buffering，使用一个队列保存渲染好的 Image，Vblank 时取队首 Image 准备呈现，copy 到呈现设备后回收供渲染器再次写入。这样做可以避免撕裂。由于队列是有固定长度的，所以如果渲染速度贼快导致队列满了，渲染这边就要不到新的 Image 用来渲染，从而导致等待（Hang），同时队列里的 Image 因为并没有更新导致该图像的生成时间和当前用户的输入时间差比较大，造成用户的输入延迟。兼容 Vulkan 的显卡必须支持这个模式。
  * `VK_PRESENT_MODE_FIFO_RELAXED_KHR`：和 FIFO 差不多，但 FIFO 状态下，如果性能不足没来得及渲染一整张画面导致队列为空，但 Vblank 来了，则呈现引擎会再度把之前呈现的画面再呈现一次，导致延迟更大了。这个模式下 Vblank 时队列空则直接（IMMEDIATE）输出，当性能跟不上刷新率时会导致撕裂，也算是一种比较好的妥协。
  * `VK_PRESENT_MODE_MAILBOX_KHR`：类似于 Triple Buffering。这里不再使用 FIFO 的概念，而是每次渲染完毕时直接拿走队尾里最旧的那个 Image 来回收，然后把新的 Image 放进队首。呈现引擎拿走队首的，避免撕裂同时减小延迟，但会浪费掉很多性能，在移动平台等需要 efficiency 的地方要慎用。

  这里首选 MAILBOX，如果不行则回退到 FIFO。

* 图像分辨率

  返回一个`VkExtent2D `（`struct { width, height }`）

* 图像数量

  图像数量和选择的显示模式有关，但要多了也不会怎么样。一个经验主义的设置方法就是比 `VkSurfaceCapabilitiesKHR` 实例里的 `minImageCount` 多要一个。

### 生成Swap Chain

使用上面的参数就可以配置一个`VkSwapchainCreateInfoKHR `结构体实例了。同时还需要配置结构体中：

* `imageArrayLayers `如果不需要VR等双屏渲染情形时就设置为1。VR需要多个Layer。

* `imageUsage`一栏若直接渲染到屏幕则使用`VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT`，若要做post-processing则用`VK_IMAGE_USAGE_TRANSFER_DST_BIT`

* 如果上述graphics和present所使用的queue不是一个queue，图像将会涉及到跨queue family的共享操作。可用的操作如下：

  * `VK_SHARING_MODE_EXCLUSIVE`：一个队列类型（Graphics）独占，显式传送给另一个（Present）队列。这种方法性能最好。
  * `VK_SHARING_MODE_CONCURRENT`：共享模式
  
若不是一个queue，这里考虑到简洁性采用共享模式。
  
* `clipped`：是否不渲染被其他窗口遮挡住的pixel

* `oldSwapChain`：当 SwapChain 被改变时（改变窗口大小），这里传入老交换链，以供参考。

创建成员变量 `VkSwapchainKHR swapChain` 并用 `vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain)` 创建出来。在程序最后的 cleanup 阶段也需要用`vkDestroySwapchainKHR` 进行销毁。

### 获取Swap Chain中的图像

创建成员变量来容纳交换链中的`VkImage`：`std::vector<VkImage> swapChainImages;`

使用 `vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());` 获取图像的 Handle。

接下来，获取 SwapChain 图像中的属性以便后续使用：

```c++
VkFormat swapChainImageFormat = surfaceFormat.format; // VK_FORMAT_B8G8R8A8_UNORM
VkExtent2D swapChainExtent = extent; // 800 x 600
```
