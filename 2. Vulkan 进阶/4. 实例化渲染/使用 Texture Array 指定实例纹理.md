## 使用 Texture Array 指定实例纹理

我们将使用一个传给 Fragment Shader 的 `Sampler2DArray`，即上一张的纹理数组，结合上每个实例的纹理数组索引来在 Fragment Shader 中采样这个采样器。

由于我们这一套 UBO 只赋予了 Vertex Shader，并没有传给 Fragment Shader，而只传了 UV 坐标，所以我们要单独传一个编号过去。这个编号当然可以是 `gl_InstanceIndex`，但在该案例中我们考虑到不同的方块有可能对应一个纹理元素，因此我们使用自定义的 `arrayIndex`。

不过还没完，在收尾 Vertex Shader 之前，让我们看看 Fragment Shader 中怎么接受一个 Texture Array。跟 Cubemap 类似，只需要在 shader 中将传统的 `Sampler2D` 替换成 `Sampler2DArray` 即可。换了个特定的采样器，在使用 `texture` 采样时我们同样可以传入一个三维向量，其中前两维 UV 对应的就是该片元的UV坐标，但第三维 W 则对应的是元素的下标（这就很方便了）。注意由于 UVW 是一个由3个 `float` 元素所构成的向量，所以下标也需要转换为 `float`。

因此，在 Vertex Shader 中我们传出 UV 坐标时，可以直接传出一个 `vec3`：`outUVW = vec3(inUV, ubo.instance[gl_InstanceIndex].arrayIndex.x);`（注意这里为了保证 16 字节 padding 做了一点 hack，即这个`.x`，其实可以使用之前提到的 `alignas(16) `来对齐），Fragment Shader 接到后直接传入 `texture` 采样函数中，既可以实现选择 Texture Array 中元素并进行采样的功能。

