# https://github.com/RikkaApps/Riru

# Riru
Riru只做一件事，注入Zygote进程，以允许模块在app或system server中运行其代码。

## 环境需要

安装[Magisk](https://github.com/topjohnwu/Magisk)的安卓7.0+的设备

## 介绍

### 安装

* 一键式

  1. 在Magisk Manager中搜索 「riru」
  2. 安装「riru」

* 手动

  1. 下载压缩包[GitHub release](https://github.com/RikkaApps/Riru/releases)
  2. 在Magisk Manager中安装(Modules - Install from storage - Select downloaded zip)

### 配置

* When the file `/data/adb/riru/disable` exists, Riru will do nothing
* When the file `/data/adb/riru/enable_hide` exists, the hide mechanism will be enabled (also requires the support of the modules)

## Riru原理

* 如何注入Zygote进程

  v22.0之前，通过替换会被zygote进程加载的系统动态库libmemtrack.so来完成Zygote注入，因为libmemtrack.so只有十几个导出函数，非常轻量便于修改，但是这种方式可能导致某些奇怪的问题因为libmemtrack.so可能被用作其他用途

  v22.0之后，通过一个更简单的方式，在`/system/etc/public.libraries.txt`文件中追加我们的so路径，该文件中的所有动态库文件都会被系统自动dlopen，这种方式可见[here](https://blog.canyie.top/2020/02/03/a-new-xposed-style-framework/)


* 如何区分当前运行在app进程还是system server进程?

  有写JNI方法例如com.android.internal.os.Zygote#nativeForkAndSpecialize和com.android.internal.os.Zygote#nativeForkSystemServer是app进程或者system server进程fork时会调用的方法，所以我们可以通过把这些方法替换咱们自己的方法来进行区分。
  替换的方式也很简单，因为libandroid_runtime.so里的所有jni方法都会通过jniRegisterNativeMethods进行注册，所以我们可以通过GOT表hook这个方法，然后替换其中参数jniMethods，然后手动调用一次注册方法来完成替换。

## 如何隐藏Riru

v22.0之后，Riru提供一种通过匿名内存来隐藏/proc/maps痕迹的方式，灵感来源于[Haruue Icymoon](https://github.com/haruue)。

## 构建

Run gradle task `:riru:assembleRelease` `:core:assembleRelease` task from Android Studio or the terminal, zip will be saved to `out`.

## 创建Riru插件

[Template](https://github.com/RikkaApps/Riru-ModuleTemplate)