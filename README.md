# Counter file format

The "offset from utc" in header line is optional. Counter values are also optional.
```
##date;time[offset from utc];counter1;...;countern##
dd.MM.yyyy;HH:mm:ss;value1;...;valuen
dd.MM.yyyy;HH:mm:ss;;...;valuen
```

Sample
```
##date;time28800;counter1;counter2;counter3##
17.12.2021;19:50:00;12;38;19
17.12.2021;19.55;00;18;;23
```

# Build

## Build dependencies

### Build breakpad
```
# Comment out the test related dependencies in breakpad\src\client\windows\breakpad_client.gyp
        # './unittests/client_tests.gyp:*',
        # './unittests/testing.gyp:*',
        # './tests/crash_generation_app/crash_generation_app.gyp:*',

# Clone gyp to somewhere
git clone https://chromium.googlesource.com/external/gyp

# Open Visual Studio Command Prompt and add gyp to PATH
set PATH=<gyp dir>;%PATH%

gyp.bat --no-circular-check breakpad\src\client\windows\breakpad_client.gyp -Dwin_release_RuntimeLibrary=2 -Dwin_debug_RuntimeLibrary=3

# Runtime library values:
# 0: /MT
# 1: /MTd
# 2: /MD
# 3: /MDd

# Trailing slash of OutDir is mandatory
msbuild /m /p:Configuration=Release /p:Platform=x64 /p:OutDir=%CD%\..\build-breakpad\ breakpad\src\client\windows\breakpad_client.sln
```

### Build libexpat
```
mkdir build-libexpat
cd build-libexpat
cmake ..\VisualStatistics\libexpat\expat -G "Visual Studio 15 2017 Win64"
msbuild /m /t:expat /p:Configuration=Release /p:Platform=x64 expat.sln
```

### Build pcre2
```
mkdir build-pcre2
cd build-pcre2
cmake ..\VisualStatistics\pcre2 -G "Visual Studio 15 2017 Win64" -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF -DPCRE2_SUPPORT_JIT=ON
msbuild /m /t:pcre2-8-shared /p:Configuration=Release /p:Platform=x64 PCRE2.sln
```
