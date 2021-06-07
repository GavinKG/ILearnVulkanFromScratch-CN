## tiny_obj_loader

### 加载顶点

由于索引数量将会大于65535，所以这里使用`uint_32`作为索引格式，注意在`vkCmdBindIndexBuffer`的时候也要改过来。

在创建VBO和索引缓冲之前将模型读入，这里直接上代码讲解：

```c++
    void loadModel() {
        
        // 这一系列代码获取 obj 文件的顶点属性和子物体（材料这里不用）
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
            throw std::runtime_error(warn + err);
        }

        // 这一段将读取到的数据录入到自己的vertices和indices
        // indices的录入方法假设所有顶点都是唯一的（和直接不带索引绘制没区别）
        for (const auto& shape : shapes) { // 这里将所有子物体合并
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex;
                vertex.pos = { // 这里库帮你做好了face的三角形化
                    attrib.vertices[3 * index.vertex_index + 0], // 3个float
                    attrib.vertices[3 * index.vertex_index + 1], // 表示一个
                    attrib.vertices[3 * index.vertex_index + 2]  // 顶点坐标
                };
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    // 这里obj文件中的uv坐标v从下往上，为了和Vulkan一致将其倒过来
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
                vertex.color = { 1.0f, 1.0f, 1.0f }; // 顶点颜色不会使用，占位
                vertices.push_back(vertex);
                indices.push_back(indices.size()); // 索引相当于没用
            }
        }
    }
```

### 优化重复顶点

在上述顶点集中，很多顶点都是复用的，造成大量的重复顶点，并且索引缓冲就是个摆设。这里首先需要摆脱掉重复顶点，而最直接的方法就是用一个HashMap来索引并且去重。这里选用`unordered_map`：

```c++
std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
```

每当构建好一个`Vertex`顶点，我们都让它去HashMap中找重复。如果没有重复，首先将这个顶点放入顶点集中，接下来新建一个字典项，Key传入这个顶点，Value编一个新的编号，即`uniqueVertices.size()`；如果重复，啥都不做。最后将这个索引放入索引集，则可以真正利用上索引缓冲。

由于自定义结构体 `Vertex` 被用到了HashMap中，所以这里要实现其哈希方法（用于选桶）和比较方法（用于在桶中找）。首先是比较方法，使用成员函数（由于没有转换构造函数，这里直接放在类中）：

```c++
bool operator==(const Vertex& other) const {
    return pos == other.pos && color == other.color && texCoord == other.texCoord;
}
```

然后是哈希方法，这里打开std命名空间特化 `hash` 类模板（这也是更改std的唯一用途）：

```c++
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
```

重复顶点将会被排除，索引也起到了真正的作用，看似复杂的哈希操作也只会运行在一开始，极大的优化了渲染流程。
