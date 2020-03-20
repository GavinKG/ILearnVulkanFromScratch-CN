# IBL

Image Based Lighting

之前在写PBR Shader的时候考虑的都是直接光照，即场景中只出现一个或多个光源的情况。但是这并不能反映真实情况的光照，因为真实情况下物体表面除了灯光直接照明以外，周围的物体都在为该表面间接贡献光照，所以这里我们进入间接光照的领域。

算间接光照最简单的想法就是去跟踪场景中的所有光线，即光线追踪，而众所周知光线追踪太tm慢了，所以如果我们能事先捕捉一个该物体周围的场景的Cubemap，而每个Cubemap的像素都被当成一个小发光体，不就可以差不多做到间接光照了么？当然这里差不多的意思是，理论上物体的每一个细分的表面都应该对应着自己独有的一个半球体光照环境，而不是整个物体共享一个Cubemap，但是这样的话对于每个细分表面都算Cubemap，性能开销还不如光线追踪呢，所以如果周围的物体足够远，整个物体共享一个Cubemap的误差也是很小的。

但是新问题又来了，别忘了渲染方程里的大大的积分符号，我们需要对每个细分表面对整个半球的Cubemap采样来的像素做积分。虽然这时候知道每个方向的光通量了，但是积分首先就不好运行时实现，同时因为常见的BRDF还分为diffuse和specular两个部分：diffuse倒好说，只和入射方向wi有关，而specular部分和入射出射方向都有关。所以这里需要准备两套IBL的方法，分别给BRDF中的diffuse部分和specular部分用。

回到理论上，这时候渲染方程就可以拆分为：

$L_o(p,\omega_o) =  	\int\limits_{\Omega} (k_d\frac{c}{\pi}) L_i(p,\omega_i) n \cdot \omega_i  d\omega_i 	+  	\int\limits_{\Omega} (k_s\frac{DFG}{4(\omega_o \cdot n)(\omega_i \cdot n)}) 		L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$



## Diffuse IBL

将上述diffuse部分的常数提出来：

$L_o(p,\omega_o) =  	k_d\frac{c}{\pi} \int\limits_{\Omega} L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$

p 这里因为有上面的“周围物体足够远”的假设，这里也没什么关系了。看来这时候积分只和入射方向有关了，而Cubemap本身就可以记录整个球面的入射角度，所以Cubemap在这里很好的派上了用场。为了避免运行时做“积分”，我们可以在运行前预处理这张Cubemap，让我们在每个方向上采样到的值就是该细分表面上整个半球方向上的光照强度和（pre-sum）。

那我们需要干的事情就很清楚了：将一个方向对应的Cubemap值和其它半球范围的像素加权混合到一起，成为新的Cubemap的该方向上的值。这个操作是不是很眼熟？对，就特别类似后处理的时候常用的卷积操作。

这个预处理后的Cubemap有一个名字：Irradiance Map。就长右边这样（为了区别，这里左边的图称为 Environment Map）：

![](https://learnopengl.com/img/pbr/ibl_irradiance.png)

这里需要注意，虽然现在我们能把Irradiance Map当成一个Cubemap存储起来了，但是因为这个图直接和光照挂钩，所以这张图存的时候一定要存线性空间，并且支持分量大于1的像素值。玩过RenderDoc都知道， `.hdr` 或者 `.exr` 格式的图像（radiance HDR）就可以做到这点。

还有一个问题：不论是实时还是离线来做这个积分，怎么做积分也是一个绕不开的划题。真的对每一个可能方向去采样理论上根本做不完，也没什么必要，所以这里肯定是用一种离散积分的方法，比如[黎曼积分](https://en.wikipedia.org/wiki/Riemann_sum)。

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

这段程序可以在运行图形应用程序之前就执行然后存在硬盘上，程序执行直接读；也可以动态生成然后存在显存上（和Mipmap的生成差不多）。

//TODO: 使用 Spherical Harmonics 球谐光照，提升运行时计算速度。

// TODO: Hammersley 随机采样（球面上的均匀采样）实现 importance  sampling，同样能提升运行时计算速度。http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html



## Specular IBL

之前提到了，只有入射光wi为变量的积分可以利用Cubemap，而这里wi和wo都成为了变量，这可怎么办呢？其实之前我们已经将diffuse拆出来单独求了，所以这里索性继续拆specular的公式，直到能想出一种办法单独记录（就像之前用一个Cubemap来记录一样），最后在整合到一块儿去。

接下来就按照这个指导思想来想办法拆掉specular部分。原始公式：

$
\int\limits_{\Omega} (k_s\frac{DFG}{4(\omega_o \cdot n)(\omega_i \cdot n)}
			L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$ 或者表示成 $\int\limits_{\Omega} f_r(p, \omega_i, \omega_o) L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$

动手拆吧。LearnOpenGL里使用了一种 Epic Games 提出的方法：**Split Sum Approximation**。这里称为 Sum 是因为之前不是已经把求积分变成求和来实现数值计算了么。

$L_o(p,\omega_o) \approx  	(\int\limits_{\Omega} L_i(p,\omega_i) d\omega_i)	(\int\limits_{\Omega} f_r(p, \omega_i, \omega_o) n \cdot \omega_i d\omega_i)$

由于毕竟积分不能这么拆，所以这里称其为 approximation。

### Pre-Filtered Environment Map

$\int\limits_{\Omega} L_i(p,\omega_i) d\omega_i$



### Environment BRDF

