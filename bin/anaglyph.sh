#!/usr/bin/env bash

source common.sh
ensureWorkingDir
parameterCheck "$0" $#
# Add local directory to our PATH so user can use "exectuable" instead of "./executable"
PATH=.:$PATH
findExecutable "$1"

export VIEWMAT_DISPLAY_MODE="anaglyph"
"${@}"

