## 创建 image

首先在使用 stb 库读取纹理的时候，就可以通过上述公式算出 mip 层级：

```c++
mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
```

这个值也成为了之后调用所有跟这个纹理有关的方法的一个参数，即 `mipLevels` 或是 `subresourceRange.levelCount`，而这些参数之前都填写为1，因为当时并没有生成mipmaps。

接下来在所有出现这些参数的地方将这个值传入：

```c++
VkImageCreateInfo imageInfo;
imageInfo.mipLevels = mipLevels;

VkImageViewCreateInfo viewInfo;
viewInfo.subresourceRange.levelCount = mipLevels;

VkImageMemoryBarrier barrier;
barrier.subresourceRange.levelCount = mipLevels;
```

注意这里的 `mipLevels` 只需要影响对应纹理的创建，与 swap chain 和 depth buffer image 都无关，在这些地方 mipLevels 都为1，即没有 mipmapping。

