```
mkdir build-pcre2
cd build-pcre2
cmake ..\VisualStatistics\pcre2 -G "Visual Studio 15 2017 Win64" -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF
msbuild /m /t:pcre2-8-shared /p:Configuration=Release /p:Platform=x64 PCRE2.sln
```