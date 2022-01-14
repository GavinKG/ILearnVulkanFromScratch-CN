## Sample Shading 消除着色锯齿

> Sample shading **can** be used to specify a minimum number of unique samples to process for each fragment. If sample shading is enabled an implementation **must** provide a minimum of max(⌈ `minSampleShadingFactor` × `totalSamples` ⌉, 1) unique associated data for each fragment, where `minSampleShadingFactor` is the minimum fraction of sample shading. 

之前我们在介绍 MSAA 时，提到其只是对多边形的边缘进行多重采样来抗锯齿，但有时候我们也需要在对多边形内部着色时也进行多重采样，来避免 Shading 锯齿，这时可以使用 Sample Shading 技术。

> 有很多原因可能会导致 Shading 锯齿，例如采样的纹理频率过高，Shader 中存在输出高频信号的计算（例如 `step`），或者是因为材质粗糙度过低/法线纹理频率过高导致有高亮突变。Shading 锯齿可以使用本文中介绍的方法来消除，行业中也存在其它 Shading 抗锯齿方法。

相比于之前一个像素只会执行一次 Fragment Shader 来说（回忆一下，即使我们用 MSAA，片元覆盖的每个像素也只执行一次 Fragment Shader），使用此功能后，**每个像素将会执行多个 Fragment Shader。**

开启它很简单，只需要在创建逻辑设备指定，并且在创建渲染管线的 `VkPipelineMultisampleStateCreateInfo` 指定即可：

```c++
void createLogicalDevice() {
    ...
    deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device
    ...
}

void createGraphicsPipeline() {
    ...
    multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
    multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother
    ...
}
```

我们留意上述 `minSampleShading` 这个参数（当前设置为 0.2）：这个参数意味着对于一个像素，将会有 minSampleShading * MultisampleCount 个 Fragment Shader 会被执行用来多重采样（向上取整）。**例如，如果多重采样数量为 8 的话，那么因为 8 * 0.2 = 1.6，向上取整得到 2，即每个像素会执行两次 Fragment Shader。**注意，此更改应用于整个流水线。

因为一个像素执行了多次 Fragment Shader，我们可以在 Shader 中读出当前 Fragment Shader 的序号。GLSL 中提供的内置输入变量有：

* [`int gl_SampleID`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/gl_SampleID.xhtml)：当前 Fragment Shader 序号。取值范围为 [0, `gl_NumSamples` - 1]

  > `gl_NumSamples` 即为多重采样数量。

* [`vec2 gl_SamplePosition`](https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/gl_SamplePosition.xhtml)：该子像素在当前像素中的偏移位置，每个分量的取值范围为 [0.0, 1.0]

  > 像素中心坐标为 (0.5, 0.5)。

需要注意的是，如果 Shader 中使用了上述变量，**上述 `minSampleShading` 变量值则会忽略，而将唤醒与多重采样数量相同的 Fragment Shader**（或者可以理解为 `minSampleShading` 被改成了 1）。

总结一下，有两种方法来指定执行几次 Fragment Shader：

* 在客户端创建流水线时，指定 `multisampling.minSampleShading`；
* 在 Shader 中使用 `gl_SampleID` 或者 `gl_SamplePosition`。

当然，一个像素执行多个 Fragment Shader 的大前提是我们开启了多重采样的流水线，否则只会调用一次 Fragment Shader，且 `gl_SamplePosition` 的值恒定为 `0.5, 0.5`。

详细解释可以参考 Vulkan Spec：https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#primsrast-sampleshading

> 使用 MSAA + Sample Shading 并把多重采样操作从一帧分摊到多帧，即构成了 TAA 的雏形。
