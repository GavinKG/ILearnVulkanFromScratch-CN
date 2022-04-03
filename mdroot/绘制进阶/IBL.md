# IBL

同上一篇文章一样，此章仅以最通俗最简单的方法来理解渲染中常用的 PBR 相关理论，故**仅可做为助记使用**。全网有大量的 PBR 原理推导分析文章，能够从 PBR 的发展和选型，物理原理，反射模型公式推导等诸多更详细的方法全面剖析 PBR 理论，这些优质的资料和引用将在本文文末列举出来。

---

之前在写 PBR Shader 的时候考虑的都是**直接光照**，即场景中只出现一个或多个光源的情况。但是这并不能反映真实情况的光照，因为真实情况下物体表面除了灯光直接照明以外，周围的物体都在为该表面间接贡献光照，所以这里我们进入间接光照的领域。

算间接光照最简单的想法就是去跟踪场景中的所有光线（来做蒙特卡洛积分），即 Path Tracing，而众所周知光线追踪太tm慢了，所以如果我们能事先捕捉一个该物体周围的场景的全景图，这里使用 Cubemap。当然，全景图也有其它形式的记录方法，例如 Spherical Map（我们常见的 HDRI Map 多用 Spherical Map 表示），其对于地平线的分辨率要高于正上方的天空，比较适合室外这种天空啥都没有的环境。这里沿用 Cubemap 的叫法，毕竟用什么数据形式不是重点，重点是能获取任意方向（立体角）上的一个辐照度值。而每个 Cubemap 的像素都被当成一个小发光体，不就可以差不多做到间接光照了么？当然，理论上物体的每一个细分的表面都应该对应着自己独有的一个半球体光照环境，而不是整个物体共享一个 Cubemap，但是这样的话对于每个细分表面都算 Cubemap，性能开销还不如光线追踪呢，所以**如果周围的物体足够远，整个物体共享一个 Cubemap 的误差也是很小的**。

但是新问题又来了，别忘了渲染方程里的大大的积分符号，我们需要对每个细分表面对整个半球的 Cubemap 采样来的像素做积分。虽然这时候知道每个方向的光通量了，但是积分首先就不好运行时实现，同时因为常见的 BRDF 还分为 Diffuse 和 Specular 两个部分：Diffuse 倒好说，只和入射方向 wi 有关，但需要知道整个半球的光照信息；而 Specular 部分只需要知道高光的那一片（学名叫 Specular Lobe）的光照信息，但和入射出射方向都有关。**所以这里为了快，需要准备两套 IBL 的方法，分别给 BRDF 中的 Diffuse 部分和 Specular (Glossy) 部分用。**

现在先看一下这个原始渲染方程，这里使用我们一直在用的 Cook-Torrance BRDF，并且把其 Diffuse 项和 Specular 项分开写（不考虑 Visibility 项的阴影和灯光衰减，因为目前不涉及这两点）：

$$L_o(p,\omega_o) =  	\int\limits_{\Omega} (k_d\frac{c}{\pi}) L_i(p,\omega_i) n \cdot \omega_i  \mathrm d\omega_i 	+  	\int\limits_{\Omega} (k_s\frac{DFG}{4(\omega_o \cdot n)(\omega_i \cdot n)}) 		L_i(p,\omega_i) n \cdot \omega_i  \mathrm d\omega_i$$



## Diffuse IBL

先考虑前面的漫反射部分。将上述漫反射部分的常数提出积分得到：

$$k_d\frac{c}{\pi} \int\limits_{\Omega} L_i(p,\omega_i) n \cdot \omega_i  \mathrm d\omega_i$$

p 这里因为有上面的“周围物体足够远”的假设，这里也没什么关系了。看来这时候积分只和入射方向有关了，而 Cubemap 本身就可以记录整个球面的入射角度，所以 Cubemap 在这里很好的派上了用场。为了避免运行时做“积分”，我们可以在运行前预处理这张 Cubemap，**让我们在每个方向上采样到的值就是该方向上整个半球方向上的光照强度和（pre-sum）**。

那我们需要干的事情就很清楚了：将一个方向对应的半球范围采样得来的像素加权混合到一起，成为新的 Cubemap 的该方向上的值。这个操作是不是很眼熟？对，就特别类似后处理的时候常用的卷积操作。

这个预处理后的 Cubemap 有一个名字：Irradiance Map。就长右边这样（为了区别，这里左边的图称为 Environment Map。图来自于 LearnOpenGL）：

![](https://learnopengl.com/img/pbr/ibl_irradiance.png)

这里需要注意，虽然现在我们能把 Irradiance Map 当成一个 Cubemap 存储起来了，但是因为这个图直接和光照挂钩，所以这张图存的时候一定要存线性空间，并且支持分量大于1的像素值。`.hdr` 或者 `.exr` 格式的图像（radiance HDR）就可以做到这点。

还有一个问题：不论是实时还是离线来做这个积分，怎么做积分也是一个绕不开的划题。真的对每一个可能方向去采样理论上根本做不完，也没什么必要，所以这里肯定是用一种离散积分的方法，比如最简单的[黎曼积分](https://en.wikipedia.org/wiki/Riemann_sum)。

下面首先开始简化这个积分：

$$\int\limits_{\Omega} L_i(p,\omega_i) n \cdot \omega_i  \mathrm d\omega_i$$

首先把这个立体角积分改成更熟悉一些的球体表面积积分（球坐标系，$r=1$）：

$$\int_{0}^{2\pi} \int_{0}^{\frac{\pi}{2}} L_i(p,\phi_i, \theta_i) \cos(\theta) \sin(\theta)  \mathrm d\phi \mathrm d\theta$$

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

* 用 Irradiance Map 算出 SH（以及理论依据），提升性能。全局可以使用一个 SH 模拟天光照明（例如 UE4 中的 SkyLight），也可以组建成网格，实现动态物体的 GI（Unity 中的 Light Probe Group，UE 中的 ILC）。https://www.csie.ntu.edu.tw/~cyy/courses/rendering/06fall/lectures/handouts/lec13_SH.pdf

* 使用 Hammersley 随机采样（球面上的均匀采样）实现 Importance Sampling，提升离线资产处理的性能。http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html



## Specular IBL

再考虑后半截的 Specular 部分：

$$\int\limits_{\Omega_{f_{r}}} f_r(p, \omega_i, \omega_o) L_i(p,\omega_i) n \cdot \omega_i  \mathrm d\omega_i$$

之前提到了，只有入射光 wi 为变量的积分可以利用 Cubemap，而这里 wi 和 wo 都成为了变量，并且光照项和 BRDF 项乘在一起，变量的维度非常高，不适合合在一起进行类似于预计算的操作。

这里使用**分裂和近似法（Split Sum Approximation）**来拆这个积分，并且将积分拆成光照 L 在 BRDF 覆盖范围（Specular Lobe / BRDF Lobe）内的积分和Fresnel 项乘上光通在 BRDF 覆盖范围的积分，也就是**拆成了光照（Lighting）部分和光线传播（Light Transport）部分**。拆完了以后得到：

$$L_o(p,\omega_o) \approx  	(\int\limits_{\Omega} L_i(p,\omega_i) \mathrm d\omega_i)	(\int\limits_{\Omega} f_r(p, \omega_i, \omega_o) n \cdot \omega_i \mathrm d\omega_i)$$

这里说两个问题：

1. 这个约等式其实是对不等式的一种近似。绝大多数情况下此约等式不成立，毕竟真正积分式的乘积是要用分部积分法算的，但实时渲染就喜欢近似和预计算，如果它看起来是对的，那它就是对的。
2. 之所以叫 “Split Sum”，其实是因为 Epic Games 当初用蒙特卡洛法真正去求解渲染方程时，是把那个蒙特卡洛的大和式拆成了两个小和式，这里的 Sum 指的是和式的 Sum，如果用积分形式直接分解的话理论上应该称之为 ”Split Integral“。

### 第一部分：光照部分

先来看前半部分：

$$\int\limits_{\Omega_{f{r}}} L_i(p,\omega_i) \mathrm d\omega_i$$

这部分看起来和上面 Diffuse IBL 非常接近，而唯一不一样的是它的积分域从整个半球变为了 BRDF 的覆盖范围，也就是上面一直在提的 Specular Lobe / BRDF Lobe，于是积分域就和 Lobe “撑起来的胖瘦程度”有关了，而上一篇里提到，Lobe 和 BRDF 项的 Roughness 有直接关系——越粗糙，高光越分散。因此这里虽然是光照部分，但也要考虑 Roughness 了。而 Roughness 是变量，因此我们需要得到一系列不同 Roughness 所对应的 Cubemap。

这里轮到 Mipmapping 来救场了：我们用不同的 mipmaps 离散的表示不同的 Roughness，借助着 Trilinear Filtering 三线性纹理过滤来插值得到真正 Roughness 所对应的光照强度。在实时渲染中，我们可以事先预处理原始环境贴图，得到的 Mipmap 过的环境贴图被称为 **Pre-filtered Environment Map**，预处理环境贴图，如下图所示（来自 LearnOpenGL）：

![](https://learnopengl-cn.github.io/img/07/03/02/ibl_prefilter_map.png)

但是等一下，刚刚说 Lobe 所产生的球面积分域和 BRDF 项的 Roughness 有直接关系，但其肯定当然还和出射角有关，但我们没有考虑！这里其实做了一个很大胆的假设：**假设视线方向和出射方向一样（即 $V=R=N$）**，原因是最终效果其实和考虑出射方向的效果（Ground Truth）差不多。但是这么做当然也是会有误差的，尤其是在视线几乎垂直于法线的掠射方向上会无法获得很好的掠射镜面反射。如下图所示（来自 LearnOpenGL）：

![](https://learnopengl-cn.github.io/img/07/03/02/ibl_grazing_angles.png)

### 第二部分：光线传播部分

再来看后半部分：

$$\int\limits_{\Omega} f_r(p, \omega_i, \omega_o) n \cdot \omega_i \mathrm d\omega_i$$

这个积分也被称为**镜面积分**。

这里直接涉及到一个完整的 Fresnel 公式了，里面的变量有非常的多：光照方向、观察方向、粗糙度、F0，如果我们还想预计算所有这些变量不同值的组合下的积分结果几乎是不现实的。因此我们的思路是一样的：做一些假设砍掉一些变量，想办法**让剩下的参数组合能够预计算得到一个贴图**，运行时直接采样贴图来得到积分结果就好了。

首先，对于光照方向和观察方向来说，我们假设**观察方向就是光照方向的反射方向**，即 $V=R$​，那么这两个值我们就能够用一个观察方向和法线的夹角 $\theta$​ 来表示了。Nice！

现在积分里面还有三个变量：$\theta$、粗糙度 $\alpha$、F0。我们通过一系列操作可以将 F0 提取到外面（这里先不记录推导了，但是要注意推导依旧把不等式看作了近似相等，因此依然是一种近似）得到两个积分式相加：

$$F_0\int\limits_{\Omega} f_r(p, \omega_i, \omega_o) (1-(1-\omega_o\cdot h)^5)n\cdot \omega_i) \mathrm d\omega_i + \int\limits_{\Omega} f_r(p, \omega_i, \omega_o)(1-\omega_o \cdot h)^5 n \cdot \omega_i \mathrm d\omega_i$$

积分式子里面留下来了夹角和粗糙度。此时我们终于可以对这两个变量着手开始预处理了，就把夹角 $cos(\theta)$ 放在横轴，粗糙度放在纵轴，因为值域都是 [0, 1] ，因此刚好能对应上预处理贴图的 UV 坐标，预处理贴图的红色和绿色通道就对应着这两个积分式的结果吧！式子一下子就简化为：

$$F_0 \cdot R + G$$

这张图有一个很响亮的名字：**BRDF Integration Map**，BRDF 积分图，其长这个样子（来自 LearnOpenGL，左下角为坐标原点）：

![](https://learnopengl-cn.github.io/img/07/03/02/ibl_brdf_lut.png)

横轴为 $cos(\theta)$，纵轴为粗糙度，得到的值即为两个积分式的积分结果。这张图是一张很经典的 LUT。







## Ref

PBRT

google.github.io/filament/Filament.html - "Specular BRDF integration"

GAMES202, Lecture 5

https://zhuanlan.zhihu.com/p/56063836

https://www.cnblogs.com/timlly/p/10631718.html

BRDF Viewer - https://patapom.com/topics/WebGL/BRDF/
