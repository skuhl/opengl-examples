===
=== Getting started
===

Unless you are certain Delete any cmake files to ensure we start from a fresh copy

    Linux & macOS: ./cleanup.sh
    Windows: Follow instructions in WINDOWS-VisualStudio.txt, not this file!

Search for required/optional libraries and create a Makefile.

    cmake .

If cmake indicates that an error occurred, you probably do not have
all of the libraries that this code requires. If you are missing a
library, look for additional help in this file or in other files in
this directory.

Try to compile the programs

    make

Try running a program (all programs will be placed in the "bin" directory)

    ./triangle

Check out model loading and texturing capabilities

    ./viewer --fit ../models/duck/duck.dae

Run code for an HMD (left/right eye screens side-by-side):

    ./triangle --config config/hmd.ini

Run the code using the Oculus HMD

    ./triangle --config config/oculus.ini

Run the code with red/cyan anaglyph 3D:

    ./triangle --config config/anaglyph.ini

TODO: The remainder of this section is out of date!
		
If you want to want to use VRPN to control the position and
orientation of the camera, you need to:
    
    - Edit ~/.vrpn-server so that it contains the IP address or
      hostname of the VRPN server.

    - Set the VIEWMAT_VRPN_OBJECT environment variable to the name of
      the tracked object.

    - Run the program (such as "viewer") that you want to run.


Michigan Tech: Run code on the IVS display wall (if you have permission to do so)

    ./ivs.sh ./triangle

Michigan Tech: Test to see if program will work on IVS without access to the IVS display wall.

    ./ivs-test.sh ./triangle

Michigan Tech: Run code on the IVS display wall and update the viewport based on tracking the user's head (if you have permission to do so)

    ./ivs-track.sh ./triangle

Michigan Tech: Run code on the IVS display wall and account for the screen bezels (if you have permission to do so)

    ./ivs3.sh ./triangle



===
=== Libraries used by these programs
===

OpenGL 3.2 or higher (required) - OpenGL is available on all Linux
machines. To check the version of OpenGL on your computer, run
"glxinfo | grep version". Ignore any lines that say "glx" and the
other version numbers should indicate which version of OpenGL your
machine supports. The program "glxgears" is also typically available
on Linux. It is a simple OpenGL program that shows spinning gears and
can be used to verify that OpenGL is working properly on your machine.

GLEW (required) - Some functions are only available in newer versions
of OpenGL. When you write a modern OpenGL program, many of the
functions that you call did not exist in older versions of
OpenGL. GLEW allows you to access those functions easily without extra
work on your part. It also makes it easier to determine which OpenGL
features are supported on the computer you are running the program on.

 - For a guide to install GLEW, see: GLEW-INSTALL.txt
 - Homepage: http://glew.sourceforge.net/
 - License: https://github.com/nigels-com/glew/blob/master/LICENSE.txt

GLFW (required) - OpenGL doesn't provide a cross-platform way to
create a window that you can draw on. GLFW is one cross platform
library that makes it relatively easy to create windows, handle
mouse/keyboard inputs, etc. A older, but similar, alternative is GLUT
or freeglut. To install GLFW, look for packages named glfw, libglfw,
libglfw-lib in your package manager and install those packages.

 - For a guide to install GLFW, see GLFW3-INSTALL.txt
 - Homepage: http://www.glfw.org/
 - License: http://www.glfw.org/license.html

ImageMagick's MagickCore library (optional) - By default, this
codebase uses STB (code included in this repository) to load some
types of JPG, PNG and other file formats. However, ImageMagick
supports nearly every possible file format.

 - For a guide to install ImageMagick, see IMAGEMAGICK-INSTALL.txt
 - Homepage: http://www.imagemagick.org/
 - License: http://www.imagemagick.org/script/license.php

ASSIMP (optional, necessary to load 3D model files)
 - For a guide to install ASSIMP, see ASSIMP-INSTALL.txt
 - Homepage: http://assimp.sourceforge.net/
 - License: http://assimp.sourceforge.net/main_license.html

FFmpeg (optional, necessary to load video files)
 - For a guide to install FFmpeg, see FFMPEG-INSTALL.txt
 - Homepage: https://www.ffmpeg.org/
 - License: https://www.ffmpeg.org/legal.html

VRPN (optional, necessary to communicate with a VRPN tracking system server)
 - For a guide to install VRPN, see VRPN-INSTALL.txt
 - Homepage: http://www.cs.unc.edu/Research/vrpn/
 - License: http://www.cs.unc.edu/Research/vrpn/obtaining_vrpn.html

LibOVR (optional, necessary to run this code with the Oculus HMD)
 - For a guide to install LibOVR, see OCULUS-INSTALL.txt
 - License: https://developer.oculus.com/licenses/  (exact license depends on SDK version)

===
=== Compilation system
===

cmake - Cmake is a modern compilation system that is now easily
installed on nearly any Linux machine via the package manager. You
need to download and install this before you can compile the programs
in this package.

===
=== Units & coordinate systems
===

All of this software is written such that 1 unit distance is 1
meter. Any models we load should be in meters and the tracking system
returns results in meters. We always assume OpenGL coordinate system
(X=right, Y=up, Z=out of screen toward viewer).

On the display wall, the origin is at the center of the room and the
screen is at approximately z = -3.5 meters. If you are manually
placing the camera (i.e., with the mouse) without using the tracking
system on the display wall, I'd suggest placing the camera at (0, 1.5,
0) (approximate eye height), have it look down -z, and place your
object so that it has z values less than -3.5. Then, if you view the
program later on IVS with head-tracking enabled, the view that the
user will see will be approximately the same.

===
=== Documentation
===

Make sure you have "doxygen" installed on your system (check if the
command exists on the command line). If you have doxygen installed,
you should be able to:

1)  Run "make docs" to create a folder named doxygen-docs.
2) Open "doxygen-docs/html/index.html" in your favorite web browser.
3) Click on the "Files" tab.

It is generally recommended that you look at the documentation for the
headers for the files. If you look at the documentation for the C
files, it will show you documentation for variables and functions that
you might not have access to from outside of that C file.


===
=== Comparing files
===

Use the "meld" program to compare two different files in a
user-friendly way. For example, if you want to see how two files
differ from each other, try "meld triangle.c
triangle-color.c".
