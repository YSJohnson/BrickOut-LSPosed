# Brick Out 功能模块

适用于 Brick Out 2623.0.0 的 LSPosed 模块，支持道具数量 10000 和本地内购解锁。

## 安装

安装模块 APK，在 LSPosed 中启用模块并勾选 Brick Out，然后完全退出并重新打开游戏。内购功能依赖游戏中的本地购买与奖励流程，商品的实际领取结果以游戏内表现为准。

## 构建

需要 JDK 17 或更高版本、Gradle 8.11.1、Android SDK 35、NDK 28.2.13676358 和 CMake 3.22.1。设置 ANDROID_HOME 后，在项目目录执行 `gradle assembleDebug`。APK 位于 `app/build/outputs/apk/debug/app-debug.apk`。

模块针对 arm64-v8a 和指定游戏版本构建；游戏更新后需要重新适配原生函数。
