## 附加：Sample Shading

现阶段MSAA只是对多边形的边缘进行多重采样来抗锯齿，但有时候我们也需要在对多边形内部着色时也进行多重采样，这种采样就叫做 sample shading。开启它很简单，只需要在创建逻辑设备指定，并且在创建渲染管线的 `VkPipelineMultisampleStateCreateInfo` 指定即可：

```c++
void createLogicalDevice() {
    ...
    deviceFeatures.sampleRateShading = VK_TRUE; // enable sample shading feature for the device
    ...
}

void createGraphicsPipeline() {
    ...
    multisampling.sampleShadingEnable = VK_TRUE; // enable sample shading in the pipeline
    multisampling.minSampleShading = .2f; // min fraction for sample shading; closer to one is smoother
    ...
}
```

