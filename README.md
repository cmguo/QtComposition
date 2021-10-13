# QtComposition
运行于Qt的一个支持依赖注入的组件容器，参考.NET的 [MEF](https://docs.microsoft.com/en-us/dotnet/framework/mef/) 架构

# 特性：
+ 依赖导出
++ 导出 QObject 派生类的对象
+ 依赖注入
   + 集合依赖，可以导入一组匹配的导出项目
   + 可选依赖，即使没有匹配导出，也不会失败
   + 延迟依赖，导入项目用Lazy表示，稍后可以延迟初始化，支持集合
   + 类型通配，导入不确定类型，通过名称匹配，但是都是 QObject
+ 依赖匹配
   + 类型和名称同时匹配
   + 名称可以省略，会用类型名补充
+ 共享
   + 默认导入对象是共享的
   + 导出、导入可以指定共享方式：any、shared、nonshared
+ 导入完成通知
+ 生命期
   + 对象随容器删除
   + 私有对象可以手动删除
+ 不支持
   + 构造导入
   + 非 QObject 对象

# 快速尝试
通过 Conan 包管理器依赖
+ JFrog 源：conan remote add accumulating https://accumulating.jfrog.io/artifactory/api/conan/default-conan
+ 依赖配置：[requires] QtComposition/master@cmguo/stable
+ 仅在 Qt 5.15.0 VS2019 上构建

# 方案说明
+ [基于 Qt 的组件合成框架](https://blog.csdn.net/luansxx/article/details/120668676)

# 使用：
通过全局变量初始化，注册导入和导出：
```cpp
static QImport<QTestComposition, QObject> import_qtest_impl("impl");
static QImportMany<QTestComposition, QObject> import_qtest_impls("impls", QPart::nonshared, true);
static QExport<QTestComposition> export_qtest;
```
获取释放对象
```cpp
QComponentContainer c;
QTestComposition * test = c.get_export_value<QTestComposition>(QPart::nonshared);
c.release_value(test);
```
