#!/usr/bin/env bash

function cleandir()
{
	if [[ -e Makefile ]]; then
		make -C "${1}" clean
	fi
	rm -vrf "${1}/CMakeFiles"
	rm -vf  "${1}/CMakeCache.txt"
	rm -vf  "${1}/Makefile"
	rm -vf  "${1}/cmake_install.cmake"
	rm -vf *~
	rm -vf \#*\#
}

cleandir .
cleandir samples
cleandir lib
cleandir dgr
cleandir vrpn-fake

rm -vrf doxygen-docs
rm -vrf bin/*.frag bin/*.vert
rm -vf *.exe


if [[ -x .git ]]; then
	echo
	echo
	echo "Consider removing the following files because they are not in the git repo and are not ignored:"
	echo
	git ls-files --others --exclude-standard
fi
