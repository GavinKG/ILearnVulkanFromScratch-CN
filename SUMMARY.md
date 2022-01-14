* [引用](mdroot/引用.md)
* [Vulkan 命名和使用规范](mdroot/Vulkan 命名和使用规范.md)
* [Vulkan 基础](mdroot/Vulkan 基础.md)

    * [准备流程](mdroot/Vulkan 基础/准备流程.md)
      * [使用 GLFW 创建窗口](mdroot/Vulkan 基础/准备流程/使用 GLFW 创建窗口.md)
      
      * [初始化 Vulkan 实例](mdroot/Vulkan 基础/准备流程/初始化 Vulkan 实例.md)
      
      * [验证层 (Validation Layer)](mdroot/Vulkan 基础/准备流程/验证层.md)
      
      * [借助 GLFW 创建 Surface](mdroot/Vulkan 基础/准备流程/借助 GLFW 创建 Surface.md)
      
      * [指定物理设备 (Physical Device)](mdroot/Vulkan 基础/准备流程/指定物理设备.md)
      
      * [抽象逻辑设备 (Logical Device)](mdroot/Vulkan 基础/准备流程/抽象逻辑设备.md)
      
      * [交换链 (Swap Chain)](mdroot/Vulkan 基础/准备流程/Swap Chain.md)
      
      * [创建图像视图 (Image View)](mdroot/Vulkan 基础/准备流程/创建 Image View.md)
      
      * [Render Pass](mdroot/Vulkan 基础/准备流程/Render Pass.md)
      
      * [图形流水线 (Graphics Pipeline)](mdroot/Vulkan 基础/准备流程/图形流水线.md)
      
      * [帧缓冲 (Framebuffer)](mdroot/Vulkan 基础/准备流程/帧缓冲.md)
      
      * [指令缓冲 (Command Buffer)](mdroot/Vulkan 基础/准备流程/指令缓冲.md)
      
      * [渲染流程](mdroot/Vulkan 基础/准备流程/渲染流程.md)
      
      * [重建 Swap Chain](mdroot/Vulkan 基础/准备流程/重建 Swap Chain.md)
      
      * [后记](mdroot/Vulkan 基础/准备流程/后记.md)
    * [绘制几何体](mdroot/Vulkan 基础/绘制几何体.md)
      * [顶点 (Vertex)](mdroot/Vulkan 基础/绘制几何体/顶点.md)
    
      * [顶点缓冲 (Vertex Buffer)](mdroot/Vulkan 基础/绘制几何体/顶点缓冲.md)
    
      * [Staging Buffer](mdroot/Vulkan 基础/绘制几何体/Staging Buffer.md)
    
      * [索引缓冲 (Index Buffer)](mdroot/Vulkan 基础/绘制几何体/索引缓冲.md)
    * [资源描述与 Uniform 缓冲](mdroot/Vulkan 基础/资源描述与 Uniform 缓冲.md)
      * [资源描述 (Descriptor)](mdroot/Vulkan 基础/资源描述与 Uniform 缓冲/资源描述.md)
        
      * [Uniform 缓冲 (Uniform Buffer Object)](mdroot/Vulkan 基础/资源描述与 Uniform 缓冲/UBO.md)
    * [图像纹理 (Texturing)](mdroot/Vulkan 基础/图像纹理.md)
      * [从外部加载图像](mdroot/Vulkan 基础/图像纹理/从外部加载图像.md)
        
      * [创建图像对象 (Image)](mdroot/Vulkan 基础/图像纹理/创建图像对象.md)
        
      * [图像视图 (Image View)](mdroot/Vulkan 基础/图像纹理/图像视图.md)
        
      * [采样器 (Sampler)](mdroot/Vulkan 基础/图像纹理/采样器.md)
        
      * [组合图像采样器 (Combined Image Sampler)](mdroot/Vulkan 基础/图像纹理/组合图像采样器.md)
        
      * [纹理坐标](mdroot/Vulkan 基础/图像纹理/纹理坐标.md)
    * [深度缓冲 (Depth Buffering)](mdroot/Vulkan 基础/深度缓冲.md)
      * [深度缓冲图像](mdroot/Vulkan 基础/深度缓冲/深度缓冲图像.md)
      * [应用深度传冲](mdroot/Vulkan 基础/深度缓冲/应用深度传冲.md)
    * [Mipmapping](mdroot/Vulkan 基础/Mipmaps.md)
      * [创建带有 Mipmaps 的图像](mdroot/Vulkan 基础/Mipmaps/创建 image.md)
      
      * [配置 mipmaps](mdroot/Vulkan 基础/Mipmaps/配置 mipmaps.md)
    * [多重采样抗锯齿 (MSAA)](mdroot/Vulkan 基础/MSAA.md)
      * [更新流程](mdroot/Vulkan 基础/MSAA/修改流程.md)
      
      * [Sample Shading 消除着色锯齿](mdroot/Vulkan 基础/MSAA/Sample Shading.md)
    * [后记](mdroot/Vulkan 基础/后记.md)
* [Vulkan 进阶](mdroot/Vulkan 进阶.md)
  * [流水线复用/缓存](mdroot/Vulkan 进阶/流水线复用.md)
    * [派生流水线](mdroot/Vulkan 进阶/流水线复用/派生流水线.md)
  
    * [流水线缓存](mdroot/Vulkan 进阶/流水线复用/流水线缓存.md)
  * [向 Shader 传输数据](mdroot/Vulkan 进阶/向 Shader 传输数据.md)
    * [Dynamic Uniform Buffer](mdroot/Vulkan 进阶/向 Shader 传输数据/Dynamic Uniform Buffer.md)
  
    * [Push Constants](mdroot/Vulkan 进阶/向 Shader 传输数据/Push Constants.md)
    * [Specialization Constants](mdroot/Vulkan 进阶/向 Shader 传输数据/Specialization Constants.md)
  * [高级纹理](mdroot/Vulkan 进阶/高级纹理.md)
    * [纹理压缩](mdroot/Vulkan 进阶/高级纹理/纹理压缩.md)
  
    * [Cubemap](mdroot/Vulkan 进阶/高级纹理/Cubemap.md)
  
    * [Texture Array 和实例化渲染](mdroot/Vulkan 进阶/高级纹理/Texture Array 和实例化渲染.md)
  
    * [三维纹理](mdroot/Vulkan 进阶/高级纹理/三维纹理.md)
  
  * [实例化渲染](mdroot/Vulkan 进阶/实例化渲染.md)
    * [使用 UBO 传递所有实例数据](mdroot/Vulkan 进阶/实例化渲染/使用 UBO 传递所有实例数据.md)
  
    * [使用 Texture Array 指定实例纹理](mdroot/Vulkan 进阶/实例化渲染/使用 Texture Array 指定实例纹理.md)
    * [使用 VAO 传递所有实例数据](mdroot/Vulkan 进阶/实例化渲染/使用 VAO 传递所有实例数据.md)
  * [Subpass](mdroot/Vulkan 进阶/Subpass.md)
    * [Subpass 初步](mdroot/Vulkan 进阶/Subpass/Subpass 初步.md)
  
    * [Subpass 实战：延迟渲染优化](mdroot/Vulkan 进阶/Subpass/Subpass 实战：延迟渲染.md)
  
  * [离屏渲染](mdroot/Vulkan 进阶/离屏渲染.md)
    * [渲染流程](mdroot/Vulkan 进阶/离屏渲染/渲染流程.md)
    * [渲染资源清单](mdroot/Vulkan 进阶/离屏渲染/渲染资源清单.md)
  * [截取屏幕](mdroot/Vulkan 进阶/截取屏幕.md)
    * [原理](mdroot/Vulkan 进阶/截取屏幕/原理.md)
    * [PPM 格式](mdroot/Vulkan 进阶/截取屏幕/PPM 格式.md)
  * [模板缓冲和模板测试](mdroot/Vulkan 进阶/模板缓冲和模板测试.md)
    * [物体描边效果](mdroot/Vulkan 进阶/模板缓冲和模板测试/物体描边效果.md)
      * [创建 Stencil Buffer](mdroot/Vulkan 进阶/模板缓冲和模板测试/创建 Stencil Buffer.md)
  
      * [渲染流程](mdroot/Vulkan 进阶/模板缓冲和模板测试/渲染流程.md)
  
      * [物体描边 Shader](mdroot/Vulkan 进阶/模板缓冲和模板测试/物体描边 Shader.md)
  
  * [使用第三方库加载模型](mdroot/Vulkan 进阶/使用第三方库加载模型.md)
    * [tiny_obj_loader](mdroot/Vulkan 进阶/使用第三方库加载模型/tiny_obj_loader.md)
  
    * [Open Asset Import Library](mdroot/Vulkan 进阶/使用第三方库加载模型/Open Asset Import Library.md)
  
  * [Vulkan in Action：CPU 粒子系统](mdroot/Vulkan 进阶/Vulkan in Action：CPU 粒子系统.md)
    * [粒子](mdroot/Vulkan 进阶/Vulkan in Action：CPU 粒子系统/粒子.md)
    * [粒子发射器](mdroot/Vulkan 进阶/Vulkan in Action：CPU 粒子系统/粒子发射器.md)
  * [Vulkan in Action：简单场景绘制](mdroot/Vulkan 进阶/Vulkan in Action：简单场景绘制.md)
    * [渲染资源](mdroot/Vulkan 进阶/Vulkan in Action：简单场景绘制/渲染资源.md)
  
    * [渲染流程](mdroot/Vulkan 进阶/Vulkan in Action：简单场景绘制/渲染流程.md)
    * [总结](mdroot/Vulkan 进阶/Vulkan in Action：简单场景绘制/总结.md)
  
  * [次级指令缓冲和多线程录制](mdroot/Vulkan 进阶/次级指令缓冲和多线程录制.md)
    * [次级指令缓冲](mdroot/Vulkan 进阶/次级指令缓冲和多线程录制/次级指令缓冲.md)
    * [多线程指令缓冲录制](mdroot/Vulkan 进阶/次级指令缓冲和多线程录制/多线程指令缓冲录制.md)
  * [间接绘制](mdroot/Vulkan 进阶/间接绘制.md)
    * [离线准备绘制命令](mdroot/Vulkan 进阶/间接绘制/离线准备绘制命令.md)
    * [渲染](mdroot/Vulkan 进阶/间接绘制/渲染.md)
  * [查询](mdroot/Vulkan 进阶/查询.md)
    * [查询流程](mdroot/Vulkan 进阶/查询/查询流程.md)
  
    * [遮挡查询](mdroot/Vulkan 进阶/查询/遮挡查询.md)
  
    * [流水线统计数据查询](mdroot/Vulkan 进阶/查询/流水线统计数据查询.md)
  
    * [时间戳查询](mdroot/Vulkan 进阶/查询/时间戳查询.md)
  
  * [Compute Shader](mdroot/Vulkan 进阶/Compute Shader.md)
    * [基本流程](mdroot/Vulkan 进阶/Compute Shader/基本流程.md)
  
    * [Shader 初见：图像处理](mdroot/Vulkan 进阶/Compute Shader/Shader 初见：图像处理.md)
  
    * [GPU 粒子系统初步](mdroot/Vulkan 进阶/Compute Shader/GPU 粒子系统初步.md)
  
    * [GPU 粒子系统进阶：N体模拟](mdroot/Vulkan 进阶/Compute Shader/GPU 粒子系统进阶：N体模拟.md)
  
    * [基于 Compute 的光线追踪](mdroot/Vulkan 进阶/Compute Shader/光线追踪.md)
    * [Compute Shader 简单实现](mdroot/Vulkan 进阶/Compute Shader/光线追踪/Compute Shader 简单实现.md)
* [绘制进阶](mdroot/绘制进阶.md)
  * [光照](mdroot/绘制进阶/光照.md)
    
      * [拟真光照模型](mdroot/绘制进阶/光照/拟真光照模型.md)
      
      * [光照纹理](mdroot/绘制进阶/光照/光照纹理.md)
    * [基于物理的着色](mdroot/绘制进阶/基于物理的着色.md)

      * [PBR 通俗理解和直接光照](mdroot/绘制进阶/基于物理的着色/PBR 通俗理解和直接光照.md)
    * [IBL](mdroot/绘制进阶/基于物理的着色/IBL.md)
    * [复杂材质](mdroot/绘制进阶/基于物理的着色/复杂材质.md)
    * [后处理特效](mdroot/绘制进阶/后处理特效.md)
    
      * [全屏幕多边形](mdroot/绘制进阶/后处理特效/全屏幕多边形.md)
        
      * [后处理特效汇总](mdroot/绘制进阶/后处理特效/后处理特效汇总.md)
    * [HDR](mdroot/绘制进阶/HDR.md)
    
      * [HDR 简介](mdroot/绘制进阶/HDR/HDR 简介.md)
    
      * [泛光特效](mdroot/绘制进阶/HDR/泛光特效.md)
    
      * [绘制流程](mdroot/绘制进阶/HDR/绘制流程.md)
    * [阴影](mdroot/绘制进阶/阴影.md)
    
      * [阴影映射](mdroot/绘制进阶/阴影/阴影映射.md)
    
      * [点光源阴影与全向阴影贴图](mdroot/绘制进阶/阴影/点光源阴影与全向阴影贴图.md)
    
      * [级联阴影映射](mdroot/绘制进阶/阴影/级联阴影映射.md)
    * [骨骼动画](mdroot/绘制进阶/骨骼动画.md)
* [概念汇总](mdroot/概念汇总.md)

    * [设备与逻辑设备 Physical / Logical Device](mdroot/概念汇总/设备与逻辑设备.md)

    * [缓冲与图像 Buffer & Image](mdroot/概念汇总/缓冲与图像.md)

    * [图像布局和排列 Image Layout & Tiling](mdroot/概念汇总/图像布局和排列.md)

    * [指令缓冲 Command Buffer](mdroot/概念汇总/指令缓冲.md)

    * [表面 Surface](mdroot/概念汇总/表面.md)

    * [交换链 Swap Chain](mdroot/概念汇总/交换链.md)

    * [渲染流程和渲染目标 Render Pass & Attachment](mdroot/概念汇总/渲染流程和渲染目标.md)

    * [帧缓冲 Framebuffer](mdroot/概念汇总/帧缓冲.md)

    * [队列 Queue](mdroot/概念汇总/队列.md)

    * [视图 View](mdroot/概念汇总/视图.md)

    * [资源描述符集 Descriptor Set](mdroot/概念汇总/资源描述符集.md)

    * [同步 synchronization](mdroot/概念汇总/同步.md)

    * [Shader 的数据流向](mdroot/概念汇总/Shader 的数据流向.md)

    * [不同图形 API 之间的区别](mdroot/概念汇总/不同图形 API 之间的区别.md)