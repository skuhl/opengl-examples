:: TODO: Figure out how to tell 3rd party library CMake files to not build their tools.
setlocal
@echo off
:: Install Git, CMake, MinGW, and the DirectX SDK (you also might need to set an env var)
set repository_folder=%~dp0\..\
set /p libraries_folder="Enter Directory for Library Installation: "
set /p mingw_folder="Enter Directory of MinGW Installation: "

if [%libraries_folder%]==[] (
	echo Must have valid libraries folder
	goto :eof
)
if NOT EXIST %libraries_folder%  (
	echo Library folder must exist
	goto :eof
)
if [%mingw_folder%]==[] (
	echo Must have valid MinGW folder
	goto :eof
)
if NOT EXIST %mingw_folder%  (
	echo MinGW folder must exist
	goto :eof
)

:: Ensure the path variable have mingw and msys on top
set path=%mingw_folder%bin;%mingw_folder%msys\1.0\bin;%path%

pushd %repository_folder%

:: Install freeglut
pushd %libraries_folder%
wget --no-check-certificate http://downloads.sourceforge.net/project/freeglut/freeglut/3.0.0/freeglut-3.0.0.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Ffreeglut%2Ffiles%2Ffreeglut%2F3.0.0%2F&ts=1426962454&use_mirror=iweb
7z x freeglut-3.0.0.tar.gz
7z x freeglut-3.0.0.tar
rm freeglut-3.0.0.tar*
pushd freeglut-3.0.0
cmake -G "MinGW Makefiles" .
cmake -G "MinGW Makefiles" .
mingw32-make
copy /y %libraries_folder%freeglut-3.0.0\bin\libfreeglut.dll %repository_folder%bin\libfreeglut.dll
popd

:: Install glew
pushd %libraries_folder%
wget --no-check-certificate https://sourceforge.net/projects/glew/files/glew/1.12.0/glew-1.12.0.tgz/download
7z x glew-1.12.0.tgz
7z x glew-1.12.0.tar
rm glew-1.12.0.t*
pushd glew-1.12.0
gcc -DGLEW_NO_GLU -O2 -Wall -W -Iinclude  -DGLEW_BUILD -o src/glew.o -c src/glew.c
gcc -shared -Wl,-soname,libglew32.dll -Wl,--out-implib,lib/libglew32.dll.a    -o lib/glew32.dll src/glew.o -L/mingw/lib -lglu32 -lopengl32 -lgdi32 -luser32 -lkernel32
ar cr lib/libglew32.a src/glew.o
gcc -DGLEW_NO_GLU -DGLEW_MX -O2 -Wall -W -Iinclude  -DGLEW_BUILD -o src/glew.mx.o -c src/glew.c
gcc -shared -Wl,-soname,libglew32mx.dll -Wl,--out-implib,lib/libglew32mx.dll.a -o lib/glew32mx.dll src/glew.mx.o -L/mingw/lib -lglu32 -lopengl32 -lgdi32 -luser32 -lkernel32
ar cr lib/libglew32mx.a src/glew.mx.o
copy /y %libraries_folder%glew-1.12.0\lib\glew32.dll %repository_folder%bin\glew32.dll
popd

:: Install zlib
pushd %libraries_folder%
git clone https://github.com/madler/zlib.git
pushd zlib
cmake -G "MinGW Makefiles" .
cmake -G "MinGW Makefiles" .
mingw32-make
copy /y %libraries_folder%zlib\libzlib.dll %repository_folder%bin\libzlib.dll
popd
set ZLIB_HOME=%libraries_folder%zlib

:: Install assimp
pushd %libraries_folder%
git clone https://github.com/assimp/assimp
pushd assimp
cmake -G "MinGW Makefiles" .
cmake -G "MinGW Makefiles" .
mingw32-make
copy /y %libraries_folder%assimp\bin\libassimp.dll %repository_folder%bin\libassimp.dll
popd

:: Install freetype
pushd %libraries_folder%
git clone https://github.com/winlibs/freetype
pushd freetype
cmake -G "MinGW Makefiles" .
cmake -G "MinGW Makefiles" .
mingw32-make
popd

endlocal

pause