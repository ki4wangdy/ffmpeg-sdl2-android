
core目录的作用：

该目录主要通过j4a(bilibili开源的jni生成工具，参考jni4android)，通过j4a工具，生成jni代码，从而在jni层直接访问java层对象。

编译流程：

1.先进入jni4android目录，参考README，生成j4a可执行命令

2.将上述生成的j4a文件拷贝到core目录中的bin文件夹里

3.core目录里的make命令
