# 不同图形 API 之间的区别

本文中将会考虑的 API：

* OpenGL/GLES/WebGL（下文统称 OpenGL）
* Direct3D
* Metal
* Vulkan

表格中带 `*` 的字段为非正交字段，可能为其它字段的通俗理解。



## NDC 坐标

|                                | OpenGL   | Vulkan  | Direct3D | Metal   |
| ------------------------------ | -------- | ------- | -------- | ------- |
| 深度范围 `[MinZ,MaxZ]`         | `[-1,1]` | `[0,1]` | `[0,1]`  | `[0,1]` |
| 左手系 / 右手系                | 左       | 右      | 左       | 左      |
| Y 轴方向（当 Z 指向屏幕内侧）* | Y-up     | Y-down  | Y-up     | Y-up    |

**Workarounds：**

* OpenGL 4.5 可以使用 [ARB_clip_control](https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_clip_control.txt) 将深度范围从 `[-1, 1]` 转换为 `[0, 1]`，GLES 和 WebGL 并不支持：

  > This extension can be used to render content used in a Direct3D
  >   application in OpenGL in a straightforward way without modifying vertex or
  >     matrix data.  When rendering into a window, the command
  >    
  >      glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
  >    
  >  configures the near clip plane to correspond to a z normalized device
  >     coordinate of 0 as in Direct3D.  Geometry with (x,y) normalized device
  >     coordinates of (-1,-1) correspond to the lower-left corner of the viewport
  >     in Direct3D, so no change relative to OpenGL conventions is needed there.
  >     Other state related to screen-space coordinates may need to be modified
  >     for the application to map from Direct3D to OpenGL window coordinate
  >     conventions.
  
* Vulkan 可以使用 [VK_KHR_maintenance1](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VK_KHR_maintenance1) 将右手系转换为左手系，或使用引用中 Shader 代码在输出时反转 Y 轴（前提是按照 OpenGL 约定进行的之前的计算）：

  > - Allow negative height to be specified in the [VkViewport](https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkViewport)::`height` field to perform y-inversion of the clip-space to framebuffer-space transform. This allows apps to avoid having to use `gl_Position.y = -gl_Position.y` in shaders also targeting other APIs.
  
  上述两种操作将会把 OpenGL 和 Vulkan 的坐标完全转换成 Direct3D 和 Metal 的坐标约定。若需要建立跨图形 API 渲染器时可以使用上述操作避免在 Shader / 矩阵中硬编码或做区分。

**游戏引擎 Workarounds：**

* Unity：（Unity 世界空间为 Y-up 左手坐标系）

  Unity 在 Shader 中使用 `UNITY_NEAR_CLIP_VALUE` 判定深度范围的问题，在OpenGL平台该宏取值为-1，其余平台为0。需要注意的是，Unity 在所有非 OpenGL 平台上都做了Reversed-Z 优化（OpenGL 的 [-1, 1] 不好做 Reversed-Z），因此真正的近平面值也需要考虑 `UNITY_REVERSED_Z` 宏。

  ```glsl
  // HLSLSupport.cginc:558
  
  #if defined(SHADER_API_D3D11) || defined(SHADER_API_PSSL) || defined(SHADER_API_XBOXONE) || defined(SHADER_API_METAL) || defined(SHADER_API_VULKAN) || defined(SHADER_API_SWITCH)
  // D3D style platforms where clip space z is [0, 1].
  #define UNITY_REVERSED_Z 1
  #endif
  ```

* Unreal：（Unreal 世界空间为 Z-up 左手坐标系）

  

## 视口坐标（Viewport / Framebuffer Coordinate）

|                        | OpenGL | Vulkan | Direct3D | Metal  |
| ---------------------- | ------ | ------ | -------- | ------ |
| Y 轴方向（Y-up/down）* | Y-up   | Y-down | Y-down   | Y-down |
| (0, 0)点位置           | 左下   | 左上   | 左上     | 左上   |

**Workarounds：**

这里只有 OpenGL 的原点位置为左下，因此在需要输入视口坐标的平台（例如 `imageStore`）注意反转 Y 轴即可。



## 纹理坐标 （Texture Coordinate）

纹理坐标与视口坐标唯一不同在于纹理坐标取值范围`[0, 1]`，而视口坐标取值范围为`[0, 视口X/Y分辨率]`，不同 API 区别同上。

