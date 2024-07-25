#!/bin/bash

# This setup presumes that the muse environment has been created prior
# to sourcing this script.

# Modify binary search path
export BASE_RELEASE=${PWD}
export PATH=${PWD}:$PATH

# make directories if they doesn't exist
mkdir -p Run/bin/
mkdir -p Run/obj/
mkdir -p Utilities/obj/
mkdir -p output

export PATH=Run/bin/:$PATH
