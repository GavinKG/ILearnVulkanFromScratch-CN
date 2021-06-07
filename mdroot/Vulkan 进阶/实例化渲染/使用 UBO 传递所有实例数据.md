## 使用 UBO 传递所有实例数据

该案例中，UBO 的组织方式如下：

```c++
struct UboInstanceData {
	glm::mat4 model;
	glm::vec4 arrayIndex;
};

struct {
	struct {
		glm::mat4 projection;
		glm::mat4 view;
	} matrices;
	UboInstanceData *instance;		
} uboVS;
```

可以看出，我们将所有实例的数据和全局的 VP 矩阵都放在了一个大的 UBO 中，Uniform Buffer 中的数据组织形式为 `[VP Matrix][UboInstanceData][UboInstanceData]...[UboInstanceData]`。在准备 UBO 时，将 `instance` 成员指向一个 `UboInstanceData` 实例数组，分配上不同的 Model 变换矩阵以及纹理数组中对应的纹理（注意需要转换成 float）。由于 `new[]` 运算符分配的内存地址保证连续性，所以直接可以 map `*instance` 指针到 buffer 的指定地方。

下面到了绘制的环节。我们一直用的 `vkCmdDrawIndexed()` 函数直接支持实例化绘制，只需要指定 `instanceCount` 和 `firstInstance` 参数即可（之前这两个参数一直指定为 1 和 0）。

下面就是最重要的 Vertex Shader 环节。在 `vkCmdDrawIndexed()` 提交后，在GPU内部会产生 `instanceCount` 个物体对象，其编号 `gl_InstanceIndex` （一个内置 shader 变量）会被指定为对应实例编号，范围为 [`firstInstance`, `firstInstance + instanceCount`)。由于我们的大 Uniform Buffer 每个 Vertex Shader 都可见，我们便可以直接把传入的 `arrayIndex` 变量当成下标访问对应的 Model 变换矩阵：

```GLSL
struct Instance
{
	mat4 model;
	vec4 arrayIndex;
};

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	Instance instance[8];
} ubo;

// ...

mat4 modelView = ubo.view * ubo.instance[gl_InstanceIndex].model;
gl_Position = ubo.projection * modelView * vec4(inPos, 1.0);
```

