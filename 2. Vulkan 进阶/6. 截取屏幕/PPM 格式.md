## PPM 格式

全称 Portable PixMap，是一种使用 RGB 顺序排列的，未压缩的图片文件格式，同时支持使用 Human Readable 的 ASCII 进行编码（P3），或者使用二进制 `uint8` 进行编码（P6）。该案例中使用 P6 编码。具体文件格式定义可以参考 https://en.wikipedia.org/wiki/Netpbm_format。

当获得指向图像起始位置的指针后，我们就可以通过使用标准库设施 `std::ofstream` 创建一个二进制文件并写入 header 和真正的图像数据到硬盘中。

