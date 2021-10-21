# IBL

同上一篇文章一样，此章仅以最通俗最简单的方法来理解渲染中常用的 PBR 相关理论，故**仅可做为助记使用**。全网有大量的 PBR 原理推导分析文章，能够从 PBR 的发展和选型，物理原理，反射模型公式推导等诸多更详细的方法全面剖析 PBR 理论，这些优质的资料和引用将在本文文末列举出来。

---

之前在写 PBR Shader 的时候考虑的都是**直接光照**，即场景中只出现一个或多个光源的情况。但是这并不能反映真实情况的光照，因为真实情况下物体表面除了灯光直接照明以外，周围的物体都在为该表面间接贡献光照，所以这里我们进入间接光照的领域。

算间接光照最简单的想法就是去跟踪场景中的所有光线，即光线追踪，而众所周知光线追踪太tm慢了，所以如果我们能事先捕捉一个该物体周围的场景的Cubemap（或者其它全景图，例如类似经纬线的 Equirectangular map，其对于地平线的分辨率要高于正上方的天空，比较适合室外这种天空啥都没有的环境。这里只讨论 Cubemap 这种万金油的做法），而每个 Cubemap 的像素都被当成一个小发光体，不就可以差不多做到间接光照了么？当然，理论上物体的每一个细分的表面都应该对应着自己独有的一个半球体光照环境，而不是整个物体共享一个 Cubemap，但是这样的话对于每个细分表面都算 Cubemap，性能开销还不如光线追踪呢，所以**如果周围的物体足够远，整个物体共享一个 Cubemap 的误差也是很小的**。

但是新问题又来了，别忘了渲染方程里的大大的积分符号，我们需要对每个细分表面对整个半球的 Cubemap 采样来的像素做积分。虽然这时候知道每个方向的光通量了，但是积分首先就不好运行时实现，同时因为常见的 BRDF 还分为 Diffuse 和 Specular 两个部分：Diffuse 倒好说，只和入射方向 wi 有关，但需要知道整个半球的光照信息；而 Specular 部分只需要知道高光的那一片（学名叫 Specular Lobe）的光照信息，但和入射出射方向都有关。**所以这里为了快，需要准备两套 IBL 的方法，分别给 BRDF 中的 Diffuse 部分和 Specular 部分用。**



$L_o(p,\omega_o) =  	\int\limits_{\Omega} (k_d\frac{c}{\pi}) L_i(p,\omega_i) n \cdot \omega_i  d\omega_i 	+  	\int\limits_{\Omega} (k_s\frac{DFG}{4(\omega_o \cdot n)(\omega_i \cdot n)}) 		L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$



## Diffuse IBL

将上述漫反射部分的常数提出来：

$L_o(p,\omega_o) =  	k_d\frac{c}{\pi} \int\limits_{\Omega} L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$

p 这里因为有上面的“周围物体足够远”的假设，这里也没什么关系了。看来这时候积分只和入射方向有关了，而 Cubemap 本身就可以记录整个球面的入射角度，所以 Cubemap 在这里很好的派上了用场。为了避免运行时做“积分”，我们可以在运行前预处理这张 Cubemap，**让我们在每个方向上采样到的值就是该细分表面上整个半球方向上的光照强度和（pre-sum）**。

那我们需要干的事情就很清楚了：将一个方向对应的 Cubemap 值和其它半球范围的像素加权混合到一起，成为新的 Cubemap 的该方向上的值。这个操作是不是很眼熟？对，就特别类似后处理的时候常用的卷积操作。

这个预处理后的 Cubemap 有一个名字：Irradiance Map。就长右边这样（为了区别，这里左边的图称为 Environment Map。图来自于 LearnOpenGL）：

![](https://learnopengl.com/img/pbr/ibl_irradiance.png)

这里需要注意，虽然现在我们能把 Irradiance Map 当成一个 Cubemap 存储起来了，但是因为这个图直接和光照挂钩，所以这张图存的时候一定要存线性空间，并且支持分量大于1的像素值。`.hdr` 或者 `.exr` 格式的图像（radiance HDR）就可以做到这点。

还有一个问题：不论是实时还是离线来做这个积分，怎么做积分也是一个绕不开的划题。真的对每一个可能方向去采样理论上根本做不完，也没什么必要，所以这里肯定是用一种离散积分的方法，比如**[黎曼积分](https://en.wikipedia.org/wiki/Riemann_sum)**。

下面首先开始简化这个积分：

$\int\limits_{\Omega} L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$

首先把这个立体角积分改成更熟悉一些的球体表面积积分（球坐标系，$r=1$）：

$\int_{0}^{2\pi} \int_{0}^{\frac{\pi}{2}} L_i(p,\phi_i, \theta_i) \cos(\theta) \sin(\theta)  d\phi d\theta$

这时候套上黎曼积分，我们可以写出以下 GLSL 程序：

```GLSL
vec3 irradiance = vec3(0.0);  

vec3 up    = vec3(0.0, 1.0, 0.0);
vec3 right = cross(up, normal);
up         = cross(normal, right);

float sampleDelta = 0.025;
float nrSamples = 0.0; 
for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
{
    for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
    {
        // spherical to cartesian (in tangent space)
        vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
        // tangent space to world space (for cubemap to use)
        vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

        irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
        nrSamples++;
    }
}
irradiance = PI * irradiance * (1.0 / float(nrSamples));
```

这段程序可以在运行图形应用程序之前就执行然后存在硬盘上，程序执行直接读；也可以动态生成然后存在显存上（和 Mipmap 的生成差不多）。

此时该 Irradiance Map 已经可以直接使用了。但考虑到运行时性能和效果，此处还可以做几点增强。我可能在之后的笔记中提到：

* 用 Irradiance Map 算出 SH，提升性能。全局可以使用一个 SH 模拟天光照明（例如 UE4 中的 SkyLight），也可以组建成网格，实现动态物体的 GI（Unity 中的 Light Probe Group，UE 中的 ILC）。https://www.csie.ntu.edu.tw/~cyy/courses/rendering/06fall/lectures/handouts/lec13_SH.pdf

* 使用 Hammersley 随机采样（球面上的均匀采样）实现 Importance Sampling，提升离线资产处理的性能。http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html



## Specular IBL

之前提到了，只有入射光 wi 为变量的积分可以利用 Cubemap，而这里 w i和 wo 都成为了变量，这可怎么办呢？其实之前我们已经将 Diffuse 拆出来单独求了，所以这里索性继续拆 Specular 的公式，直到能想出一种办法单独记录（就像之前用一个 Cubemap 来记录一样），最后在整合到一块儿去。

接下来就按照这个指导思想来想办法拆掉 Specular 部分！

原始公式：

$
\int\limits_{\Omega} (k_s\frac{DFG}{4(\omega_o \cdot n)(\omega_i \cdot n)}
			L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$  

或者表示成 $\int\limits_{\Omega} f_r(p, \omega_i, \omega_o) L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$

动手拆吧。LearnOpenGL 里使用了一种 Epic Games 提出的方法：**分裂和近似法（Split Sum Approximation）**。这里称为 Sum 是因为之前不是已经把求积分变成求和来实现数值计算了么。拆完了以后得到：

$L_o(p,\omega_o) \approx  	(\int\limits_{\Omega} L_i(p,\omega_i) d\omega_i)	(\int\limits_{\Omega} f_r(p, \omega_i, \omega_o) n \cdot \omega_i d\omega_i)$

### 第一部分

先来看前半部分：$\int\limits_{\Omega} L_i(p,\omega_i) d\omega_i$​

TODO

### 第二部分

再来看后半部分：$$\int\limits_{\Omega} f_r(p, \omega_i, \omega_o) n \cdot \omega_i d\omega_i$$







## Ref

google.github.io/filament/Filament.html 中 "Specular BRDF integration"

