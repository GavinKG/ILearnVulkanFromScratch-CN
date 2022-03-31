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

