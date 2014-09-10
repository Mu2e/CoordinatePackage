#!/bin/bash

# Setup framework
setup -B art v1_10_00b -q+e5:+prof

# Setup root

# Modify binary search path 
export BASE_RELEASE=${PWD}
export PATH=${PWD}:$PATH

# make directories if they doesn't exist
mkdir -p Run/bin/ 
mkdir -p Run/obj/
mkdir -p Utilities/obj/
mkdir -p geom
mkdir -p output

export PATH=Run/bin/:$PATH
