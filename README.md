# Build

## Build dependencies

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
