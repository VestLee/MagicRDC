# MagicRDC
 
Adrill-main和Adrill-Tools 分别是安卓7及以下注入方案的源码和可执行文件，来自：https://github.com/mustime/Adrill

dlfcn_compat-master 是 绕开Android 7.0 及以上，系统阻止应用动态链接非公开 NDK库。制作fake dlopn、dlsym函数 的源码，来自：https://github.com/lizhangqu/dlfcn_compat

mprop-master 是安卓8及以下 修改debuggable为1的方案，来自：https://github.com/wpvsyou/mprop

JNI 包括了安卓8及以下注入方案、Hook OpenNativeLibrary的源码，编译时使用NDK交叉编译即可。
libs 是生成的文件
RDC中时已经配置好的文件，包括白名单renderdoc.cfg
