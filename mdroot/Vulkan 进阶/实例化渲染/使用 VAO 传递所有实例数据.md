## 使用 VAO 传递所有实例数据

我们在之前使用了一个 “uber” UBO 来传递数据，其中存放着所有实例的数据，这在实例数量小，即 UBO 中数据量小时可以被采用。但，UBO 的大小是有 limit 的，在一般的显卡上只能允许 64 KB，所以在面对超多实例物体时我们需要一种不把所有实例的数据打包在一起传递的方法。

在流水线创建填写 `VkVertexInputBindingDescription` 时，其中有一个属性 `VkVertexInputRate`。由于之前传递的都是每个顶点的属性，例如每个顶点的位置、法线、UV、顶点颜色等，每一个顶点都需要对应着不同的值，因此我们把该属性设为 `VK_VERTEX_INPUT_RATE_VERTEX`，所以一个物体的每个顶点获取到的数据都是不同的。该属性还有一个枚举值：`VK_VERTEX_INPUT_RATE_INSTANCE`，代表**一个物体实例对应一份数据**，其中这个物体的每个顶点获取到的数据**相同**。这既为我们需要的方法：使用 VAO 传递所有实例的数据，并且在 Vertex Shader 中只会拿到对应的数据。每个实例拿到的数据在实例 VAO 的下标为该实例编号，在客户端 Draw Call 中由 `instanceCount` 和 `firstIndex` 决定，在 Vertex Shader 中为 `gl_InstanceIndex`。

因此，在流水线绑定顶点输入时，我们需要创建两个绑定，分别对应着传统的逐顶点输入和逐实例输入。来看看创建流水线的代码（这里使用 Sascha 提供的 helper）：

```c++
bindingDescriptions = {
    
	// Binding point 0: Mesh vertex layout description at per-vertex rate
	vks::initializers::vertexInputBindingDescription(0, vertexLayout.stride(), VK_VERTEX_INPUT_RATE_VERTEX),
    
	// Binding point 1: Instanced data at per-instance rate
	vks::initializers::vertexInputBindingDescription(1, sizeof(InstanceData), VK_VERTEX_INPUT_RATE_INSTANCE)
};

attributeDescriptions = {
    
    // Per-Vertex attributes
	vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0),					// Vertex Pos
	vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),	// Normal
	vks::initializers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6),		// UV
	vks::initializers::vertexInputAttributeDescription(0, 3, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 8),	// Color
    
	// Per-Instance attributes
	// These are fetched for each instance rendered
	vks::initializers::vertexInputAttributeDescription(1, 4, VK_FORMAT_R32G32B32_SFLOAT, 0),					// Init WorldPos
	vks::initializers::vertexInputAttributeDescription(1, 5, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3),	// Init Rotation
	vks::initializers::vertexInputAttributeDescription(1, 6, VK_FORMAT_R32_SFLOAT,sizeof(float) * 6),			// Init Scale
	vks::initializers::vertexInputAttributeDescription(1, 7, VK_FORMAT_R32_SINT, sizeof(float) * 7)				// TexArray Index
};

inputState.pVertexBindingDescriptions = bindingDescriptions.data();
inputState.pVertexAttributeDescriptions = attributeDescriptions.data();
```

顶点属性缓冲的创建方法和之前没有任何区别，在这里由于所有会随时间改变的值均可以直接从通用的 UBO 传进，所以实例的顶点属性缓冲也可以使用 staging buffer 提交到设备专属内存中去。在录制 Command Buffer 的时候，也要调用两次 `vkCmdBindVertexBuffer` 并指定 binding 来绑定对应缓冲。

