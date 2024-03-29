# Vulkan 学习笔记



作者：Gavin_KG

gavin_kg@outlook.com



### Update Log

一晃眼，从开始写这套学习笔记到现在已经过去了两年时间，自己也从象牙塔里的学生成长为了某个互联网公司的游戏引擎开发大头兵。当团队都在使用商业游戏引擎时，自己的研究重心或多或少的会从底层图形 API 移到对商业游戏引擎的优化改造上，并且由于工作压力格外繁重，这套学习笔记也几乎属于停滞状态，实属抱歉。

在手机游戏画面越来越好的今天，商业引擎也对 Vulkan 等现代图形 API 新特性的支持越来越充分，本人在引擎优化的同时也一直在试图落地这些新特性到不同平台上，因此之后**可能会开一章 “Vulkan 高级” 来专门讲讲这些新特性对于传统渲染管线的一些翻天覆地的变化**，并且附上代码。同时，由于前几章的学习笔记年代比较久远，我对一些概念可能有了更深刻的了解，因此我也会**同时重构之前几章，修正一些错误**，使得刚接触 Vulkan 编程的小伙伴们能够更好的上手，并且加入一些**第一次看可能略有超纲（权当混个眼熟，如果看不懂可以直接跳过），但理解深入后再回来看也能了解到其更完善的定义和使用方法的文本**，争取做到自顶向下和自底向上交替进行的学习路线。同时，我也会**使用引用块来插入一些 Tips 和经验之谈**，让读者能够提前知道某个看似平常的特性是如何在实际工程被巧妙利用的。

所以，在 2022 的新年，这篇笔记被正式重启了！一是想继续记录自己在高级图形 API 上的收获，二是也想让更多感兴趣的小伙伴们能够快速了解 Vulkan 和现代图形 API 的魅力。欢迎 Issue 和邮件催更。



### 这是什么？

该笔记融合了 Vulkan Toturial by Alexander Overvoorde 的中文翻译（“Vulkan 基础” 章节）， Sascha Willems 的 Vulkan 开源案例库的全部中文流程讲解（“Vulkan 进阶” 章节和部分 “绘制进阶” 章节），以及从四面八方收集来的 Vulkan 独有的概念讲解和 Pro Tips（大多在 “概念汇总” 章节，也有很多贯穿在行文之中），同时也包含了很多 API 无关的高级绘制方法。鉴于国内 Vulkan API 的中文参考资料较少，教育环境对于现代图形 API 也不够看重，因此我将我的学习经历和成果分享出来，希望可以帮到更多的同道中人并共同成长。

**这篇学习笔记尚未完成，且并没有以“教程“自封，仅为本人学习笔记资料，里面的所有内容均不保证 100% 准确性。**一开始我只是抱着跟着别人教程记笔记给自己看的态度来起笔的，因此前几个小章节语句非常笼统，到"Vulkan 进阶"章节后才开始着重组织语言保证阅读性。



---

GitHub Pages: https://gavinkg.github.io/ILearnVulkanFromScratch-CN/

GitHub Repo: https://github.com/GavinKG/ILearnVulkanFromScratch-CN



*此 README.md 在  Repo 和 Gitbook 中共享。*