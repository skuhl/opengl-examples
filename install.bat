setlocal
@echo off
:: Install Git, CMake, MinGW, and the DirectX SDK (you also might need to set an env var)
set repository_folder=%~dp0
set libraries_folder=%1
set mingw_folder=%2

if [%libraries_folder%]==[] (
	echo Must have valid libraries folder
	goto :eof
)
if NOT EXIST %libraries_folder%  (
	echo Must have valid libraries folder
	goto :eof
)
if [%mingw_folder%]==[] (
	echo Must have valid MinGW folder
	goto :eof
)
if NOT EXIST %mingw_folder%  (
	echo Must have valid MinGW folder
	goto :eof
)

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
mingw32-make
copy /y %libraries_folder%freeglut-3.0.0\bin\libfreeglut.dll %repository_folder%libfreeglut.dll
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
copy /y %libraries_folder%glew-1.12.0\lib\glew32.dll %repository_folder%glew32.dll
popd

:: Install assimp
pushd %libraries_folder%
git clone https://github.com/assimp/assimp
pushd assimp
cmake -G "MinGW Makefiles" .
mingw32-make
copy /y %libraries_folder%assimp\bin\libassimp.dll %repository_folder%libassimp.dll
popd

pushd %repository_folder%
:: pushd lib
:: wget --no-check-certificate https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
:: wget --no-check-certificate https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
:: popd

:: Download ZLib binaries
wget --no-check-certificate http://downloads.sourceforge.net/project/mingw/MinGW/Extension/zlib/zlib-1.2.5-1/libz-1.2.5-1-mingw32-dll-1.tar.lzma?r=&ts=1426965944&use_mirror=tcpdiag
7z x libz-1.2.5-1-mingw32-dll-1.tar.lzma
7z x libz-1.2.5-1-mingw32-dll-1.tar
rm libz-1.2.5-1-mingw32-dll-1.tar*
move /y bin\libz-1.dll libz-1.dll
wget --no-check-certificate http://downloads.sourceforge.net/project/mingw/MinGW/Extension/bzip2/bzip2-1.0.6-4/libbz2-1.0.6-4-mingw32-dll-2.tar.lzma?r=&ts=1426965980&use_mirror=hivelocity
7z x libbz2-1.0.6-4-mingw32-dll-2.tar.lzma
7z x libbz2-1.0.6-4-mingw32-dll-2.tar
rm libbz2-1.0.6-4-mingw32-dll-2.tar*
move /y bin\libbz2-2.dll libbz2-2.dll

rmdir bin

popd

endlocal

pause