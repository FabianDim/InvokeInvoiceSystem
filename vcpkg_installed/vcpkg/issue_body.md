Package: qt5-base:x64-windows@5.15.16

**Host Environment**

- Host: x64-windows
- Compiler: MSVC 19.44.35213.0
-    vcpkg-tool version: 2025-04-01-9c604254140797833b6f76908435c9fcbf09920e
    vcpkg-readonly: true
    vcpkg-scripts version: 4f8fe05871555c1798dbcb1957d0d595e94f7b57

**To Reproduce**

`vcpkg install `

**Failure logs**

```
CMake Warning at scripts/cmake/vcpkg_buildpath_length_warning.cmake:4 (message):
  qt5-base's buildsystem uses very long paths and may fail on your system.

  We recommend moving vcpkg to a short path such as 'C:\src\vcpkg' or using
  the subst command.
Call Stack (most recent call first):
  C:/Users/Fabian/AppData/Local/vcpkg/registries/git-trees/d207be89c93ec616752e5a19f97c2d794f117779/portfile.cmake:1 (vcpkg_buildpath_length_warning)
  scripts/ports.cmake:203 (include)


-- Figuring out qt target mkspec. Target arch x64
-- Target mkspec set to: win32-msvc
-- Host mkspec not set. Qt's own buildsystem will try to figure out the host system
CMake Warning at scripts/cmake/vcpkg_buildpath_length_warning.cmake:4 (message):
  qt5-base's buildsystem uses very long paths and may fail on your system.

  We recommend moving vcpkg to a short path such as 'C:\src\vcpkg' or using
  the subst command.
Call Stack (most recent call first):
  C:/Users/Fabian/AppData/Local/vcpkg/registries/git-trees/d207be89c93ec616752e5a19f97c2d794f117779/cmake/qt_download_submodule.cmake:13 (vcpkg_buildpath_length_warning)
  C:/Users/Fabian/AppData/Local/vcpkg/registries/git-trees/d207be89c93ec616752e5a19f97c2d794f117779/portfile.cmake:47 (qt_download_submodule)
  scripts/ports.cmake:203 (include)


Downloading qtbase-everywhere-opensource-src-5.15.16.tar.xz, trying https://download.qt.io/archive/qt/5.15/5.15.16/submodules/qtbase-everywhere-opensource-src-5.15.16.tar.xz
Successfully downloaded qtbase-everywhere-opensource-src-5.15.16.tar.xz
-- Extracting source C:/Users/Fabian/AppData/Local/vcpkg/downloads/qtbase-everywhere-opensource-src-5.15.16.tar.xz
-- Applying patch patches/0001-CVE-2023-51714-qtbase-5.15.diff
-- Applying patch patches/0002-CVE-2023-51714-qtbase-5.15.diff
-- Applying patch patches/CVE-2024-25580-qtbase-5.15.diff
-- Applying patch patches/CVE-2024-39936-qtbase-5.15.patch
-- Applying patch patches/winmain_pro.patch
-- Applying patch patches/windows_prf.patch
-- Applying patch patches/qt_app.patch
-- Applying patch patches/gui_configure.patch
-- Applying patch patches/xlib.patch
-- Applying patch patches/egl.patch
-- Applying patch patches/qtbug_96392.patch
-- Applying patch patches/mysql_plugin_include.patch
-- Applying patch patches/mysql-configure.patch
-- Applying patch patches/patch-qtbase-memory_resource.diff
-- Applying patch patches/Qt5BasicConfig.patch
-- Applying patch patches/Qt5PluginTarget.patch
-- Applying patch patches/create_cmake.patch
-- Applying patch patches/Qt5GuiConfigExtras.patch
-- Applying patch patches/fix_angle.patch
-- Applying patch patches/mingw9.patch
-- Applying patch patches/qmake-arm64.patch
-- Using source at C:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/vcpkg/blds/qt5-base/src/5.15.16-ce7e700ea9.clean
-- Found external ninja('1.12.1').
-- Getting CMake variables for x64-windows
-- Using cached msys2-mingw-w64-x86_64-pkgconf-1~2.3.0-1-any.pkg.tar.zst
-- Using cached msys2-msys2-runtime-3.5.4-2-x86_64.pkg.tar.zst
-- Using msys root at C:/Users/Fabian/AppData/Local/vcpkg/downloads/tools/msys2/21caed2f81ec917b
-- Configuring x64-windows-dbg
-- Configuring x64-windows-dbg done
-- Configuring x64-windows-rel
-- Configuring x64-windows-rel done
Downloading https://github.com/lexxmark/winflexbison/releases/download/v2.5.24/win_flex_bison-2.5.24.zip -> win_flex_bison-2.5.24.zip
Successfully downloaded win_flex_bison-2.5.24.zip
-- Building x64-windows-dbg
CMake Error at scripts/cmake/vcpkg_execute_build_process.cmake:134 (message):
    Command failed: C:/Users/Fabian/AppData/Local/vcpkg/downloads/tools/jom/jom-1_1_4/jom.exe /J 9
    Working Directory: C:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/vcpkg/blds/qt5-base/x64-windows-dbg
    See logs for more information:
      C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\build-x64-windows-dbg-out.log
      C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\build-x64-windows-dbg-err.log

Call Stack (most recent call first):
  C:/Users/Fabian/AppData/Local/vcpkg/registries/git-trees/d207be89c93ec616752e5a19f97c2d794f117779/cmake/install_qt.cmake:86 (vcpkg_execute_build_process)
  C:/Users/Fabian/AppData/Local/vcpkg/registries/git-trees/d207be89c93ec616752e5a19f97c2d794f117779/portfile.cmake:405 (install_qt)
  scripts/ports.cmake:203 (include)



```

<details><summary>C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\build-x64-windows-dbg-err.log</summary>

```

jom 1.1.4 - empower your cores

cl : Command line warning D9025 : overriding '/Fo./' with '/Foqlibraryinfo_final.obj'
WARNING: Failure to find: C:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/vcpkg/blds/qt5-base/x64-windows-dbg/src/corelib/qt5cored_metatypes.json
WARNING: Failure to find: C:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/vcpkg/blds/qt5-base/x64-windows-dbg/src/corelib/qt5core_metatypes.json
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\corelib\kernel\qobjectdefs.h(0): Note: No relevant classes found. No output generated.
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
...
Skipped 287 lines
...
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
WARNING: Failure to find: C:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/vcpkg/blds/qt5-base/x64-windows-dbg/src/widgets/qt5widgetsd_metatypes.json
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
jom: C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\src\platformsupport\fontdatabases\Makefile.Debug [.obj\debug\qwindowsfontdatabase.obj] Error 2
WARNING: Failure to find: C:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/vcpkg/blds/qt5-base/x64-windows-dbg/src/widgets/qt5widgets_metatypes.json
jom: C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\src\platformsupport\fontdatabases\Makefile.Debug [.obj\debug\qwindowsfontdatabase_ft.obj] Error 2
jom: C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\src\platformsupport\fontdatabases\Makefile [debug] Error 2
jom: C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\src\platformsupport\Makefile [sub-fontdatabases-make_first] Error 2
jom: C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\src\Makefile [sub-platformsupport-make_first] Error 2
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
cl : Command line warning D9025 : overriding '/Zi' with '/Z7'
jom: C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\Makefile [sub-src-make_first] Error 2
```
</details>

<details><summary>C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\build-x64-windows-dbg-out.log</summary>

```
	cd src\ && ( if not exist Makefile C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\bin\qmake.exe -o Makefile C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\src.pro ) && C:\Users\Fabian\AppData\Local\vcpkg\downloads\tools\jom\jom-1_1_4\jom.exe -f Makefile
	cd doc\ && ( if not exist Makefile C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\bin\qmake.exe -o Makefile C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\doc\doc.pro ) && C:\Users\Fabian\AppData\Local\vcpkg\downloads\tools\jom\jom-1_1_4\jom.exe -f Makefile
	C:\Users\Fabian\AppData\Local\vcpkg\downloads\tools\jom\jom-1_1_4\jom.exe -f Makefile.Debug
	cd tools\bootstrap\ && ( if not exist Makefile C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\bin\qmake.exe -o Makefile C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\tools\bootstrap\bootstrap.pro ) && C:\Users\Fabian\AppData\Local\vcpkg\downloads\tools\jom\jom-1_1_4\jom.exe -f Makefile
	cd qmake\ && ( if not exist Makefile.qmake-aux C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\bin\qmake.exe -o Makefile.qmake-aux C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\qmake\qmake-aux.pro ) && C:\Users\Fabian\AppData\Local\vcpkg\downloads\tools\jom\jom-1_1_4\jom.exe -f Makefile.qmake-aux
	C:\Users\Fabian\AppData\Local\vcpkg\downloads\tools\jom\jom-1_1_4\jom.exe -f Makefile.qmake-aux.Debug
	C:\Users\Fabian\AppData\Local\vcpkg\downloads\tools\jom\jom-1_1_4\jom.exe binary
	cl -Yuqmake_pch.h -FIqmake_pch.h -Fpqmake_pch.pch -c -Fo./ -Fdqmake.pdb  -W2 -nologo -O2  /MP /wd4577   -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\qmake -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\qmake\library -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\qmake\generators -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\qmake\generators\unix -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\qmake\generators\win32 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\qmake\generators\mac  -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean/src/3rdparty/tinycbor/src  -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean/include -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean/include\QtCore -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean/include\QtCore\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean/include\QtCore\5.15.16\QtCore  -I..\src\corelib\global  -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\mkspecs\win32-msvc   -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS  -DQT_VERSION_STR=\"5.15.16\" -DQT_VERSION_MAJOR=5 -DQT_VERSION_MINOR=15 -DQT_VERSION_PATCH=16  -DQT_BUILD_QMAKE -DQT_BOOTSTRAPPED -DPROEVALUATOR_FULL  -DQT_NO_FOREACH -DUNICODE -D_ENABLE_EXTENDED_ALIGNED_STORAGE  -Foqlibraryinfo_final.obj C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\corelib\global\qlibraryinfo.cpp
qlibraryinfo.cpp
	link  /OUT:..\qmake\qmake.exe project.obj main.obj ioutils.obj proitems.obj qmakevfs.obj  qmakeglobals.obj qmakeparser.obj qmakeevaluator.obj qmakebuiltins.obj  makefile.obj unixmake.obj unixmake2.obj mingw_make.obj  option.obj winmakefile.obj projectgenerator.obj property.obj meta.obj  makefiledeps.obj metamakefile.obj xmloutput.obj  msvc_nmake.obj msvc_vcproj.obj msvc_vcxproj.obj  msvc_objectmodel.obj msbuild_objectmodel.obj registry.obj qbitarray.obj  qbuffer.obj  qcryptographichash.obj  qfilesystementry.obj  qfilesystemengine.obj  qfilesystemengine_win.obj  qfilesystemiterator_win.obj  qfsfileengine.obj  qfsfileengine_iterator.obj  qarraydata.obj  qbytearray.obj  qbytearraylist.obj  qvsnprintf.obj  qbytearraymatcher.obj  qcalendar.obj  qdatetime.obj  qdir.obj  qdiriterator.obj  qfiledevice.obj  qfile.obj  qtemporaryfile.obj  qabstractfileengine.obj  qfsfileengine_win.obj  qsystemlibrary.obj  qfileinfo.obj  qendian.obj  qglobal.obj  qgregoriancalendar.obj  qhash.obj  qiodevice.obj  qringbuffer.obj  qdebug.obj  qlist.obj  qlocale.obj  qlocale_tools.obj  qlocale_win.obj  qversionnumber.obj  qmalloc.obj  qmap.obj  qoperatingsystemversion.obj  qoperatingsystemversion_win.obj  qregexp.obj  qromancalendar.obj  qutfcodec.obj  qstring.obj  qstringlist.obj  qstringbuilder.obj  qsystemerror.obj  qtextstream.obj  qdatastream.obj  quuid.obj  qrandom.obj  qsettings.obj  qvariant.obj  qsettings_win.obj  qmetatype.obj  qxmlstream.obj  qxmlutils.obj  qnumeric.obj  qlogging.obj  qcborstreamwriter.obj  qcborvalue.obj  qjsoncbor.obj  qjsondocument.obj  qjsonparser.obj  qjsonarray.obj  qjsonobject.obj  qjsonvalue.obj qlibraryinfo_final.obj qmake_pch.obj ole32.lib advapi32.lib shell32.lib netapi32.lib
Microsoft (R) Incremental Linker Version 14.44.35213.0
Copyright (C) Microsoft Corporation.  All rights reserved.

	C:\Users\Fabian\AppData\Local\vcpkg\downloads\tools\jom\jom-1_1_4\jom.exe -f Makefile.Release
	cl.exe -c -nologo -Zc:wchar_t -FS -Zc:rvalueCast -Zc:inline -Zc:strictStrings -Zc:throwingNew -Zc:referenceBinding -Zc:__cplusplus -O2 -Zi -MD -std:c++17 -utf-8 /wd4530 /wd4577 -W3 -w34100 -w34189 -w44996 -w44456 -w44457 -w44458 -wd4577 -wd4467 /Fd../../../lib/Qt5Bootstrap.pdb -DUNICODE -D_UNICODE -DWIN32 -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64 -DQT_NO_LINKED_LIST -DQT_NO_JAVA_STYLE_ITERATORS -DQT_VERSION_STR="\"5.15.16\"" -DQT_VERSION_MAJOR=5 -DQT_VERSION_MINOR=15 -DQT_VERSION_PATCH=16 -DQT_BOOTSTRAPPED -DQT_NO_CAST_TO_ASCII -DQT_NO_FOREACH -DQT_NO_CAST_FROM_ASCII -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT -DQT_BUILD_BOOTSTRAP_LIB -DQT_BUILDING_QT -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES -DQT_ASCII_CAST_WARNINGS -DQT_MOC_COMPAT -DQT_USE_QSTRINGBUILDER -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x040800 -DQT_DEPRECATED_WARNINGS_SINCE=0x060000 -DNDEBUG -DQT_NO_EXCEPTIONS -DQT_NO_DEBUG -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\tools\bootstrap -I. -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\tools -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\3rdparty\tinycbor\src -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore\5.15.16\QtCore -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtXml -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtXml\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtXml\5.15.16\QtXml -IC:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/vcpkg/blds/qt5-base/x64-windows-dbg/include -IC:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/vcpkg/blds/qt5-base/x64-windows-dbg/include/QtCore -IC:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/vcpkg/blds/qt5-base/x64-windows-dbg/include/QtXml -Itmp -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\x64-windows\include -IC:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/x64-windows/include/qt5 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\mkspecs\win32-msvc -Fo.obj\release\ @C:\Users\Fabian\AppData\Local\Temp\qlatincodec.obj.20948.265.jom
qlatincodec.cpp
...
Skipped 2845 lines
...
qitemeditorfactory.cpp
	cl.exe -c -FIkernel\qt_widgets_pch.h -Yukernel\qt_widgets_pch.h -Fp.pch\debug\Qt5Widgetsd_pch.pch -nologo -Zc:wchar_t -FS -Zc:rvalueCast -Zc:inline -Zc:strictStrings -Zc:throwingNew -Zc:referenceBinding -Zc:__cplusplus -Zi -MDd -nologo -DWIN32 -D_WINDOWS -utf-8 -GR -EHsc -MP -MDd -Z7 -Ob0 -Od -RTC1 -std:c++17 -utf-8 /wd4530 /wd4577 -W3 -w34100 -w34189 -w44996 -w44456 -w44457 -w44458 -wd4577 -wd4467 /Fd.obj\debug\Qt5Widgetsd.vc.pdb -DUNICODE -D_UNICODE -DWIN32 -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64 -DQT_NO_LINKED_LIST -DQT_NO_JAVA_STYLE_ITERATORS -DQT_NO_USING_NAMESPACE -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT -DQT_BUILD_WIDGETS_LIB -DQT_BUILDING_QT -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES -DQT_NO_CAST_TO_ASCII -DQT_ASCII_CAST_WARNINGS -DQT_MOC_COMPAT -DQT_USE_QSTRINGBUILDER -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x040800 -DQT_DEPRECATED_WARNINGS_SINCE=0x060000 -DQT_NO_EXCEPTIONS -DQT_GUI_LIB -DQT_CORE_LIB -D_WINDLL -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\widgets -I. -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\widgets\dialogs -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtWidgets -I..\..\include -I..\..\include\QtWidgets -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtWidgets\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtWidgets\5.15.16\QtWidgets -I..\..\include\QtWidgets\5.15.16 -I..\..\include\QtWidgets\5.15.16\QtWidgets -Itmp -I.tracegen\debug -I.uic -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtGui\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtGui\5.15.16\QtGui -I..\..\include\QtGui\5.15.16 -I..\..\include\QtGui\5.15.16\QtGui -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore\5.15.16\QtCore -I..\..\include\QtCore\5.15.16 -I..\..\include\QtCore\5.15.16\QtCore -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtGui -I..\..\include\QtGui -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore -I..\..\include\QtCore -I.moc\debug -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\x64-windows\include -IC:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/x64-windows/include/qt5 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\mkspecs\win32-msvc -Fo.obj\debug\ @C:\Users\Fabian\AppData\Local\Temp\qtableview.obj.16080.73907.jom
qtableview.cpp
	cl.exe -c -FIkernel\qt_widgets_pch.h -Yukernel\qt_widgets_pch.h -Fp.pch\debug\Qt5Widgetsd_pch.pch -nologo -Zc:wchar_t -FS -Zc:rvalueCast -Zc:inline -Zc:strictStrings -Zc:throwingNew -Zc:referenceBinding -Zc:__cplusplus -Zi -MDd -nologo -DWIN32 -D_WINDOWS -utf-8 -GR -EHsc -MP -MDd -Z7 -Ob0 -Od -RTC1 -std:c++17 -utf-8 /wd4530 /wd4577 -W3 -w34100 -w34189 -w44996 -w44456 -w44457 -w44458 -wd4577 -wd4467 /Fd.obj\debug\Qt5Widgetsd.vc.pdb -DUNICODE -D_UNICODE -DWIN32 -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64 -DQT_NO_LINKED_LIST -DQT_NO_JAVA_STYLE_ITERATORS -DQT_NO_USING_NAMESPACE -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT -DQT_BUILD_WIDGETS_LIB -DQT_BUILDING_QT -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES -DQT_NO_CAST_TO_ASCII -DQT_ASCII_CAST_WARNINGS -DQT_MOC_COMPAT -DQT_USE_QSTRINGBUILDER -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x040800 -DQT_DEPRECATED_WARNINGS_SINCE=0x060000 -DQT_NO_EXCEPTIONS -DQT_GUI_LIB -DQT_CORE_LIB -D_WINDLL -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\widgets -I. -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\widgets\dialogs -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtWidgets -I..\..\include -I..\..\include\QtWidgets -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtWidgets\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtWidgets\5.15.16\QtWidgets -I..\..\include\QtWidgets\5.15.16 -I..\..\include\QtWidgets\5.15.16\QtWidgets -Itmp -I.tracegen\debug -I.uic -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtGui\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtGui\5.15.16\QtGui -I..\..\include\QtGui\5.15.16 -I..\..\include\QtGui\5.15.16\QtGui -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore\5.15.16\QtCore -I..\..\include\QtCore\5.15.16 -I..\..\include\QtCore\5.15.16\QtCore -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtGui -I..\..\include\QtGui -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore -I..\..\include\QtCore -I.moc\debug -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\x64-windows\include -IC:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/x64-windows/include/qt5 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\mkspecs\win32-msvc -Fo.obj\debug\ @C:\Users\Fabian\AppData\Local\Temp\qlistwidget.obj.16080.74203.jom
qlistwidget.cpp
	C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\bin\moc.exe --collect-json -o qt5widgetsd_metatypes.json .moc\debug\moc_qaction.cpp.json .moc\debug\moc_qactiongroup.cpp.json .moc\debug\moc_qapplication.cpp.json .moc\debug\moc_qwidgetrepaintmanager_p.cpp.json .moc\debug\moc_qboxlayout.cpp.json .moc\debug\moc_qdesktopwidget.cpp.json .moc\debug\moc_qgridlayout.cpp.json .moc\debug\moc_qlayout.cpp.json .moc\debug\moc_qshortcut.cpp.json .moc\debug\moc_qsizepolicy.cpp.json .moc\debug\moc_qstackedlayout.cpp.json .moc\debug\moc_qwidget.cpp.json .moc\debug\moc_qwidget_p.cpp.json .moc\debug\moc_qwidgetaction.cpp.json .moc\debug\moc_qgesture.cpp.json .moc\debug\moc_qgesturemanager_p.cpp.json .moc\debug\moc_qdesktopwidget_p.cpp.json .moc\debug\moc_qwidgetwindow_p.cpp.json .moc\debug\moc_qwindowcontainer_p.cpp.json .moc\debug\moc_qopenglwidget.cpp.json .moc\debug\moc_qformlayout.cpp.json .moc\debug\moc_qstyle.cpp.json .moc\debug\moc_qstyleplugin.cpp.json .moc\debug\moc_qcommonstyle.cpp.json .moc\debug\moc_qproxystyle.cpp.json .moc\debug\moc_qstylesheetstyle_p.cpp.json .moc\debug\moc_qpixmapstyle_p.cpp.json .moc\debug\moc_qstyleanimation_p.cpp.json .moc\debug\moc_qwindowsstyle_p.cpp.json .moc\debug\moc_qfusionstyle_p.cpp.json .moc\debug\moc_qframe.cpp.json .moc\debug\moc_qabstractscrollarea.cpp.json .moc\debug\moc_qabstractscrollarea_p.cpp.json .moc\debug\moc_qfocusframe.cpp.json .moc\debug\moc_qwidgetanimator_p.cpp.json .moc\debug\moc_qabstractbutton.cpp.json .moc\debug\moc_qabstractslider.cpp.json .moc\debug\moc_qbuttongroup.cpp.json .moc\debug\moc_qcalendarwidget.cpp.json .moc\debug\moc_qcheckbox.cpp.json .moc\debug\moc_qcombobox.cpp.json .moc\debug\moc_qcombobox_p.cpp.json .moc\debug\moc_qcommandlinkbutton.cpp.json .moc\debug\moc_qdatetimeedit.cpp.json .moc\debug\moc_qdatetimeedit_p.cpp.json .moc\debug\moc_qdial.cpp.json .moc\debug\moc_qdockwidget.cpp.json .moc\debug\moc_qdockwidget_p.cpp.json .moc\debug\moc_qfontcombobox.cpp.json .moc\debug\moc_qgroupbox.cpp.json .moc\debug\moc_qkeysequenceedit.cpp.json .moc\debug\moc_qlabel.cpp.json .moc\debug\moc_qlcdnumber.cpp.json .moc\debug\moc_qlineedit.cpp.json .moc\debug\moc_qlineedit_p.cpp.json .moc\debug\moc_qwidgetlinecontrol_p.cpp.json .moc\debug\moc_qmainwindow.cpp.json .moc\debug\moc_qmainwindowlayout_p.cpp.json .moc\debug\moc_qmdiarea.cpp.json .moc\debug\moc_qmdisubwindow.cpp.json .moc\debug\moc_qmenu.cpp.json .moc\debug\moc_qmenubar.cpp.json .moc\debug\moc_qprogressbar.cpp.json .moc\debug\moc_qpushbutton.cpp.json .moc\debug\moc_qradiobutton.cpp.json .moc\debug\moc_qwidgetresizehandler_p.cpp.json .moc\debug\moc_qdialogbuttonbox.cpp.json .moc\debug\moc_qrubberband.cpp.json .moc\debug\moc_qscrollarea.cpp.json .moc\debug\moc_qscrollbar.cpp.json .moc\debug\moc_qsizegrip.cpp.json .moc\debug\moc_qslider.cpp.json .moc\debug\moc_qabstractspinbox.cpp.json .moc\debug\moc_qspinbox.cpp.json .moc\debug\moc_qsplashscreen.cpp.json .moc\debug\moc_qsplitter.cpp.json .moc\debug\moc_qstackedwidget.cpp.json .moc\debug\moc_qstatusbar.cpp.json .moc\debug\moc_qtabbar.cpp.json .moc\debug\moc_qplaintextedit.cpp.json .moc\debug\moc_qplaintextedit_p.cpp.json .moc\debug\moc_qtextedit.cpp.json .moc\debug\moc_qtextbrowser.cpp.json .moc\debug\moc_qtabwidget.cpp.json .moc\debug\moc_qtoolbar.cpp.json .moc\debug\moc_qtoolbarlayout_p.cpp.json .moc\debug\moc_qtoolbarseparator_p.cpp.json .moc\debug\moc_qtoolbox.cpp.json .moc\debug\moc_qtoolbutton.cpp.json .moc\debug\moc_qtoolbarextension_p.cpp.json .moc\debug\moc_qwidgettextcontrol_p.cpp.json .moc\debug\moc_qcolordialog.cpp.json .moc\debug\moc_qdialog.cpp.json .moc\debug\moc_qerrormessage.cpp.json .moc\debug\moc_qfiledialog.cpp.json .moc\debug\moc_qsidebar_p.cpp.json .moc\debug\moc_qfilesystemmodel.cpp.json .moc\debug\moc_qfileinfogatherer_p.cpp.json .moc\debug\moc_qfontdialog.cpp.json .moc\debug\moc_qinputdialog.cpp.json .moc\debug\moc_qmessagebox.cpp.json .moc\debug\moc_qprogressdialog.cpp.json .moc\debug\moc_qwizard.cpp.json .moc\debug\moc_qabstractitemview.cpp.json .moc\debug\moc_qheaderview.cpp.json .moc\debug\moc_qabstractitemdelegate.cpp.json .moc\debug\moc_qitemdelegate.cpp.json .moc\debug\moc_qitemeditorfactory_p.cpp.json .moc\debug\moc_qstyleditemdelegate.cpp.json .moc\debug\moc_qcolumnviewgrip_p.cpp.json .moc\debug\moc_qcolumnview.cpp.json .moc\debug\moc_qdatawidgetmapper.cpp.json .moc\debug\moc_qdirmodel.cpp.json .moc\debug\moc_qlistview.cpp.json .moc\debug\moc_qlistwidget.cpp.json .moc\debug\moc_qlistwidget_p.cpp.json .moc\debug\moc_qtableview.cpp.json .moc\debug\moc_qtablewidget.cpp.json .moc\debug\moc_qtablewidget_p.cpp.json .moc\debug\moc_qtreeview.cpp.json .moc\debug\moc_qtreewidget.cpp.json .moc\debug\moc_qtreewidget_p.cpp.json .moc\debug\moc_qgraphicsitem.cpp.json .moc\debug\moc_qgraphicsproxywidget.cpp.json .moc\debug\moc_qgraphicsscene.cpp.json .moc\debug\moc_qgraphicsscenebsptreeindex_p.cpp.json .moc\debug\moc_qgraphicssceneindex_p.cpp.json .moc\debug\moc_qgraphicsscenelinearindex_p.cpp.json .moc\debug\moc_qgraphicstransform.cpp.json .moc\debug\moc_qgraphicsview.cpp.json .moc\debug\moc_qgraphicswidget.cpp.json .moc\debug\moc_qgraphicsanchorlayout.cpp.json .moc\debug\moc_qgraphicsitemanimation.cpp.json .moc\debug\moc_qsystemtrayicon.cpp.json .moc\debug\moc_qsystemtrayicon_p.cpp.json .moc\debug\moc_qcompleter.cpp.json .moc\debug\moc_qcompleter_p.cpp.json .moc\debug\moc_qscroller.cpp.json .moc\debug\moc_qscroller_p.cpp.json .moc\debug\moc_qflickgesture_p.cpp.json .moc\debug\moc_qundostack.cpp.json .moc\debug\moc_qundostack_p.cpp.json .moc\debug\moc_qundogroup.cpp.json .moc\debug\moc_qundoview.cpp.json .moc\debug\moc_qkeyeventtransition.cpp.json .moc\debug\moc_qmouseeventtransition.cpp.json .moc\debug\moc_qbasickeyeventtransition_p.cpp.json .moc\debug\moc_qbasicmouseeventtransition_p.cpp.json .moc\debug\moc_qgraphicseffect.cpp.json .moc\debug\moc_qgraphicseffect_p.cpp.json .moc\debug\moc_qpixmapfilter_p.cpp.json .moc\debug\qwidgetrepaintmanager.moc.json .moc\debug\qtooltip.moc.json .moc\debug\qwhatsthis.moc.json .moc\debug\qcalendarwidget.moc.json .moc\debug\qdockwidget.moc.json .moc\debug\qeffects.moc.json .moc\debug\qfontcombobox.moc.json .moc\debug\qmdisubwindow.moc.json .moc\debug\qmenu.moc.json .moc\debug\qtabbar.moc.json .moc\debug\qtoolbox.moc.json .moc\debug\qcolordialog.moc.json .moc\debug\qfontdialog.moc.json .moc\debug\qinputdialog.moc.json .moc\debug\qmessagebox.moc.json .moc\debug\qitemeditorfactory.moc.json .moc\debug\qlistwidget.moc.json .moc\debug\qtableview.moc.json .moc\debug\qundoview.moc.json
	cl.exe -c -FIkernel\qt_widgets_pch.h -Yukernel\qt_widgets_pch.h -Fp.pch\debug\Qt5Widgetsd_pch.pch -nologo -Zc:wchar_t -FS -Zc:rvalueCast -Zc:inline -Zc:strictStrings -Zc:throwingNew -Zc:referenceBinding -Zc:__cplusplus -Zi -MDd -nologo -DWIN32 -D_WINDOWS -utf-8 -GR -EHsc -MP -MDd -Z7 -Ob0 -Od -RTC1 -std:c++17 -utf-8 /wd4530 /wd4577 -W3 -w34100 -w34189 -w44996 -w44456 -w44457 -w44458 -wd4577 -wd4467 /Fd.obj\debug\Qt5Widgetsd.vc.pdb -DUNICODE -D_UNICODE -DWIN32 -D_ENABLE_EXTENDED_ALIGNED_STORAGE -DWIN64 -DQT_NO_LINKED_LIST -DQT_NO_JAVA_STYLE_ITERATORS -DQT_NO_USING_NAMESPACE -DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT -DQT_BUILD_WIDGETS_LIB -DQT_BUILDING_QT -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES -DQT_NO_CAST_TO_ASCII -DQT_ASCII_CAST_WARNINGS -DQT_MOC_COMPAT -DQT_USE_QSTRINGBUILDER -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE=0x040800 -DQT_DEPRECATED_WARNINGS_SINCE=0x060000 -DQT_NO_EXCEPTIONS -DQT_GUI_LIB -DQT_CORE_LIB -D_WINDLL -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\widgets -I. -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\src\widgets\dialogs -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtWidgets -I..\..\include -I..\..\include\QtWidgets -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtWidgets\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtWidgets\5.15.16\QtWidgets -I..\..\include\QtWidgets\5.15.16 -I..\..\include\QtWidgets\5.15.16\QtWidgets -Itmp -I.tracegen\debug -I.uic -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtGui\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtGui\5.15.16\QtGui -I..\..\include\QtGui\5.15.16 -I..\..\include\QtGui\5.15.16\QtGui -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore\5.15.16 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore\5.15.16\QtCore -I..\..\include\QtCore\5.15.16 -I..\..\include\QtCore\5.15.16\QtCore -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtGui -I..\..\include\QtGui -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\include\QtCore -I..\..\include\QtCore -I.moc\debug -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\x64-windows\include -IC:/Users/Fabian/dev/repos/InvokeInvoiceSystem2/vcpkg_installed/x64-windows/include/qt5 -IC:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\src\5.15.16-ce7e700ea9.clean\mkspecs\win32-msvc -Fo.obj\debug\ @C:\Users\Fabian\AppData\Local\Temp\qundoview.obj.16080.74938.jom
qundoview.cpp
	C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\bin\qmake.exe -install qinstall C:\Users\Fabian\dev\repos\InvokeInvoiceSystem2\vcpkg_installed\vcpkg\blds\qt5-base\x64-windows-dbg\src\widgets\qt5widgetsd_metatypes.json ..\..\lib\metatypes\qt5widgetsd_metatypes.json
	link.exe /NOLOGO /DYNAMICBASE /NXCOMPAT -machine:x64 -nologo -debug -INCREMENTAL /DEBUG /DLL /SUBSYSTEM:WINDOWS -machine:x64 -nologo -debug -INCREMENTAL /VERSION:5.15 /MANIFEST:embed /OUT:..\..\lib\Qt5Widgetsd.dll @C:\Users\Fabian\AppData\Local\Temp\Qt5Widgetsd.dll.16080.76313.jom
   Creating library ..\..\lib\Qt5Widgetsd.lib and object ..\..\lib\Qt5Widgetsd.exp
	copy /y ..\..\lib\Qt5Widgetsd.dll ..\..\bin
        1 file(s) copied.
```
</details>

**Additional context**

<details><summary>vcpkg.json</summary>

```
{
  "name": "invoke-invoice-system",
  "version-string": "0.1.0",
  "dependencies": [
    "fmt",
    "gtest",
    "jansson",
    "libharu",
    "libiconv",
    "mongo-cxx-driver",
    "qtkeychain"
  ]
}

```
</details>
