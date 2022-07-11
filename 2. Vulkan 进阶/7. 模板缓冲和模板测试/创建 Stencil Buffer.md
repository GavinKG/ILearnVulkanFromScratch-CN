## 创建 Stencil Buffer

之前介绍过，stencil buffer 一般情况下是和 depth buffer 一起创建出来的，又称 depth/stencil buffer，因此创建步骤和之前一模一样。

之前创建 depth/stencil buffer 时常用的几种格式是：

* `VK_FORMAT_D32_SFLOAT_S8_UINT`：32位有符号浮点数表示深度，8位无符号整型表示模板，但由于对齐原因，加起来总共占64位，因此浪费了24位。
*  `VK_FORMAT_D24_UNORM_S8_UINT`：24位 UNORM 表示深度，8位无符号整型表示模板。

所以一般情况下，stencil 部分通常使用一个 8-bit 无符号整数表示，即 `uint8_t`，表示范围为 0~255。
