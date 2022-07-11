支持功能：

* glTF
* 所有【Vulkan基础】中的特性
* CSM Shadow
* PrePass - CSM - Opaque - Translucent
* 动态 mipmapping
* Frustum Culling，多线程（可选）
* Sort Key
* Dynamic Instancing
* PBR 材质，使用 Microfacet BRDF
* CMake
* Push Constant 传递 MVP
* Specialization Constants 实现 Uber Shader，避免 Dummy Texture 采样
* SDR 渲染，不 Tonemap
* （可选）Pipeline Cache

Draw：

```
Prepare View:
vkCmdBeginRenderPass
vkCmdSetViewport
vkCmdSetScissor

Object rendering:
vkCmdBindPipeline
vkCmdBindDescriptorSets
vkCmdBindVertexBuffers
vkCmdBindIndexBuffer
vkCmdDrawIndexed

Object rendering without rebinding pipeline (w/ sort key):
vkCmdBindDescriptorSets
vkCmdBindVertexBuffers
vkCmdBindIndexBuffer
vkCmdDrawIndexed

Object rendering, same object, dynamic instancing
vkCmdDrawIndexed

Debug:
vkCmdDebugMarkerBeginEXT
```

