# PBR 通俗理解

[待完善]

完整的渲染方程：

$L_o(p,\omega_o) = \int\limits_{\Omega} f_r(p,\omega_i,\omega_o) L_i(p,\omega_i) n \cdot \omega_i  d\omega_i$

通俗理解：从p点向wo方向角（观察角度/立体角）出射的光 = 反射系数公式BRDF(p点，输入方向角wi，输出方向角wo) * 入射光单位面积光通量(p点、输入方向角wi) * 表面辐射（即dot(n, wi)），其中wi在整个半球面上(omega)积分。

（有些地方积分域会用 H 符号代替，真正代表半球）

## BRDF

> 双向反射分布函数(Bidirectional Reflectance Distribution Function,BRDF)是用来定义给定入射方向上的[辐射照度](https://baike.baidu.com/item/辐射照度/2601111)如何影响给定出射方向上的辐射率。更笼统地说，它描述了入射光线经过某个表面反射后如何在各个出射方向上分布这可以是从理想镜面反射到漫反射、各向同性或者各向异性的各种反射。

本质上是一个二维输入->一维输出函数。当将输入输出指定为光线入射方向wi和出射方向wo时，输出一个intensity值表示该输出方向输出多少入射方向的光，即输入表面辐射*intensity=输出表面辐射。这正好也是BRDF英文名字的由来。所以可以认为，**BRDF 基于物理定义了一个“材质”**。

当然，因为已经知道了材质可以被分为两大种：金属和电解质，所以我们可以不针对每一种想要数学建模的材质都定义一个独特的BRDF，而是可以通过输入一些参数来特例化这两大种材质，这些参数就包括：

* roughness：描述微平面粗糙程度
* metalness / metallic：描述材质的金属“程度”。理论上一个材质要不然就是金属要不然就是电解质，但这里允许[0, 1]连续取值是因为这么处理可以模拟那种金属上有灰/划痕等小于一个像素的footprint的效果（这里footprint的概念可以参考real-time rendering 那本书）。

有了这些额外的输入，一些牛逼人士就开始着手来建模一个通用的BRDF了：

* Cook-Torrance BRDF

  $f_r = k_d f_{lambert} +  f_{cook-torrance}$

  * "lambert"部分：这一部分描述所有漫反射的光，而且用的是lambert model。计算非常简单：

    $f_{lambert} = \frac{c}{\pi}$

    c 代表漫反射光diffuse，或者称为albedo，或者记为$c_{diff}$或$c_{surf}$。需要注意，因为BRDF不考虑光源方向辐照度->表面接收到的辐照度（即`dot(n, l)`），所以Lambert部分就是一个常量，结合上公式外面那个点乘不就是完整的Lambert光照么，这也就是为什么管它叫lambert的原因。

  * "Cook Torrance"部分：

    这部分描述那些非漫反射的高光，这里单独开一章说，要不然项目级数太多了。
    
  * kd为漫反射部分占整个输出光能量的比值，之后会提到如何算高光部分占整个输出光能量的比值ks。可以得知有ks+kd=1。

## Cook Torrance specular BRDF part

$f_{cook-torrance} = \frac{DFG}{4(\omega_o \cdot n)(\omega_i \cdot n)}$

三大头D、F、G从三大不同角度描述了到底怎么个基于物理，分开来说每一块可能有以下实现方法（实现方法并不唯一）：

* D (Normal Distribution Funcion - NDF)

  $NDF_{GGX TR}(n, h, \alpha) = \frac{\alpha^2}{\pi((n \cdot h)^2 (\alpha^2 - 1) + 1)^2}$

  这个公式估算了微平面和半程向量 $h$ 的对齐程度，从而能够反映出材质的**粗糙程度**。n不用说，就是法向量。h就是半程向量，即 normalize(l+v)。这里alpha即是roughness，也正是因为有roughness一说，NDF的存在才有意义。
  
  NDF不难理解：当我们的视线正好和反射光线相同（即半程向量正好为法线），且平面完全光滑时，反射光线不就都射眼睛里了么，产生大片亮斑；反之则啥也看不见（类似于图1）。但是因为有roughness的微平面属性存在，半程向量不为法线时，其实也能看见光！这种关系的描述就是NDF。
  
  这个公式有一个很响亮的名字：GGX/Trowbridge-Reitz（GGXTR）！NDF在这里可能是最能够直接感受到光照效果的 part 了。
  
  ![](https://learnopengl.com/img/pbr/ndf.png)
  
* G (Geometry Function)

  $G_{SchlickGGX}(n, v, k)        		 =    		\frac{n \cdot v}    	{(n \cdot v)(1 - k) + k }$

  当表面足够粗糙时，表面本身隆起来的小疙瘩可能会遮挡住旁边的表面接受光照，导致如果视线足够与法向量平行时，可能很多视线处于小疙瘩的背阴处，从而不接受光照，就像图里这样：

  ![](https://learnopengl.com/img/pbr/geometry_shadowing.png)

  可见这个什么 Schlick GGX 公式就是处理这种情况使的。公式接受法线、观察方向和一个k，输出取值范围[0, 1]。k其实就是a的一种变形，本质上还是反应了roughness：

  $k_{direct} = \frac{(\alpha + 1)^2}{8}$    $k_{IBL} = \frac{\alpha^2}{2}$

  一切看起来很美好，但是仔细想一下啊，视线可能因为被遮挡看不到被照到的部分（上图红色，Geometry Obstruction），光本身也会因为找不到某些地方产生阴影（Geometry Shadowing），所以上述公式的这个观察方向也要分类讨论为光线方向和视线方向，因此我们用下面这个公式 Smith's Schlick-GGX 统一上面的两种情况:

  $G(n, v, l, k) = G_{SchlickGGX}(n, v, k) G_{SchlickGGX}(n, l, k)$

  这里v即视线，l即光线。该式子的取值范围为[0, 1]，效果如下（理论上NDF和GF的roughness都应该是一个，但是这里为了演示，固定了NDF的roughness）：

  ![](https://learnopengl.com/img/pbr/geometry.png)

（NDF和GF其实都从微平面理论来讨论材质属性，而下面的菲涅尔方程则是从材质的物理材料本身（substance）来讨论的）

* F (Fresnel Equation)

  菲涅尔方程描述的是光在两种不同折射率的介质中传播时的反射和折射（Wiki）。？？？

  菲涅尔方程还能够描述一个现象（Fresnel eﬀect）：当对一个平面以近似平行的方法观看的时候，反射比会陡增（Real-time Rendering 4th - p318 配图）。试着把一张打印纸卷成纸筒（尽量细），把纸筒的一侧贴在明亮的显示屏上，顺着纸筒的另一侧看过去，看看靠近显示屏的纸是不是变得更亮了？

  菲涅尔方程可以用下面这个 Fresnel-Schlick 公式来近似：

  $F_{Schlick}(h, v, F_0) =     F_0 + (1 - F_0) ( 1 - (h \cdot v))^5$

  其中h,v都熟悉，F0是个什么？F0被称为 Fresnel Reﬂectance Values，其大体上能描述**一个材质（考虑为平滑的完美平面）将照上去的整个光谱的光通过直接反射输出的光的能量比率**，即材质的**直接反射颜色**。问题来了：除了直接反射输出的光，还有怎么着输出的光？这里就要提到此表面散射的概念了（我觉得这里才是引入这个概念的时候，而不是在一开始）：

  ![](https://learnopengl.com/img/pbr/surface_reaction.png)

  其中黄色出射光为直接反射光，红色光为次表面反射光。可以看到，在一个平滑的平面上（即不考虑微平面理论），在宏观上来说，反射光即为材质的高光specular部分，而次表面反射因为是完全随机出射，所以构成了漫反射diffuse部分。
  
  之前提到了，金属和电解质物理性质不太一样，这里就体现出来了：
  
  * 金属：立刻捕获所有光，吸收一部分，剩下的直接反射走，没有多少次表面散射，即所有反射光线都来自于直接反射F0。（金属表面的自由电子随着入射光振动，终止入射光，产生反射光）
  * 电解质：不怎么直接反射，几乎所有反射光光都是次表面散射贡献的。（材料里的电子负责振动）
  
  所以对于F0来说：
  
  * 金属：0.5~1.0，并且对于每个波长的可见光来说F0值都不一样。
  * 电介质：平均值在0.04左右。对于每个波长的可见光来说F0值都差不多。
  
  具体反射的物理学知识可以参考[Wikipedia](https://zh.wikipedia.org/wiki/%E5%8F%8D%E5%B0%84_(%E7%89%A9%E7%90%86%E5%AD%A6))。当然F0也可以直接由折光率（refractive index）来算出来，但是因为写代码也用不上，这里忽略。
  
  公式运算完毕的值 F 其实就是之前提到的**高光部分占整个输出光能量的比值** ks。于是kd也可以随机用  1-ks（即 1-F ）算出来。

## 如何让美工也知道F0是什么玩意儿

其实到现在，之前一直提到的一个参数 metalness / matallic 一直没被用到，而被反复用到的一个参数叫F0。用过商业引擎的人也都知道，在引擎的PBR材质上也没有这个F0的参数。那么怎么从metalness转化为F0呢？

这里再复习一下到底是谁决定了物体的颜色：

* 金属：全部为 F0，kd = 0
* 电解质： 几乎全是次表面散射的贡献值 kd，只有一小小部分为F0（因为毕竟平均还有0.04呢嘛）。

看来F0和kd的混合就可以做到让这套算法兼容两者：

```c
F0 = mix(vec3(0.04), albedo, metallic); // linear interpolation "mix"
// ...
kd = vec3(1.0) - F;
kd *= 1.0 - metallic;
```

我们就可以用比较贴近生活的metallic参数来代替晦涩难懂的F0了。

综上，对于一个材质面板来说，和 PBR 有关系的属性如下：

* albedo：可以是纯色，可以是纹理，也可以是纹理*tint。当纯金属时该值直接成为F0。
* metalness / metallic：取值范围 [0, 1]，同样也可以是纹理。
* roughness：取值范围 [0, 1]，决定着微平面属性，同样也可以是纹理。



## 后记

这里只是大概介绍了一下这个渲染方程大致的结构和每个结构的用处，并没有具体进行数学推导。下面有几个推导的链接：

https://zhuanlan.zhihu.com/p/21489591

https://www.cnblogs.com/TracePlus/p/4141833.html

Epic Games 在 SIGGRAPH 上的ppt

https://cdn2-unrealengine-1251447533.file.myqcloud.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf

LearnOpenGL 的代码在这个链接中：

https://learnopengl.com/PBR/Lighting