![unmaintained](http://img.shields.io/badge/status-unmaintained-red.png)

Morph .NET Compiler
===================
Welcome to Morph!

Morph is a unique .NET to native compiler. It translates Microsoft intermediate language (MSIL) applications into a small static library that can be used with ANY operating system and in any environment. 
The main features of the compiler are:
* Includes a CLR library that is completely written in C#, including memory allocation and garbage collection.
* Generates a **small footprint** for small MCU usage (Cortex ARM0).
* Compatible with gcc cross-compilers and [IAR](http://www.iar.com/ewarm/) compiler.
* Offers a complete interoperability with C code (.NET calls to native C and native C calls to .NET).

Requirements
============
Windows
-------
Need [Visual Studio](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx) Express 2010 or higher, or Visual Studio 2010 or higher. Visual Studio Community is recommended.

Ubuntu (Debian)
---------------
* Building tools
`sudo apt-get install automake autoconf libtool build-essential git-core`

[Mono](http://www.mono-project.com/) installation
`sudo apt-get install mono-devel mono-xbuild mono-gmcs`

CentOS/Fedora/Redhat
--------------------
* Building tools
    `sudo yum install autoconf automake gcc-c++`
    `sudo yum install glibc-devel.i686 libgcc.i686 libstdc++-devel.i686`

* [Mono](http://www.mono-project.com/) installation
    `sudo yum install mono-devel`

> ***Some distributions of CentOS have a problem with libstd++ (need to check). For some reason the symbol __cxa_get_exception_ptr is not defined. ***

ArchLinux
---------
* Building tools
    `sudo pacman -S base-devel automake autoconf git`
    `sudo pacman -S gcc-multilib libtool-multilib binutils-multilib`
* [Mono](http://www.mono-project.com/) installation
    `sudo pacman -S monodevelop`

Cygwin
------
Install [cygwin](http://www.cygwin.com/) with the following packages:
* gcc
* g++
* automake
* autoconf
* libtool
* mono

Setting Up Environment
======================
Git
---
>
```
git clone https://github.com/eladraz/xStl
git clone https://github.com/eladraz/pe
git clone https://github.com/eladraz/elf
git clone https://github.com/eladraz/dismount
git clone https://github.com/eladraz/morph
```
>

How to Build
============
Windows
-------
In order to pass variable arguments to [Visual Studio](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx), you need to declare the following system arguments:
* XSTL_PATH (for example: `C:\WORK\github\xStl`)
* PELIB_PATH (for example: `C:\WORK\github\pe`)
* ELFLIB_PATH (for example: `C:\WORK\github\elf`)
* DISMOUNT_PATH (for example: `C:\WORK\github\dismount`)
* MORPH_PATH (for example: `C:\WORK\github\morph`)

> To add system variables you should:
>> * Right-click **My Computer**, and then click **Properties**.
>> * Click the **Advanced** tab.
>> * Click **Environment variables**.
>> * Click one of the following options, for either a user or a system variable:
>>    * Click *New* to add a new variable name and value.
>>    * Click an existing variable, and then click *Edit* to change its name or value.
>>    * Click an existing variable, and then click *Delete* to remove it.

In order to build the Morph compiler, open `Clr.sln` solution project with [Visual Studio](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx).
In Visual Studio's configuration manager, choose the desired build configuration (Win32/x64/Debug/Release) and build `clr_console`. 


Linux
-----
### Compilation Shortcut (build.sh script)
Set up the following environment variables:
>
```
export XSTL_PATH=`pwd`/xStl
export PELIB_PATH=`pwd`/pe
export ELFLIB_PATH=`pwd`/elf
export DISMOUNT_PATH=`pwd`/dismount
```

Build all projects (the default configuration of the compilation script is the debug/unicode version):
>
```
cd xStl
./build.sh
cd ../
```
>
```
cd pe
./build.sh
cd ../
```
>
```
cd elf
./build.sh
cd ../
```
>
```
cd dismount
./build.sh
cd ../
```
>
```
cd clr
./build.sh
cd ../
```

### Compile
*** Please see xStl, pelib, elflib and dismount libraries for compilation instructions. ***

1. Run `./autogen.sh` in order to generate `Makefile.in` configure script.
2. Run `./configure` to generate `Makefile` (See "Configure Argument" section, below).
3. Run `make` and `make install`.

#### Configure Argument
** Please make sure that xStl, pelib, elflib and dismount compile using the same settings. **
```
* --enable-debug      Compile with debugging flags
* --enable-unicode    Compile with UNICODE support
* --enable-tests      Compile tdump tool and compile with debug traces
* --with-xstl         Must be set with xStl location
* --with-pelib        Must be set with pelib location
* --with-elflib       Must be set with elflib location
* --with-dismount     Must be set with dismount location
```

##### Cross-Compile
In order to cross-compile (ARM, for example):
```
./autogen.sh
./configure --build= --host=arm-none-linux-gnueabi --with-xstl=${XSTL_PATH} --with-pelib=${PELIB_PATH} --with-elflib=${ELFLIB_PATH} --with-dismount=${DISMOUNT_PATH}
make
make install
```
Please note that it's important to have a PATH to the cross-compiler bin folder (e.g. `arm-none-linux-gnueabi-g++`).
> Android SDK PATH located at: `mydroid/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/`.
> However, for some reason Android cross-compile uses local `ld` instead of cross-compile `ld`.

Usage
=====
```
Usage: bin\exeDebug\clr_console\clr_console.exe [options] <clrcore.dll path> <.NET PE file-name>
  Where options may be:
  -p <path>   Specify the path to the precompiled repository file.
  -o <type>   Specify output type. This option may be specified more than once.
              If not specified, the default is both x86 and x86-mem. Possible outputs are:
                  x86   Create an ELF output.o file compiled for x86
                  x86-mem   Create memory-linked x86 code and execute it
                  32c   Create a C/C++ language output.c file
                  arm   Create an ARM-compiled output file
                  thumb   Create a THUMB-compiled output file
  -c <param>  Override a compiler parameter. This option may be specified more than once.
              Possible compiler parameters are:
                  eh+   Enable exception handling (default)
                  eh-   Disable exception handling (might be required for some output types)
                  opt+   Enable compiler optimizations
                  opt-   Disable compiler optimizations (default)
                  dev   Enable developer-level verbosity of output traces (debug builds only)

Example 1: This will compile the compiler-test program and then run it from memory:
  clr_console.exe -o x86-mem clrcore.dll TestSimpleCompiler1.exe

Example 2: This will compile the compiler-test program into an ARM object file using a precompiled repository, but without exception handling support:
  clr_console.exe -p precomp.dat -c eh- -o arm clrcore.dll TestSimpleCompiler1.exe
```

In order to run Morph you should compile the clrcore.dll, which is the framework.
Use Mono's xbuild (`xbuild netcore/corelib.sln`) to compile the `netcore/corelib.sln` solution, or use [Visual Studio](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx) by opening the C# project `netcore/corelib.sln` and compiling it.
> The library file will be located at `netcore/clr/clrcore/bin/***<Debug/Release>***/clrcore.dll`.

Compile one of the test solutions such as `tests\NET\TestSimpleCompiler1\TestSimpleCompiler1.sln` (under `tests\NET\`).
This can be done either by running `xbuild` or by using Visual Studio.

In order to compile using Morph compiler, please execute:

*Linux*: `mcc -o x86 netcore/clr/clrcore/bin/<Debug/Release>/clrcore.dll TestSimpleCompiler1.exe`
*Windows*: `clr_console.exe -o x86 netcore\clr\clrcore\bin\<Debug/Release>\clrcore.dll tests\NET\TestSimpleCompiler1\bin\Debug\TestSimpleCompiler1.exe`

The output file is `output.o`.

For x86 32-bit Systems
----------------------
The mcc `-o x86-mem` will link and run the output file from within the memory using the `executer\runtime\RuntimeClasses` library. 

Linking
=======
When compiling the project with `-o x86` or `-o arm` or `-o thumb`, the result file is a simple ELF .o file.
In order to link the resulted ELF file, simply link it with the relevant wrapper functions:

Cygwin
------
>
```gcc -m32 output.o netcore/wrapper/wrapper.c -o myprog.exe
./my_prog.exe
```

Linux
-----
>
```gcc -m32 output.o netcore/wrapper/wrapper.c -DLINUX_VER -g -o my_prog
./my_prog
```


Full Example
============
Linux
-----
Build a sample application using [Mono](http://www.mono-project.com/) or Visual Studio:
`xbuild tests/NET/TestSimpleCompiler1/TestSimpleCompiler1.sln`

Execute the Morph compiler:
`./out/bin/mcc -o x86 tests/NET/TestSimpleCompiler1/bin/Debug/clrcore.dll tests/NET/TestSimpleCompiler1/bin/Debug/TestSimpleCompiler1.exe`

Link to get a full executable:
`gcc -m32 output.o netcore/wrapper/wrapper.c -DLINUX_VER -g -o my_prog`

Execute!
`./my_prog`

Known Issues
============
This project is at its beginning, so there are several key features that are missing. Here is a quick list of the main features that Morph currently lacks:
* ARM and THUMB throw exceptions when the function is too big for relocation. This is an easy fix (Will be delivered in the next release).
* There is no support of .NET 64-bit variables (System.Int64, System.Uint64, long).
* There is no floating point support.
* 32c compiler does not work with exception handling (in order to compile an application you must provide the `eh-` argument).
* The `clrcore` framework is very minimal. The idea is to implement a core library in C# and use an existing library such as Mono (Ximian/Novel's BSD library) on top of it. However, since I am currently in the process of stabilizing the code, a minimal amount of Mono's code is mixed inside the library and not as an overlay. This will be changed, hopefully in the next releases.

License
=======
Please see LICENSE file
