#!/bin/bash
set -e

scriptdir=$(cd $(dirname $0); pwd -P)
sourcedir=$(cd $scriptdir/../..; pwd -P)
. $scriptdir/shlibs/check-env.sh
. $scriptdir/shlibs/os.sh

$scriptdir/submodule-update.sh

builddir=$VECTCOM_BUILD_DIR
mkdir -p "$builddir"
cd "$builddir"

#if [ "$OS" = "Linux" ]; then
#    export CXX=clang++
#fi
cmake $VECTCOM_CMAKE_ARGS $@ "$sourcedir"
