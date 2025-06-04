# KDE Connect Windows

KDE Connect fork on windows with many modifications and simplification, dropping many many dependencies... 

## Installation instructions
### Requirements

Windows 10 x64 (version 1803 or later) is required.

Additionally, you need to install the following tools/components:

- [Git](https://gitforwindows.org/)
- [Visual Studio Community 2022](https://visualstudio.microsoft.com/vs/community) 
  - install the `Desktop development with c++` workflow 
  - install the `WinUI applicatioin development` workflow *(check the `C++ (v14x) Universal Windows Platform tools` option(s) )*
- [Qt](https://www.qt.io/download-qt-installer) `>= 6.7.3` *(`MSVC 2022 64-bit` Prebuilt Components are required)*
  - [CMake](https://cmake.org/download/) *(can be installed via the Qt installer)*
  - [OpenSSL](https://www.openssl.org/) *(can be installed via the Qt installer)*

### Building

The following steps are done all via the command line. If you're not that comfortable with it, you can also to configure the project using a graphical tool like the CMake GUI, or Qt Creator.

```shell
git clone https://github.com/pikazh/kdeconnect-windows.git --recurse-submodules
cd kdeconnect-windows
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=D:\Qt\6.7.3\msvc2022_64["replace with your qt lib dir"] -DOPENSSL_ROOT_DIR=D:\Qt\Tools\OpenSSLv3\Win_x64["replace with you openssl lib dir"] -DCMAKE_INSTALL_PREFIX=d:\kdeconnect["replace with your install destination dir"] ..
cmake --build . --config Debug
cmake --install . --config Debug
```
And then copy "libcrypto-3-x64.dll" and "libssl-3-x64.dll" from OpenSSL to "bin" dir inside your install destination. Now you can run "kdeconnect-app.exe".
