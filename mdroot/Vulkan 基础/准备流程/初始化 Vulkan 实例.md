
## 初始化 Vulkan 实例

流程方法名：`createInstance()`

### 初始化顺序

一个Instance类似于一个Session，在instance里处理所有的和Vulkan有关的东西。初始化这些东西比较走流程，这里直接列出来吧：

* 添加一个成员变量`VkInstance instance`；

* 创建一个 `VkApplicationInfo appInfo` 实例，来告诉 Vulkan程序基本信息，例如版本号，程序名称什么的，当然这些都不是必须的：

  ```c++
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;
  ```

* 
* 创建`VkInstanceCreateInfo createInfo`，其中包括：
  * 上述 `appInfo` 结构体
  * 全局 extensions（GLFW提供窗口的extension `glfwGetRequiredInstanceExtensions()`）
  * ValidationLayers

创建Instance
`VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);`

用完别忘删，在`cleanUp()`中
`vkDestroyInstance(instance, nullptr);`

### 扩展支持

列出所有支持的扩展数量方法到数组：`vkEnumerateInstanceExtensionProperties()`

