# Counter file format
The "offset from utc" in header line is optional. Counter values are also optional. The date and time in each line represents the UTC time of the corresponding counters (since v2.2.0.1).
```
##date;time[offset from utc];counter1;...;countern##
dd.MM.yyyy;HH:mm:ss;value1;...;valuen
dd.MM.yyyy;HH:mm:ss;;...;valuen
```

Example:
```
##date;time28800;counter1;counter2;counter3##
17.12.2021;19:50:00;12;38;19
17.12.2021;19:55:00;18;;23
```

# Build for Windows

## Build dependencies
The following steps need MSVC compiler, so they should be executed in the Visual Studio Command Prompt CLI window.

### Build QScintilla
Download QScintilla from https://www.riverbankcomputing.com/static/Downloads/QScintilla/2.11.6/QScintilla-2.11.6.zip and follow the instructions in doc\html-Qt4Qt5\index.html to compile and install.

For example:
```powershell
cd Qt4Qt5
qmake CONFIG+=staticlib
nmake
nmake install
```

And then add `CONFIG += qscintilla2` to application's .pro file.

### Build breakpad
```powershell
# Comment out the test related dependencies in breakpad\src\client\windows\breakpad_client.gyp
        # './unittests/client_tests.gyp:*',
        # './unittests/testing.gyp:*',
        # './tests/crash_generation_app/crash_generation_app.gyp:*',

# Clone gyp to somewhere
git clone https://chromium.googlesource.com/external/gyp

# Add gyp to Path
set Path=<gyp dir>;%Path%

gyp.bat --no-circular-check breakpad\src\client\windows\breakpad_client.gyp -Dwin_release_RuntimeLibrary=2 -Dwin_debug_RuntimeLibrary=3

# Runtime library values:
# 0: /MT
# 1: /MTd
# 2: /MD
# 3: /MDd

# Trailing slash of OutDir is mandatory.
# Build Release version
msbuild /m /p:Configuration=Release /p:Platform=x64 /p:OutDir=%CD%\..\build-breakpad-Release\ breakpad\src\client\windows\breakpad_client.sln
# Build Debug version
msbuild /m /p:Configuration=Debug /p:Platform=x64 /p:OutDir=%CD%\..\build-breakpad-Debug\ breakpad\src\client\windows\breakpad_client.sln
```

### Build libexpat
```powershell
mkdir build-libexpat
cd build-libexpat
cmake ..\VisualStatistics\libexpat\expat -G "Visual Studio 15 2017 Win64"
msbuild /m /t:expat /p:Configuration=Release /p:Platform=x64 expat.sln
```

### Build pcre2
```powershell
mkdir build-pcre2
cd build-pcre2
cmake ..\VisualStatistics\pcre2 -G "Visual Studio 15 2017 Win64" -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF -DPCRE2_SUPPORT_JIT=ON
msbuild /m /t:pcre2-8-shared /p:Configuration=Release /p:Platform=x64 PCRE2.sln
```

### Build QuaZIP
```powershell
# Build Release version
mkdir build-quazip-Release
cd build-quazip-Release
qmake INCLUDEPATH+=<QTDIR>\include\QtZlib ..\VisualStatistics\quazip\quazip\quazip.pro
nmake
# Build Debug version
mkdir build-quazip-Debug
cd build-quazip-Debug
qmake CONFIG+=debug INCLUDEPATH+=<QTDIR>\include\QtZlib ..\VisualStatistics\quazip\quazip\quazip.pro
nmake
```

# Build for Linux

## Build dependencies

### Build QScintilla
Download QScintilla from https://www.riverbankcomputing.com/static/Downloads/QScintilla/2.11.6/QScintilla-2.11.6.tar.gz and follow the instructions in doc\html-Qt4Qt5\index.html to compile and install.

For example:
```bash
cd Qt4Qt5
qmake CONFIG+=staticlib
make
make install
```

### Build breakpad
```bash
cd VisualStatistics/breakpad
git clone https://chromium.googlesource.com/linux-syscall-support src/third_party/lss
./configure
make
```

### Build libexpat
```bash
cd VisualStatistics/libexpat/expat
./buildconf.sh
./configure
make
```

### Build pcre2
```bash
cd VisualStatistics/pcre2
./autogen.sh
./configure
make
```

### Build QuaZIP
```bash
# Build Release version
mkdir build-quazip-Release
cd build-quazip-Release
qmake ../VisualStatistics/quazip/quazip/quazip.pro
make
# Build Debug version
mkdir build-quazip-Debug
cd build-quazip-Debug
qmake CONFIG+=debug ../VisualStatistics/quazip/quazip/quazip.pro
make
```