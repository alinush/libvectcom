#!/bin/sh
set -e

scriptdir=$(cd $(dirname $0); pwd -P)

plot_cmd=$scriptdir/../../scripts/linux/plot-vcs.py

rm -f $scriptdir/fk-vs-pointproofs.png 
$plot_cmd $scriptdir/fk-vs-pointproofs.png 0 0 $scriptdir/*.csv
