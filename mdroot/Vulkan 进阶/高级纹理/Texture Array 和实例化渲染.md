## Texture Array 和实例化渲染

Unity Doc 上的一段介绍：

> A texture array is a collection of same size/format/flags 2D textures that look like a single object to the GPU, and can be sampled in the **shader** with a texture element index. They are useful for implementing custom **terrain rendering** systems or other special effects where you need an efficient way of accessing many textures of the same size and format. Elements of a 2D texture array are also known as slices, or layers.

一个纹理对象可以存储不止一张图片信息，即存储的是一个数组，每个元素都是一张图片。这样免了频繁地去切换当前需要bind的纹理，而且可以节省系统资源。

### 准备纹理

和上面的步骤一样，首先使用  `gli` 库读入纹理到 `gli::texture2d_array` 变量中。

在创建 Image 的时候，大部分流程和 cubemap 也是相同的，因为可以发现，cubemap 和 texture array 在 Vulkan 中其实就是一个有多个 layers 的纹理，只不过看待方式不一样，在之后 shader 中的采样器可以看出来。具体来说，创建 image 时将 `arrayLayers` 指定为数组大小即可，甚至不用指定 `flags`。

之后在也要对 texture array 中的每一个元素设置 `VkBufferImageCopy` 。与 cubemap 的设置方法**相同**，texture array 也是通过使用 `bufferOffset` 成员变量来划分缓冲。注意该案例中并没有考虑 mipmap，因此 layout 的计算方法为：`offset += tex2DArray[i][0].size();`。可以看到 `gli` 在组织 texture array 时的数据结构与 cubemap 并没有什么区别。

划分好 staging buffer 之后，就可以用 layout transition -> copy -> layout transition 来创建纹理了。需要注意的是两次改变 layout 时候的 `subresourceRange` 的 `layerCount` 不要忘记改成**数组大小**。

之后，创建 sampler 的步骤和往常一样（又是什么都不用改），创建 image view 时也要改变对应的参数（又只是 `layerCount` 一个）。紧接着设置好 descriptor sets，准备阶段就结束了。


