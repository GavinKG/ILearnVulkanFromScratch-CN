
**主要参考资源：Vulkan Toturial by Alexander Overvoorde**

对于使用面向对象的C++范式来说，一个App类需要有：

```c++
initVulkan()
mainLoop()
cleanUp()
```

准备流程阶段需要调用的初始化函数都会放在`initVulkan()`中。

在不需要进行特殊内存分配和性能要求的应用程序中，可以大胆的使用C++标准库，例如STL，智能指针等。


