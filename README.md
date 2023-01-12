# MagicRDC

## Background
RenderDoc在开发中用于Debug，用于查看渲染结果是否符合渲染预期，方便定位到渲染流程中是哪个Pass出了问题。这个功能也被应用于学习中，学习优秀项目的渲染流程。但是一些大型项目是做了反外挂防护的，使用RDC进行分析的时候，程序会闪退。所以需要一些方法绕开防护。

该项目针对 Windows 平台 Android 模拟器环境 和 Android 环境的整合了大佬们常用的解决方案。

## WindowsRelated
适用于 Windows 平台的解决方案。主要思路是 FAKER d3d11.dll 从而加载 renderdoc.dll。

思路来自于[知乎Blog](https://zhuanlan.zhihu.com/p/353043910)

源代码在[SeanPesce/d3d11-wrappe](https://github.com/SeanPesce/d3d11-wrapper)的基础上进行更改，使得模拟器可以在加载 FAKER 之后加载 renderdoc.dll

具体的修改内容和使用方案可以参考[我的博客](https://www.cnblogs.com/vestlee/p/17003036.html) 

## AndroidRelated
适用于 Android 平台的解决方案。

### 方案一：修改debuggable的值为1

mprop-master，是安卓8及以下修改debuggable为1的方案，来自开源库：[mprop-master](https://github.com/wpvsyou/mprop)

Magisk，是安卓8以上修改debuggable为1的方案，参考：[Magisk](https://github.com/topjohnwu/Magisk)

### 方案二：Fake dlopen 加载 renderdoc.dll
思路来自[知乎Blog](https://zhuanlan.zhihu.com/p/376316855)

Adrill-main和Adrill-Tools 分别是安卓7及以下注入方案的源码和可执行文件，来自开源库：[Adrill](https://github.com/mustime/Adrill)

dlfcn_compat-master 是绕开Android 7.0 及以上，系统阻止应用动态链接非公开 NDK库。制作fake dlopn、dlsym函数的源码，来自开源库[dlfcn_compat](https://github.com/lizhangqu/dlfcn_compat)

OpenNativeLibrary中是注入&Hook相关代码。其中JNI 包括了安卓8及以下注入方案、Hook OpenNativeLibrary的源码，编译时使用NDK交叉编译即可。libs 是生成的文件。

RDC中时已经配置好的文件，包括白名单renderdoc.cfg，和上述工具库。

具体的修改内容和使用方案可以参考[我的博客](https://www.cnblogs.com/vestlee/p/17022863.html)
