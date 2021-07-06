

## 借助 GLFW 创建 Surface

流程方法名：`createSurface()`

一个 surface 是一个呈现 Rendered Image 的“表面”，实现它极大程度依赖操作系统。这里我们交给 GLFW 来帮忙。

该功能需要`VK_KHR_surface `extension，而 GLFW 已经在上述`glfwGetRequiredInstanceExtensions`的时候返回了这个extension，其实GLFW也返回了很多平台相关的extensions。

创建一个成员变量`VkSurfaceKHR surface`，并且借助GLFW获取这个平台相关的结构体：

```c++
void createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
```

当然也可以使用`vkCreateWin32SurfaceKHR`方法结合上`VkWin32SurfaceCreateInfoKHR`（Windows平台）获取`surface`。

在cleanup阶段需要用`vkDestroySurfaceKHR`销毁surface。

