
## 初始化 Vulkan 实例

流程方法名：`createInstance()`

### 初始化顺序

一个Instance类似于一个Session，在instance里处理所有的和Vulkan有关的东西

添加一个成员变量`VkInstance instance`

创建`VkApplicationInfo appInfo` ，记录程序的常用信息，可选
`sType = VK_STRUCTURE_TYPE_APPLICATION_INFO`

创建`VkInstanceCreateInfo createInfo`，其中包括：
* 上述appInfo
* 全局extensions（GLFW提供窗口的extension `glfwGetRequiredInstanceExtensions()`）
* ValidationLayers

创建Instance
`VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);`

用完别忘删，在`cleanUp()`中
`vkDestroyInstance(instance, nullptr);`

### 扩展支持

列出所有支持的扩展数量方法到数组：`vkEnumerateInstanceExtensionProperties()`

