# Build

## Build dependencies

### Build libexpat
```
mkdir build-libexpat
cd build-libexpat
cmake ..\VisualStatistics\libexpat\expat -G "Visual Studio 15 2017 Win64"
msbuild /m /t:expat /p:Configuration=Release /p:Platform=x64 expat.sln
```
