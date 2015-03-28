#!/usr/bin/env bash

if [[ -e Makefile ]]; then
	make clean
fi
rm -vf cmake_install.cmake
rm -vrf CMakeFiles
rm -vrf doxygen-docs
rm -vf CMakeCache.txt
rm -vf Makefile
rm -vf libkuhl.a
rm -vf *~
rm -vf \#*\#
rm -vf *.exe
