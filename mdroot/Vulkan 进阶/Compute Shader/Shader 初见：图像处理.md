## Compute Shader 初见：图像处理

https://www.khronos.org/opengl/wiki/Compute_Shader

此处（同样）使用一个图像处理的案例来初步介绍 Compute Shader。Compute Shader 与传统的 Shader 并没有很大格式上的区别，除去所有的固定输入输出（`location`）之外，引入了几个新的内置变量和方法：

* 在 Compute Shader 中，需要使用 Layout Qualifier 指定 Local Size，例如下方代码即指定的  Local Size 为 16x16x1：

  ```glsl
  layout (local_size_x = 16, local_size_y = 16) in;
  ```

* Compute Shader 没有固定输出，所有对资源的修改均直接作用资源之上。在这里我们使用 Image 资源，对其进行的操作称为 Image Load Store Operations。下列代码指定的资源类型为 `image2D ` ，表示我们需要的是图片而不是采样器，**我们使用像素坐标来获取像素颜色值，而并不是通过纹理坐标采样颜色值**。使用 `readonly` 表示这张图片我们只会读取（image load）它而不会去将数据写入（image store）它：

  ```glsl
layout (binding = 0, rgba8) uniform readonly image2D inputImage;
  layout (binding = 1, rgba8) uniform image2D resultImage;
  // layout (binding = 0) uniform sampler2D no_sampler;
  ```
  
* 一些 Compute Shader 的内置变量定义如下：

  ```glsl
  in uvec3 	gl_NumWorkGroups;			// 即为 vkDispatch 中指定的 Work Group 的三维大小
  in uvec3	gl_WorkGroupID;				// 当前 Work Group 的三维编号，在 gl_NumWorkGroups 范围之内
  const uvec3 gl_WorkGroupSize;			// 即上述声明的 Local Size。该值为编译时常量
  in uvec3 	gl_LocalInvocationID;		// 当前 Work Group 中这个 Shader 的三维编号，在上述 Local Size 范围之内
  
  /*
    该 Shader 在所有 Shader 中的三维编号
    即 gl_GlobalInvocationID = gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID;
    此处正好对应着这个 Shader 处理的图像的像素坐标
  */
  in uvec3 gl_GlobalInvocationID;
  
  /*
    该 Shader 在 Work Group 中的顺序编号
    即 gl_GlobalInvocationID =
            gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y +
            gl_LocalInvocationID.y * gl_WorkGroupSize.x +
            gl_LocalInvocationID.x;
    即将三位编号三维展开到一维
  */
  in uint  gl_LocalInvocationIndex;
  ```



### 图像处理着色器

这里用到的图像处理方法都首先进行临近像素的值获取，之后按照卷积核算出中间像素的颜色，最终写回。

在这里首先在一个 3x3 的循环中获取这个像素及其邻近九个像素的颜色值并存储。在当前 Shader 中，其所对应的像素位置即是 `gl_GlobalInvocationID.xy`。通过调用 `imageLoad` 并传入 `image2D` 类型的变量和偏移后的像素坐标 `ivec2(gl_GlobalInvocationID.x + i, gl_GlobalInvocationID.y + j)` 来获取这九个像素的颜色。在该案例中我们将 RGB 通道分别存储并分别跑卷积运算。

3x3 卷积函数比较简单，实现如下：

```glsl
float conv(in float[9] kernel, in float[9] data, in float denom, in float offset) 
{
   float res = 0.0;
   for (int i=0; i<9; ++i) 
   {
      res += kernel[i] * data[i];
   }
   return clamp(res/denom + offset, 0.0, 1.0);
}
```

得出每个通道的颜色值后通过 `imageStore` 方法并传入图像、像素坐标和像素值来进行图像的写操作。大功告成！



### 附1：常见图像处理卷积核（均为3x3）

* 锐化 Sharpen

  ```
  -1 -1 -1     0 -1  0
  -1  9 -1    -1  5 -1
  -1 -1 -1     0 -1  0
  ```

* 浮雕 Emboss
  ```
  -2 -1  0
  -1  1  1
   0  1  2
  ```
  
* 边缘检测 Outline / Edge

  ```
  -1 -1 -1
  -1  8 -1
  -1 -1 -1
  ```



### 附2：在图像处理时 Compute vs. Fragment Shader

https://computergraphics.stackexchange.com/questions/54/when-is-a-compute-shader-more-efficient-than-a-pixel-shader-for-image-filtering

