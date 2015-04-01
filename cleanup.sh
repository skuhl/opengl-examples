#!/usr/bin/env bash

function cleandir()
{
	echo
	echo "=== Cleaning ${1}"
	# If makefile still exists, try "make clean"
	if [[ -e Makefile ]]; then
		make --quiet -C "${1}" clean
	fi
	rm -vrf "${1}/CMakeFiles"
	rm -vf  "${1}/CMakeCache.txt"
	rm -vf  "${1}/Makefile"
	rm -vf  "${1}/cmake_install.cmake"

	# Text editor backup files:
	rm -vf *~ \#*\#
}

# Get the directory that this script is in (might not be the same as
# the current working directory).
THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# Clean the directory the script is in.
cleandir ${THIS_DIR}
for D in *; do # For each file and directory
    if [ -d "${D}" ]; then # If it is a directory
		if [[ -e "${D}/CMakeLists.txt" ]]; then # if CMakeLists file is present
			cleandir "${THIS_DIR}/${D}"
		fi
    fi
done

rm -vrf "${THIS_DIR}/doxygen-docs"
rm -vrf "${THIS_DIR}/bin/*.frag" "${THIS_DIR}/bin/*.vert" "${THIS_DIR}/bin/*libOVR*.so*"
rm -vf "${THIS_DIR}/*.exe"


if [[ -x "${THIS_DIR}/.git" && -x /usr/bin/git ]]; then
	echo
	echo
	echo "Consider removing the following files because they are not in the git repo and are not ignored:"
	echo
	git -C "${THIS_DIR}" ls-files --others --exclude-standard
fi
