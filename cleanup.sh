#!/usr/bin/env bash

function cleandir()
{
	# If makefile still exists, try "make clean"
	if [[ -e Makefile ]]; then
		make -C "${1}" clean
	fi
	rm -vrf "${1}/CMakeFiles"
	rm -vf  "${1}/CMakeCache.txt"
	rm -vf  "${1}/Makefile"
	rm -vf  "${1}/cmake_install.cmake"

	# Text editor backup files:
	rm -vf *~ \#*\#
}

# Clean the current directory
cleandir .
for D in *; do # For each file and directory
    if [ -d "${D}" ]; then # If it is a directory
		if [[ -e "${D}/CMakeLists.txt" ]]; then # if CMakeLists file is present
			cleandir "${D}"
		fi
    fi
done

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
