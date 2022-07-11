## 物体描边 Shader

在描边的 Vertex Shader 中，我们要让物体“鼓”起来，这样用原来的物体当模板盖住这个鼓起来的物体，鼓起来的物体才会露出一个边，即我们所需要的轮廓。“鼓”起来的方法即将所有顶点沿着法线的方向移动一段距离（即挤出操作），这段距离就是描边的粗细程度。代码如下：

```glsl
void main() 
{
	vec4 pos = vec4(inPos.xyz + inNormal * ubo.outlineWidth, inPos.w);
	gl_Position = ubo.projection * ubo.model_view * pos;
}
```

注意，这个挤出操作在模型空间内执行，因此真正渲染到屏幕上的时候，这个描边并不等宽。若想等宽可以考虑将 `ubo.outlineWidth` 依照透视进行改变。

