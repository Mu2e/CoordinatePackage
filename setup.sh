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

if [[ "$MU2E_SPACK" == "true" ]]; then
    # This is not guaranteed to be robust but it works for now
    # All env variables point into the spack environment
    export BOOST_INC=${ROOT_INC}
    export ROOT_INC=${ROOT_INC}/root
    export BOOST_LIB=`echo $MUSE_LIBRARY_PATH  | awk 'BEGIN{FS=":"}{print $1}'`

    # This is needed at run time since we are not doing rpath linking
    export LD_LIBRARY_PATH=$BOOST_LIB
fi
