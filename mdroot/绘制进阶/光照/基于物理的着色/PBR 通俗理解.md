# PBR 通俗理解

【未完成，未标记图像出处】

首先看一下这个完整的渲染方程：

$$L_o(p,\omega_o) = \int\limits_{\Omega} f_r(p,\omega_i,\omega_o) L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$$

其中 $$p$$ 代表表面上需要着色的点，$$\omega_i$$ 代表输入方向角，$$\omega_o$$ 代表输出方向角，$$n$$ 代表表面的宏观法向量。

用大白话来说，这个公式描述的是从 $$p$$ 点向 $$\omega_o$$ 方向角出射的光，等于使用 BRDF 算出来的出射能量占入射能量比 $$f_r(p,\omega_i,\omega_o)$$，乘上入射光单位面积光通量 $$L_i(p,\omega_i)$$，再乘上表面辐射 $$n \cdot \omega_i$$。由于要考虑到整个半球面上所有的入射光，因此渲染方程对 $$\omega_i$$ 在整个半球面上做了积分。（有些地方积分域会用 H 符号代替，真正代表半球）

此处我们首先只考虑直接光照，即 $$\omega_i$$ 为固定方向。间接光照、全局光照同样依赖于这个完整的渲染方程，但此时入射光从四面八方射来，如何**高效/投机取巧完成这个积分**将在后续文章记录。

## BRDF

> 双向反射分布函数(Bidirectional Reflectance Distribution Function,BRDF)是用来定义给定入射方向上的[辐射照度](https://baike.baidu.com/item/辐射照度/2601111)如何影响给定出射方向上的辐射率。更笼统地说，它描述了入射光线经过某个表面反射后如何在各个出射方向上分布这可以是从理想镜面反射到漫反射、各向同性或者各向异性的各种反射。

本质上是一个一维输入，二维输出函数。输入为入射方向 `wi` ，输出为整个半球的出射方向 `wo` 所对应的辐射比率。当我们固定一个出射方向角时，该函数输出的就是该方向输出的光的能量占输入光能量的比值（取值范围 [0, 1]）。所以可以认为，**BRDF 基于物理定义了一个“材质”**。

![Bi-Directional Reflection Distribution Functions - Chuck Moidel](PBR 通俗理解.assets/image007.jpg)

当前已知材质可以被分为两大种：**金属和电介质**，所以我们可以不针对每一种想要数学建模的材质都定义一个独特的 BRDF，而是可以通过输入一些参数来特例化这两大种材质，这些参数就包括：

* roughness：描述微平面粗糙程度
* metalness / metallic：描述材质的金属“程度”。理论上一个材质要不然就是金属要不然就是电介质，但这里允许 [0, 1] 连续取值是因为这么处理可以模拟那种金属上有灰/划痕等小于一个像素的 footprint 的效果（这里footprint 的概念可以参考 real-time rendering 那本书）。

有了这些额外的输入，一些牛逼人士就开始着手来建模一个通用的，描述最基础硬表面的 BRDF 了：



## Cook-Torrance BRDF

$$f_r = k_d f_{lambert} +  f_{cook-torrance}$$

* "Lambert" 部分：这一部分描述所有**漫反射**的光，而且用的是 Lambert Model。计算非常简单：

  $$f_{lambert} = \frac{c}{\pi}$$

  c 代表漫反射光 diffuse，或者称为 albedo，或者记为 $$c_{diff}$$ 或 $$c_{surf}$$。需要注意，因为在 BRDF 中不需要考虑光源方向辐照度/表面接收到的辐照度（即`dot(n, l)`），所以 Lambert 部分就是一个常量，结合上公式外面那个点乘（即`dot(n, l)`）不就是完整的 Lambert 光照么，这也就是为什么管它叫 Lambert 的原因。

  // TODO：pi 和积分

* "Cook Torrance" 部分：

  这部分描述那些非漫反射的高光，这里单独开一章说，要不然项目级数太多了。
  
* kd 为漫反射部分占整个输出光能量的比值，之后会提到如何算高光部分占整个输出光能量的比值 ks。可以得知有 $$k_s+k_d=1$$。

### Cook Torrance specular BRDF part

这部分描述那些非漫反射的高光：

$$f_{cook-torrance} = \frac{DFG}{4(\omega_o \cdot n)(\omega_i \cdot n)}$$

看到这个公式不要慌。三大头 D、F、G 从三大不同角度描述了到底怎么个基于物理，分开来说每一块可能有以下实现方法（实现方法并不唯一）：

* D (Normal Distribution Funcion - NDF)

  $$NDF_{GGX TR}(n, h, \alpha) = \frac{\alpha^2}{\pi((n \cdot h)^2 (\alpha^2 - 1) + 1)^2}$$

  这个公式估算了微平面和半程向量 $$h$$ 的对齐程度，从而能够反映出材质的**粗糙程度**。n不用说，就是法向量。h 就是半程向量，即 `normalize(l+v)`。这里 $$\alpha$$ 即是 roughness，也正是因为有 roughness 一说，NDF 的存在才有意义。
  
  NDF 不难理解：当我们的视线正好和反射光线相同（即半程向量正好为宏观物体的表面法线），且平面完全光滑时，反射光线不就都射眼睛里了么，产生大片亮斑；反之则啥也看不见（类似于下图左面的球）。但是因为有roughness 的微平面属性存在，半程向量不为宏观物体的表面法线（但和微观的微平面法线相同）时，其实也能看见光！这种关系的描述就是 NDF。
  
  这个公式有一个很响亮的名字：GGX/Trowbridge-Reitz (GGXTR)！NDF 在这里可能是最能够直接感受到光照效果的部分了。
  
  ![](PBR 通俗理解.assets/ndf.png)
  
* G (Geometry Function)

  $$G_{SchlickGGX}(n, v, k)        		 =    		\frac{n \cdot v}    	{(n \cdot v)(1 - k) + k }$$

  当表面足够粗糙时，视线可能因为被遮挡看不到被照到的部分（Geometry Obstruction），光本身也会因为照不到某些地方产生阴影（Geometry Shadowing），如下图所示：

  ![](PBR 通俗理解.assets/geometry_shadowing.png)

  可见这个 Schlick GGX 公式就是处理这种情况使的。公式接受法线、观察方向和一个 k，输出取值范围 [0, 1]。k其实就是 $$\alpha$$ 的一种变形，本质上还是反应了roughness：

  $$k_{direct} = \frac{(\alpha + 1)^2}{8}$$  

  $$k_{IBL} = \frac{\alpha^2}{2}$$

  一切看起来很美好，但是仔细想一下啊，视线可能因为被遮挡看不到被照到的部分（上图红色，Geometry Obstruction），光本身也会因为找不到某些地方产生阴影（Geometry Shadowing），所以上述公式的这个观察方向也要分类讨论为光线方向和视线方向，因此我们用下面这个公式 Smith's Schlick-GGX 统一上面的两种情况:

  $$G(n, v, l, k) = G_{SchlickGGX}(n, v, k) G_{SchlickGGX}(n, l, k)$$

  这里 `v` 即视线，`l` 即光线。该式子的取值范围为 [0, 1]，效果如下（理论上 NDF 和这里 G 的 roughness 都应该是一个，但是这里为了演示，固定了 NDF 的 roughness）：
  
  ![](PBR 通俗理解.assets/geometry.png)

* F (Fresnel Equation)

  上面提到的 NDF 和 GF 其实都从微平面理论来讨论材质属性，而下面的菲涅尔方程则是从材质的物理材料本身（substance）来讨论的

  菲涅尔方程描述的是光在两种不同折射率的介质中传播时的反射和折射（Wiki）。？？？

  菲涅尔方程还能够描述一个现象，称为 Fresnel Eﬀect：当对一个平面以近似平行的方法观看的时候，反射比会陡增（Real-time Rendering 4th - p318 配图）。试着把一张打印纸卷成尽量细的纸筒，把纸筒的一侧贴在明亮的显示屏上，顺着纸筒的另一侧看过去，看看靠近显示屏的纸是不是变得更亮了？

  菲涅尔方程可以用下面这个 Fresnel-Schlick 公式来近似：

  $$F_{Schlick}(h, v, F_0) =     F_0 + (1 - F_0) ( 1 - (h \cdot v))^5$$

  其中 h, v 都熟悉，F0 是个什么？F0 被称为 Fresnel Reﬂectance Values，其大体上能描述**一个材质（考虑为平滑的完美平面）将照上去的不同颜色（此处可以考虑为RGB）的光通过直接反射输出的光的能量比率**。问题来了：除了直接反射输出的光，还有如何输出的光呢？这里就要提到次表面散射 Subsurface Scattering 的概念了：

  ![](https://learnopengl.com/img/pbr/surface_reaction.png)
  
  其中黄色出射光为直接反射光，红色光为次表面反射光。可以看到，在一个平滑的平面上（即不考虑微平面理论），在宏观上来说，反射光即为材质的高光 specular 部分，而次表面反射因为是完全随机出射，所以构成了漫反射 diffuse 部分。
  
  这里还是得提一句 Subsurface Scattering 的注意事项。先从引擎玩起再来补图形学知识的人可能会疑惑，Subsurface Scattering 不是做皮肤那种，模拟光从一个像素点入射，从另一个像素点出射的物理现象吗？但这里整篇文章模拟的都是一个像素所覆盖的平面所产生的物理属性（还是之前提到的 pixel footprint 的概念），而这种跨越多个像素的、宏观的次表面反射会在后面使用一个单独的物理模型去模拟，即 BSSRDF。
  
  之前提到了，金属和电介质物理性质不太一样，也就是这两种材质对直接反射和次表面反射的处理方法不太一样，这里就体现出来了：
  
  * 金属：立刻捕获所有光，吸收一部分**来产生颜色**，剩下的直接反射走，**没有多少次表面散射**，即**所有反射光线都来自于直接反射 F0**。（金属表面的自由电子随着入射光振动，终止入射光，产生反射光）
  * 电介质：**不怎么直接反射**，几乎所有反射光光都是次表面散射贡献的（材料里的电子负责振动）。
  
  所以对于 F0 来说：
  
  * 金属：0.5~1.0，并且对于每个波长的可见光来说 F0 值都不一样。
  * 电介质：平均值在 0.04 左右（所以说几乎不怎么直接反射）。对于每个波长的可见光来说 F0 值都差不多。
  
  具体反射的物理学知识可以参考 [Wikipedia](https://zh.wikipedia.org/wiki/%E5%8F%8D%E5%B0%84_(%E7%89%A9%E7%90%86%E5%AD%A6))。当然 F0 也可以直接由折光率（refractive index）来算出来，但是因为写代码也用不上，这里忽略。
  
  公式运算完毕的值 F（这里就是 $$F_{Schlick}(h, v, F_0)$$ 的简写）其实就是之前提到的**高光部分占整个输出光能量的比值** ks。于是 kd 也可以随即用  1-ks（即 1-F ）算出来。



## 如何让美术也知道 F0 是什么玩意儿

其实到现在，之前一直提到的一个参数 metalness / matallic 一直没被用到，而被反复用到的一个参数叫 F0 (Fresnel Reﬂectance Values)。用过商业引擎的人也都知道，在引擎的 PBR 材质主节点上也没有这个 F0 的参数。同时讲了半天，对于美术来说最重要的材质颜色同样没有体现在模型中。那么怎么从暴露出来的 metalness 和材质颜色（Base Color, 或称 Albedo）转化为 F0 呢？

这里再复习一下到底是谁决定了物体的颜色：

* 金属：全部为 F0，kd = 0。入射的一瞬间金属表面吸收不同波长的光（产生材质的颜色）并**直接反射**。
* 电介质： 几乎全是次表面散射的贡献值 kd，只有一小小部分为F0（因为毕竟平均还有 0.04 呢嘛）。

看来 F0 和 kd 的混合就可以做到让这套算法兼容两者：

```c
F0 = mix(vec3(0.04), albedo, metallic); // using metallic to lerp between 0.04 and albedo
// Calculate F...
kd = vec3(1.0) - F;
kd *= 1.0 - metallic;
```

我们就可以用比较贴近生活的 metallic 参数来代替晦涩难懂的 F0 了。

综上，对于一个材质面板来说，和 PBR 有关系的属性如下：

* albedo：可以是纯色，可以是纹理，也可以是纹理 + tint。当纯金属时该值直接代表了 F0（即直接反射率，因为金属也不会有次表面反射）。
* metalness / metallic：取值范围 [0, 1]，同样也可以是纹理。
* roughness：取值范围 [0, 1]，决定着微平面属性，同样也可以是纹理。



## 实现

这里记录一下 LearnOpenGL 中 PBR 直接光照的基础实现，放在这里纯是因为看着方便：

https://learnopengl.com/code_viewer_gh.php?code=src/6.pbr/1.1.lighting/1.1.pbr.fs

```c
const float PI = 3.14159265359;
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
```

```c
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform vec3  albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI = 3.14159265359;
  
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main() {		
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance    = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lightColors[i] * attenuation;        
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }   
  
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    FragColor = vec4(color, 1.0);
}  
```



## 后记

这里只是大概介绍了一下这个渲染方程大致的结构和每个结构的用处，并没有具体进行数学推导。下面有几个推导的链接：

https://zhuanlan.zhihu.com/p/21489591

https://www.cnblogs.com/TracePlus/p/4141833.html

Epic Games 在 SIGGRAPH 上的 ppt

https://cdn2-unrealengine-1251447533.file.myqcloud.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf

LearnOpenGL 的代码在这个链接中：

https://learnopengl.com/PBR/Lighting

https://learnopengl.com/code_viewer_gh.php?code=src/6.pbr/1.1.lighting/1.1.pbr.fs

SIGGRAPH:

[SIGGRAPH University - Introduction to "Physically Based Shading in Theory and Practice"](https://www.youtube.com/watch?v=j-A0mwsJRmk)

[Physically Based Shading in Theory and Practice](https://www.youtube.com/watch?v=zs0oYjwjNEo)

商业引擎/渲染器中实现PBR的链接：

https://docs.unity3d.com/Packages/com.unity.render-pipelines.universal@11.0/manual/shading-model.html

https://marmoset.co/posts/physically-based-rendering-and-you-can-too/