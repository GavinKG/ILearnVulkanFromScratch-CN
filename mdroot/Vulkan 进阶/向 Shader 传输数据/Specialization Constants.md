## Specialization Constants

> Specialization constants are a mechanism whereby constants in a SPIR-V module can have their constant value specified at the time the `VkPipeline` is created. This allows a SPIR-V module to have constants that canbe modified while executing an application that uses the Vulkan API.

Specialization Constants 为 GLSL 中的一个常量，**在创建流水线时**指定其值。在这个案例中，shader 内部可以根据这个常量来决定使用哪个光照模型，从而避免掉重新编译和改变binding的工作：

```GLSL
// define ...
layout (constant_id = 0) const int LIGHTING_MODEL = 0;
// ...
void main() {
	switch (LIGHTING_MODEL) {
		case PHONG:
			// using phong lighting model
		break;
		case TOON:
			// toon shading
		break;
		// ...
	}
}
```

在创建流水线（注意不是创建 Pipeline Layout）的时候，对于每一个你想要改变该常量的流水线，需要首先对于每一个常量填写一个 `VkSpecializationMapEntry` 结构体，里面包含这个常量的值 `constant_id`，以及 `size` 和 `offset` ，这里需要这两个元素的原因是如果有多个常量需要提交，我们需要把所有常量打包成一个 struct 再提交，所以需要指定这个常量在 struct 中的偏移量和该类型的大小（`sizeof`）。

之后使用 `VkSpecializationInfo` 传入上述声明的 struct。之后在每次声明流水线的时候改变那个 struct 中对应的值即可（因为已经绑定好了）。大功告成！

注意，在流水线指定这个值之后，这个值**不会**再在流水线生命周期内更改，当然就不能使用 UBO 进行更改。因此，这个值适合传一些只和流水线相关的常量，例如 uber shader 的模块选择，是否可视化 shader debug 等。

