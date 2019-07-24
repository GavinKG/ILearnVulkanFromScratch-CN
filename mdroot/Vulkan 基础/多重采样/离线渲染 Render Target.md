## 离线渲染 Render Target

多重采样渲染出的图像并不会直接能够被没有开启多重采样的 swap chain image 直接使用，所以这里不能再把渲染出的图像直接输出到 swap chain image 中了，而是先输出到一个离屏的开启多重采样的 render target 中，该图像存在着所有的采样点的颜色数据和深度数据，再将这个图像进行采样点解析（Resolve MSAA），得出一个混合好的没有开多重采样的图像用于展示。

首先为了这个 render target 建立成员变量三大件：

```c++
VkImage colorImage;
VkDeviceMemory colorImageMemory;
VkImageView colorImageView;
```

不要忘记在 swap chain 的清理阶段销毁掉。

在建立贴图和深度 Image 的阶段，我们同时开始建立这个支持多重采样的 color attachment：

* 长宽：均为屏幕大小，即 `swapChainExtent`
* mipmap：一个 render target 不会用来当普通的纹理，所以直接给1，即不开启mipmapping。
* usage：由于要当作第一阶段的 color attachment，并且第二阶段结束就不用了，所以这里指定`VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT`
* memory properties：由于客户端不需要这张图做什么，所以指定`VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`
* tiling：`VK_IMAGE_TILING_OPTIMAL`

之后紧接着创建 image view。接下来使用 barrier 将图像布局从 `VK_IMAGE_LAYOUT_UNDEFINED` 转换到 `VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL`：

* `srcAccessMask`：`0`
* `dstAccessMask`：`VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT`

* `sourceStage`：`VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT`
* `destinationStage`：`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`

由于深度缓冲也需要配合使用多重采样，所以在深度图创建时也要指定多重采样数。

