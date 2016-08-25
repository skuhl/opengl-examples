OpenGL examples
===============

This code provides a library to support the development of basic OpenGL and virtual reality applications. It was developed at Michigan Technological University for interactive computer graphics and virtual reality courses. Features include:

* Cross platform: Runs on Linux, Mac OS X, and Windows.
* Image loading: Loads a few image formats by default (JPEG, PNG, etc). If ImageMagick is present, nearly any image file format can be loaded (ImageMagick has been tested on Linux and Mac OS X).
* Image writing: Can save arbitrary data into an image. For example, you can record screenshots of every frame to disk and use a utility to combine them into a video file.
* 3D model loading: Numerous types of 3D model files can be loaded via the Assimp library. Also supports animated characters.
* Video file loading: Videos can be displayed as textures if the ffmpeg library is available (tested on Linux and Mac OS X).
* VRPN support: Can communicate with virtual reality tracking systems which support the VRPN protocol. Includes a utilities to record and playback motions.
* Example programs: Includes numerous sample programs which demonstrate texturing, panorama images, 3D model loading and use of tracking systems.
* Oculus support: Current windows drivers and older Oculus drivers for Linux are supported.
* Red/Cyan anaglyph stereoscopy.
* Multi-host distributed rendering: If multiple computers power multiple screens, this library provides rudimentary support to specify which part of the screen each computer is responsible for rendering and communicating information to keep the scene synchronized.


See the "help" folder for basic information about compiling and running this software.
