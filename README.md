# ssEngine

引擎正在模块化处理的初期，大部分代码在 `source/as-is` 里（未模块化处理的代码）。

## Build Instructions

### 在 Windows 10 上使用 Visual Studio 2017 编译

记得在 VS2017 安装器上勾选 C++ 和 CMake 支持。

在 VS 里用 Open Folder 打开项目文件夹。等待 VS 初始化完成。

#### 编译 Windows 版
编译、运行、调试 Windows 版就和其它 VS 项目一样。

#### 编译到 WebAssembly
首次编译要先下载安装 Emscripten。按照官网配置好 Emscripten 后，__以管理员身份__打开命令提示符，运行

```
emsdk activate latest --global
```

在 VS 的 Project Settings 下拉框里选择 `Web (Release)` ，然后点击菜单 `CMake -> Build All` 进行编译。

编译好的文件是 `bin/ss-engine.{html,js,wasm,data}`.
__复制到网页服务器上__再用浏览器访问查看结果。

#### 编译到 WebAssembly (使用 Docker)
如果无论如何装不上 Windows 版的 Emscripten， 可以尝试使用 Docker。

下载安装 Docker for Windows。这需要 Windows 10 Pro 或 Enterprise 版。Home 版无法安装。

在 Docker 的 Settings 里，设置 Shared Drives。勾选项目文件所在磁盘，点 Apply。为了加快下载速度，再在 Daemon 设置里添加一个 registry mirror `https://registry.docker-cn.com`, 点 Apply，等待 Docker 重启。

在 VS 的 Solution Explorer 里随便找一个文件或文件夹，右键菜单里点 `Web / Build with Docker` 开始编译。第一次编译前会下载所需的镜像文件，这需要较长时间。

编译好的文件是 `bin/ss-engine.{html,js,wasm,data}`.
__复制到网页服务器上__再用浏览器访问查看结果。

### 在 Arch Linux 里编译
#### 编译到 WebAssembly

依赖：

```
pacman -Syu --needed base base-devel cmake emscripten
yaourt -S --needed binaryen
```

编译：

```
make web -j8
```

编译好的文件是 `bin/ss-engine.{html,js,wasm,data}`.
__复制到网页服务器上__再用浏览器访问查看结果。

#### 编译到 Linux

依赖：

```
pacman -Syu --needed base base-devel glfw-x11 freeimage assimp glew libglvnd
```

编译运行：

```
make linux -j8
make run  # 运行
```

编译好的文件是 `bin/ss-engine`.

