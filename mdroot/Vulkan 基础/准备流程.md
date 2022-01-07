# 准备流程

在开始 coding 之前，我们需要首先配置好 IDE 的静态库和头文件的路径。该笔记中将要使用到的库和头文件依赖如下（按照文件类型分类）：

### 环境

* Vulkan SDK 版本：1.2.198.0

* IDE：Visual Studio 2019
* 编译器 / 生成工具：MSVC v142

### 头文件 

*Visual Studio：项目属性页 -> 配置属性 -> C/C++ -> 常规 -> 附加包含目录*

* Vulkan SDK：`$(VK_SDK_PATH)/Include`
* GLFW 图形库框架：`$(GLFW)/include`。官网：https://www.glfw.org/。本笔记中只会使用 GLFW 创建 Surface 和 Vulkan 上下文。
* GLM 数学库： `$(VK_SDK_PATH)/Third-Party/Include/glm`（在使用安装器安装 Vulkan SDK 并勾选安装第三方库时才会有该目录）

### 静态库搜索目录

*Visual Studio：项目属性页 -> 配置属性 -> 常规 -> 附加库目录*

* Vulkan SDK：`$(VK_SDK_PATH)/Lib`

* GLFW 图形库框架：`$(GLFW)/lib-vc201x`

### 静态库文件

*Visual Studio：项目属性页 -> 配置属性 -> 输入 -> 附加依赖项*

* Vulkan SDK：vulkan-1.lib
* GLFW 图形库框架：glfw3.lib

### 测试

配置好后把下面这段从 Vulkan Tutorial 的代码在 IDE 里跑跑，跑出了个黑色窗口就算配置成功了。

```c++
#define GLFW_INCLUDE_VULKAN // 不要忘了！要不然 glfw 中就不会包含 Vulkan 的东西了。
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported" << std::endl;

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
```

