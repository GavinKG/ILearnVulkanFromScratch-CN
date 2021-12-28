## 修改 Render Pass

已经创建出 depth attachment，现在要把它加进我们需要用的这一套 render pass了。照例需要先填写一个`VkAttachmentDescription `：

```c++
VkAttachmentDescription depthAttachment = {};
depthAttachment.format = findDepthFormat();
depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

// 不关心上一次图像的内容，直接清空
depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

// 临时attachment，我们这里绘制完了以后就不需要了，所以这里不Store。
depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

// 没用到模板，都不管
depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

//  不关心上一次图像的布局，不管它
depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

// 这里很好理解
depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
```

接下来该修改这里subpass的实现了。同 color attachment 一样，这里使用`VkAttachmentReference `做引用，并在`VkSubpassDescription`声明中传入该ref。注意这里接受的并不是一个数组，因为一个subpass只能用一个深度（+模板）缓冲。

最最后修改`VkRenderPassCreateInfo `创建信息，并把两个 attachment 都传进去。

在设置 render pass 的时候也需要处理一些小细节，比如为 depth buffer 设置一个clear value。这里不同于颜色设置，由于在深度缓冲中最近为0，最深为1，所以这里要全部设置为1，即

```c++
clearValues[1].depthStencil = {1.0f, 0};
```

不要忘记也需要填上这里没使用到的 stencil 值。

