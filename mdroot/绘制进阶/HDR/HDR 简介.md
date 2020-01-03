## HDR 简介

[https://learnopengl-cn.github.io/05%20Advanced%20Lighting/06%20HDR/](https://learnopengl-cn.github.io/05 Advanced Lighting/06 HDR/)，这里有HDR原理的介绍，此处不再赘述。

有时候，多个光源所贡献的颜色值加和可能会超过 1.0，但当在使用 `VK_FORMAT_B8G8R8A8_UNORM` 类型的 attachment 作为 render target 时，超过 1.0 的颜色值会被 clamp 到 1.0；同时，传统的显示设备（SDR 显示设备，即不接受亮度通道的设备）无法显示范围超过 1.0 的颜色值，即使 swap chain 使用浮点格式也会被卡到 1.0 上。因此，如果我们想要在传统显示器上显示“HDR”效果，则首先需要将最终画面渲染到浮点帧缓冲，即 framebuffer attachment 格式为 `VK_FORMAT_R32G32B32A32_SFLOAT`，再使用一种合适的 Tone Mapping 算法将这些越界值压缩回 1.0 以便显示，即从 HDR 转换成 LDR。

Tone Mapping 方法有很多种。最简单的即为 Reinhard  Mapping，原理相当粗暴：

```glsl
vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
```

其曲线如下：

![img](https://pic3.zhimg.com/80/9031118d0581d546051b3c60e4ad2402_hd.png)

当然还有很多其它方法，比如直接用指数函数模拟出的“S 曲线”算法（该案例中采用的方法，见本章最后）；下图所使用的 “Filmic tone mapping”，其为 Naughty Dog 在 Uncharted 2 中使用的 Tone Mapping 方法，出自 <https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting；还有业界公认的 ACES Toon Mapping，对 HDR10 显示设备兼容性好，在 UE4 等引擎和各大本世代游戏广泛应用，ACES Tone Mapping 的原理和实现可以参考这篇博客：<https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/>。

![Comparison of tonemapping or not, from Uncharted 2](https://i.stack.imgur.com/uCvQb.jpg)

因为人眼毕竟不能把全范围的HDR转换为可以感知的SDR（太阳光亮度过高了），有时我们为了模拟人眼也会使用”曝光度“值去调整感知范围。其原理很简单，将颜色值乘以这个曝光度值（或$2^{曝光度值}$，用来和 F-stop 对应）最为最终颜色值即可。曝光度和 Tone Mapping 可以结合在一起使用（或称为 Tone Mapping 的 “Magic Number”），产生更惊艳的画面，该案例中具体代码如下：

```glsl
layout(...) in float exposure;
outColor.rgb = vec3(1.0) - exp(-color.rgb * exposure);
```

