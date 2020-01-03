## Swap Chain

流程方法名：`createSwapChain()`

> the general purpose of the swap chain is to synchronize the presentation of images with the refresh rate of the screen.

交换链可能包括多个后台缓冲区，它本质上是一个包括前台缓冲区的循环队列。因此，在OpenGL时代的默认Frame Buffer可以被Swap Chain替代

### 配置参数

到这里，Swap Chain肯定被设备所支持，所以接下来的任务就是要找出对于**这个教程**最合适的参数：

* 表面格式

  现在要从之前验证设备时获取到的众多`VkSurfaceFormatKHR`中获取到最合适的一种。这里选择`VK_FORMAT_B8G8R8A8_UNORM`作为教程所使用的色彩深度（格式），以及``VK_COLOR_SPACE_SRGB_NONLINEAR_KHR` 作为色彩空间。

  如果在之前`vkGetPhysicalDeviceSurfaceFormatsKHR`的时候得到的`VkSurfaceFormatKH.format`结果是`VK_FORMAT_UNDEFINED`，那么恭喜，显卡没有任何限制，可以直接选，否则就要从给出的格式中挑选出上述需要的格式。

* 显示模式

  显示模式决定了Swap Chain中的Image以何种方法轮换着在屏幕上显示。

  现在要从之前验证设备时获取到的众多`VkPresentModeKHR`中获取到最合适的一种。其中可选择的有：

  * `VK_PRESENT_MODE_IMMEDIATE_KHR`（直接输出撕裂）
  * `VK_PRESENT_MODE_FIFO_KHR`（类似于VSync，规定显卡必定支持）
  * `VK_PRESENT_MODE_FIFO_RELAXED_KHR`（队列空则直接输出，撕裂）（Adaptive VSync?）
  * `VK_PRESENT_MODE_MAILBOX_KHR`（用于实现三重缓冲）

  所以这里为了使用Triple-buffering选`VK_PRESENT_MODE_MAILBOX_KHR`，fallback到`VK_PRESENT_MODE_IMMEDIATE_KHR`，如果不支持的话则选择`VK_PRESENT_MODE_FIFO_KHR`（显卡驱动对这个模式支持不好(TBC)）

* 图像分辨率

  返回一个`VkExtent2D `（`struct { width, height }`）

* 图像数量

  理论上比`minImageCount`多一个，并且保证不超过`maxImageCount`就行

### 生成Swap Chain

使用上面的参数就可以配置一个`VkSwapchainCreateInfoKHR `结构体实例了。同时还需要配置结构体中：

* `imageArrayLayers `如果不需要VR等双屏渲染情形时就设置为1。VR需要多个Layer。

* `imageUsage`一栏若直接渲染到屏幕则使用`VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT`，若要做post-processing则用`VK_IMAGE_USAGE_TRANSFER_DST_BIT`

* 如果上述graphics和present所使用的queue不是一个queue，图像将会涉及到跨queue family的共享操作。可用的操作如下：

  * `VK_SHARING_MODE_EXCLUSIVE`：队列独占。若是一个queue可以采用这种方法
  * `VK_SHARING_MODE_EXCLUSIVE`：一个队列类型（Graphics）独占，显式传送给另一个（Present）队列。这种方法性能最好
  * `VK_SHARING_MODE_CONCURRENT`：共享模式

  若不是一个queue，这里考虑到简洁性采用共享模式。
  
* `clipped`：是否不渲染被其他窗口遮挡住的pixel

* `oldSwapChain`：当SwapChain被改变时（改变窗口大小）老交换链的参考

创建成员变量`VkSwapchainKHR swapChain`并用`vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain)`创建出来。在cleanup阶段也需要用`vkDestroySwapchainKHR`进行销毁

### 获取Swap Chain中的图像

创建成员变量来容纳交换链中的`VkImage`：`std::vector<VkImage> swapChainImages;`

使用`vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());`获取图像

获取SwapChain图像中的属性以便后续使用：

```c++
VkFormat swapChainImageFormat = surfaceFormat.format; // VK_FORMAT_B8G8R8A8_UNORM
VkExtent2D swapChainExtent = extent; // 800 x 600
```
