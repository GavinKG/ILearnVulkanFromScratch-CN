# Vulkan 学习笔记



作者：Gavin_KG

gavin_kg@outlook.com



### 这是什么？

该笔记融合了 Vulkan Toturial by Alexander Overvoorde 的中文翻译（“Vulkan 基础” 章节）， Sascha Willems 的 Vulkan 开源案例库的全部中文流程讲解（“Vulkan 进阶” 章节和部分 “绘制进阶” 章节），以及从 doc、会议、论文、论坛、抓帧等四面八方收集来的 Vulkan 独有的概念讲解和 Pro Tips（大多在 “概念汇总” 章节，也有很多贯穿在行文之中），同时也包含了很多 API 无关的高级绘制方法。鉴于国内 Vulkan API 的中文参考资料甚少，教育环境对于底层 API 和图形学原理也不够看重，因此我将我的学习经历和成果分享出来，希望可以帮到更多的同道中人并共同成长。

**这篇学习笔记尚未完成，且并没有以“教程“自封，仅为本人学习笔记资料，里面的所有内容均不保证 100% 准确性。**一开始我只是抱着跟着别人教程记笔记给自己看的态度来起笔的，因此前几个小章节语句非常笼统，到"Vulkan 进阶"章节后才开始着重组织语言保证阅读性。

本人为北京航空航天大学图形学实验室在读研究生，仅有 C++ / OpenGL / Unity 项目和小型游戏经验，并没有任何商业图形软件和大型商业游戏的开发经验，因此所著内容还望理性分辨和参考。如果有人愿意一同研究 Vulkan 等更现代的图形 API，或发现笔记行文不通顺或记录的内容与事实不符，请务必指导并纠正我，非常感谢。



### Update Log

**2021/7/20 Update:**

最近一直在实习，一直在忙 UE/Unity 的引擎架构向开发工作，但这篇笔记也依然会继续更新，主要还是两个方面起步：Vulkan 的高阶特性，以及绘制算法的 Vulkan 高效实现。

**2020/9/25 Update:**

虽然该网站给出的案例其实和行业中真正使用的技术相差比较大，但是作为 Vulkan 入门，这些案例还是值得反复品味的。我也打算继续把在行业中学到的很多 Vulkan 以及图形学技巧记录到之后的进阶章节中。



---

GitHub Pages: https://gavinkg.github.io/ILearnVulkanFromScratch-CN/

GitHub Repo: https://github.com/GavinKG/ILearnVulkanFromScratch-CN



*此 README.md 在  Repo 和 Gitbook 中共享。*