## Cubemap

在这个章节中我们要实现一个使用 cubemap 的天空盒（Skybox），以及一个能够反射天空盒的模型。

### 准备纹理

首先仍要遵守上面使用 staging buffer 的步骤来读取一张纹理并生成 Image。此处使用 `gli` 库读入的纹理被存于类型为 `gli::texture_cube` 的变量中。

由于 cubemap 包含 6 个面，在 Vulkan 中这6个面以 `imageCreateInfo.arrayLayers = 6` 的方法来表述，同时也需要指定 `imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT` 告诉 Vulkan 我们要创建一个 cubemap image。

之后要使用`VkBufferImageCopy` 指定每一个 cube face 的每一个 mip level，即使用两层 for loop：

```c++
for (uint32_t face = 0; face < 6; face++) {
	for (uint32_t level = 0; level < cubeMap.mipLevels; level++) {
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = level;
		bufferCopyRegion.imageSubresource.baseArrayLayer = face;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = texCube[face][level].extent().x;
		bufferCopyRegion.imageExtent.height = texCube[face][level].extent().y;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);

		// Increase offset into staging buffer for next level / face
		offset += texCube[face][level].size();
	}
}
```

可以看出 `gli` 重载了下标运算符，可以轻松的获取到 cube face 的 mip level。

划分好 staging buffer 之后，就可以用 layout transition -> copy -> layout transition 来创建纹理了。需要注意的是两次改变 layout 时候的 `subresourceRange` 的 `layerCount` 不要忘记改成6。

之后，创建 sampler 的步骤和往常一样，创建 image view 时也要改变对应的参数。需要改变参数显而易见，因此不在此处赘述。之后将天空盒和物体都设置好 descriptor sets，纹理的初始化环节就结束了。

### 绘制天空盒

这一张所有配图出自 <https://learnopengl.com/Advanced-OpenGL/Cubemaps>

![Indexing/Sampling from a cubemap in OpenGL](https://learnopengl.com/img/advanced/cubemaps_sampling.png)

天空盒的模型其实就是一个所有面都朝内的1x1x1立方体，其中心在坐标原点。

由于天空可以理解为离摄像机无穷远，因此摄像机在移动的时候应该不能让天空盒也相反着动，这里的解决办法就是将天空盒子的正中心（质心）固定在坐标系原点，在计算 MVP 矩阵乘积时，不让天空盒的立方体去乘 View Matrix，这样不管摄像机坐标系怎么改，World->View 变换后处于原点摄像机也正好在天空盒正中心。

因此，想象一条射线（如图），其射到其中一个立方体面上，而这个点的坐标即可以当成该面上贴图的坐标（当然真把它当成UV的话，需要把[-0.5, 0.5] 移到 [0, 1]，但这里不考虑，因为 shader 中方法替我们完成了）。我们可以直接将顶点的**世界空间坐标**（注意，没有经过投影变换，投影变换一般只给 `gl_Position` 使用供光栅化，其它地方使用没意义）使用 vertex shader 传出，插值后传入 fragment shader。

传入 fragment shader 之后，每个片元所对应的坐标是一向量 uvw（原来在采样 2D 纹理我们只用了二维值 uv），但由于这个cube被放置在了场景正中心，这个坐标向量也可以理解为一个**世界坐标系下的方向向量**（上图中箭头）。GLSL 中的 `texture` 方法传入这个方向向量 uvw 可以采样一个 `samplerCube` 类型的采样器，大功告成。

### 绘制反射物体

![Image of how to calculate reflection.](https://learnopengl.com/img/advanced/cubemaps_reflection_theory.png)

原理见上图。在 vertex shader 中，我们需要获得每一个顶点的世界坐标系法线方向 $\vec{N}$ 和世界坐标系位置，插值后进入 fragment shader 时根据世界坐标系视点位置和每个片元的世界位置获得视线 $\vec{I}$ ，通过 `reflect` 函数计算出世界坐标系下的反射向量 $\vec{R}$。反射向量既是我们想要采样 cubemap 的 uvw，使用 `texture` 采样传入的`SamplerCube`即可。

### 绘制折射物体

![Image explaining refraction of light for use with cubemaps.](https://learnopengl.com/img/advanced/cubemaps_refraction_theory.png)

折射和反射差不多，唯一不同就是我们要用 `refract()` 替代 `reflect()`，其需要视线、法线以及折射率。

具体  `reflect()` 和 `refract()` 的原理此处不再赘述。

### 可视化 Cubemap

//TODO

