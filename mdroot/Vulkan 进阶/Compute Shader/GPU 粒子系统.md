## GPU 粒子系统

在该案例中，GPU 粒子系统使用 Compute Shader 参考传入的 UBO 中的信息模拟粒子的行为并将粒子信息存在设备专属的 SSBO 中，并在绘图时直接使用这个 Buffer 作为顶点属性缓冲将顶点绘制出来。在介绍整套系统的流程时，我们首先讨论一种新的缓冲类型：SSBO，进而讨论计算队列和图像队列的同步机制，最终将该案例的绘制/计算流程列举出来。

### SSBO



### 同步

```
Graphics *|----IA----[x1]---VS---Rasterize---FS---Assembly---[x2]     [o2]----IA----[x1] ...
Compute  *|[x1]      [o1]-------------------Compute-------------------[x2][x1]      [o1] ...
```

```
Graphics *|[x]                                 [o]----IA----VS---Rasterize---FS---Assembly ...
Compute  *|---------------Compute--------------[x]
```



### 流程

