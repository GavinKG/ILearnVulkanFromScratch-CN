

## 借助 GLFW 获取 Surface

**原文：https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface**

流程方法名：`createSurface()`

---

一个 Surface 是一个呈现 Rendered Image 的“表面”，实现它极大程度依赖操作系统，因此这里我们交给 GLFW 来帮忙。

该功能需要 `VK_KHR_surface ` extension，而 GLFW 已经在上述 `glfwGetRequiredInstanceExtensions` 的时候返回了这个 extension。其实 GLFW 也返回了很多平台相关的 extensions，我们为了图个方便就一并拿来用了。

创建一个成员变量 `VkSurfaceKHR surface`，并且借助 GLFW 获取这个平台相关的结构体。代码非常简单：

```c++
// private class member
VkSurfaceKHR surface;

void createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
```

如果我们不想依赖 GLFW 的话，当然也可以使用 `vkCreateWin32SurfaceKHR` 方法结合上 `VkWin32SurfaceCreateInfoKHR`（Windows平台）获取`surface`。此处我们还是选择使用 GLFW 加快开发速度吧！

同样，在 cleanup 阶段需要用 `vkDestroySurfaceKHR` 销毁这个 Surface。

