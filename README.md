![unmaintained](http://img.shields.io/badge/status-unmaintained-red.png)

XDK
====
XDK is a fully featured C++ wrapper library that uses [xStl](https://github.com/eladraz/xStl) as a base library.
XDK includes the following features:
* Easy driver development.
* Full C++ features, including **exception handling**.
* Synchronization objects, threads and file access APIs.

For more information about using C++ in kernel, please see [this](ReadmeKernelCPP.html) article.

Requirements
============
The compiled binaries can be operated on `Windows NT` operating system or higher.
Need [Visual Studio](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx) Express 2010 or higher, or Visual Studio 2010 or higher.
Need [Windows Driver Kit (DDK) Version 7.1.0](http://www.microsoft.com/en-us/download/details.aspx?id=11800).

Setting Up Environment
======================
Git
---
```
git clone https://github.com/eladraz/xStl
git clone https://github.com/eladraz/XDK
```

Compilation tools
-----------------
* Install [Visual Studio](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx) Express 2010 or higher, or Visual Studio 2010 or higher.
* Install [Windows Driver Kit (DDK) Version 7.1.0](http://www.microsoft.com/en-us/download/details.aspx?id=11800)

Windows
-------
In order to pass variable arguments to [Visual Studio](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx), you need to declare the following system arguments:
* XSTL_PATH (for example: `C:\WORK\github\xStl`)
* XDK_PATH  (for example: `C:\WORK\github\XDK`)
* DDK_PATH  (for example: `C:\WinDDK\7600.16385.1`)

> To add system variables you should:
>> * Right-click **My Computer**, and then click **Properties**.
>> * Click the **Advanced** tab.
>> * Click **Environment variables**.
>> * Click *New* to add a new variable name (e.g. `XDK_PATH`) and its location (e.g. `C:\WORK\github\XDK`).

How to Build
============
In order to build the XDK library, open `XDK.sln` solution project with [Visual Studio](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx).
In Visual Studio's configuration manager, choose the desired build configuration. Choosing `Release|Win32` or `Debug|Win32` builds the user-mode components. Choosing 'Kernel Release|Win32` or `Debug Release|Win32` builds the kernel-mode components.

> NOTE: I have decided not to upload the x64 version of XDK

How to Run
==========
The XDK solution comes with the following projects:

* `xStl`       - [xStl](https://github.com/eladraz/xStl) is a cross-compile, cross-platform C++ library that works in both user mode and kernel mode.

* `xdk_loader` - A user-mode library that implements driver-loading APIs.
* `Manager`    - A generic driver-loading application that uses xdk_loader (e.g. CLI front-end for driver loading).
* `PCPlayer`   - A dedicated user-mode application that loads PCSpeakerDriver from hardcoded location (`C:\Temp\PCSpeakerDriver.sys`), which demonstrates user mode => kernel mode commands and events. The application sends a few commands to the driver to turn on the PC speaker.

* `XDK`             - A fully featured C++ kernel mode library (including synchronization objects, threads and file access file-access APIs).
* `PCSpeakerDriver` - A sample driver for PC Speaker.

License
=======
Please see LICENSE file
