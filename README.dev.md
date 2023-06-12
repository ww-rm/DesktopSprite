# DesktopSprite | Developer

[←README](README.md)

这是关于项目代码结构和一些功能实现说明, 普通用户酌情阅读.

## 环境

- `Win10+`
- `Visual Studio 2022`
- `C++14`
- [`spine-runtime`](https://github.com/EsotericSoftware/spine-runtimes/tree/3.6)
- [`jsoncpp`](https://github.com/open-source-parsers/jsoncpp)

其中 `spine-runtime` 和 `jsoncpp` 均是以源码方式引入并编译的, 内部有一些相应的适配性修改.

`spine-runtime` 版本固定了是 `v3.6.53`, 所以几乎只支持碧蓝航线导出的小人资源.

## 代码结构

TODO

## 程序配置

## 注意事项

### 性能监视器部分

性能数据使用 [PDH](https://docs.microsoft.com/en-us/windows/win32/perfctrs/performance-counters-portal) 进行收集, 数据与任务管理器看到的会有一些不同, 但是和性能监视器 [perfmon.msc](https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/perfmon) 的数据是一样的.

有关数据收集的内容见 [`perfmonitor.cpp`](DesktopSprite/src/ds/perfmonitor.cpp).

### 桌宠部分

## 关于 spine
