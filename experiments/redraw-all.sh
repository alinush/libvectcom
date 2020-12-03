#!/bin/sh
set -e

scriptdir=$(cd $(dirname $0); pwd -P)

#echo "Script dir: $scriptdir"
(cd $scriptdir/vcs/; ./redraw.sh )
