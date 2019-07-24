## 创建 Image View

流程方法名：`createImageViews()`

`VkImageView`可以理解为一个看`VkImage`的途径，包括看待方法（1D/2D/3D/CubeMap），ARGB通道等。其同时也是存放一个Texture的重要工具。

添加成员变量`std::vector<VkImageView> swapChainImageViews`并resize成`swapChainImages`的大小

对每一个在列表里的`VkImage`，配置`VkImageViewCreateInfo `并传入一系列格式配置参数，并调用`vkCreateImageView`获取`VkImageView`。注意ImageView需要调用`vkDestroyImageView`手动销毁。

其实Vulkan什么销毁什么不销毁满足C++哲学：谁创建并拥有(own)的东西谁来给销毁。因为`VkImage`是Vulkan创建的，而`VkImageView`是用户手动创建的，所以前者不用管但是要cleanup后者

