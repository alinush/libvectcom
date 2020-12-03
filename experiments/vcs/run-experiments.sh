#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Runs the VC proof computation benchmark for the specified scheme with increasing n = 2, 4, 8, 16, \dots, "
    echo
    echo "Usage: $0 <machine-type> <vc-scheme> [<naive>]"
    echo
    echo "OPTIONS:"
    echo "   <scheme>   can be either 'fk' or 'pointproofs'"
    echo "   <naive>    when <scheme> is 'pointproofs', this can be either 'naive' or 'efficient' (defaults to 'efficient')"
    exit 1
fi

scriptdir=$(cd $(dirname $0); pwd -P)
date=`date +%Y-%m-%d` # %H-%M-%S if you want hours, minutes & seconds
machine=$1
scheme=$2
naive=${3:-"efficient"}

if [ $scheme = "fk" ]; then
    :
elif [ $scheme = "pointproofs" ]; then
    :
else
    echo "ERROR: <scheme> must be either 'fk' or 'pointproofs'"
    exit 1
fi

if [ $naive = "naive" ]; then
    wantNaive=1
    echo "Doing naive Pointproofs"
elif [ $naive = "efficient" ]; then
    wantNaive=0
    echo "Doing efficient Pointproofs"
else
    echo "ERROR: <naive> must be either 'naive' or 'efficient'"
    exit 1
fi


which BenchFk 2>&1 >/dev/null || { echo "ERROR: You did not set up the environment"; exit 1; }
which BenchPointproofs 2>&1 >/dev/null || { echo "ERROR: You did not set up the environment"; exit 1; }

# Format: <scheme> <n> <samples> <naive>
benchmarks="\
fk 2    10 - 
fk 4    10 - 
fk 8    10 - 
fk 16   10 - 
fk 32   10 - 
fk 64   10 - 
fk 128  10 - 
fk 256  10 - 
fk 512  10 - 
fk 1024 10 - 
pointproofs 2    10 1
pointproofs 2    10 0
pointproofs 4    10 1
pointproofs 4    10 0
pointproofs 8    10 1
pointproofs 8    10 0
pointproofs 16   10 1
pointproofs 16   10 0
pointproofs 32   10 1
pointproofs 32   10 0
pointproofs 64   10 1
pointproofs 64   10 0
pointproofs 128  5  1
pointproofs 128  10 0
pointproofs 256  2  1
pointproofs 256  10 0
pointproofs 512  1  1
pointproofs 512  10 0
pointproofs 1024 1  1
pointproofs 1024 10 0
"

test_benchmarks="\
fk 2    10 - 
fk 4    10 - 
pointproofs 2    10 1
pointproofs 2    10 0
pointproofs 4    10 1
pointproofs 4    10 0
"

# sort by n
filtered=`echo "$benchmarks" | grep ^$scheme`
min_and_max=`echo "$filtered" | sort -t' ' -nk2 | awk 'NR==1; END{print}'`
min_n=`echo "$min_and_max" | cut -d' ' -f 2 | head -n 1`
max_n=`echo "$min_and_max" | cut -d' ' -f 2 | tail -n 1`
echo "Min n: $min_n"
echo "Max n: $max_n"

pp_file=$scriptdir/../../public-params/65536/65536

while read -r b; do
    sch=`echo $b | cut -d' ' -f 1`
    n=`echo $b | cut -d' ' -f 2`
    r=`echo $b | cut -d' ' -f 3`
    isNaive=`echo $b | cut -d' ' -f 4`
    

    if [ "$sch" != "$scheme" ]; then
        continue
    fi

    if [ "$isNaive" != "-" -a "$isNaive" != "$wantNaive" ]; then
        continue
    fi

       file=${date}-${sch}-vc-${min_n}-${max_n}-${machine}.csv
    logfile=${date}-${sch}-vc-${min_n}-${max_n}-${machine}.log
    if [ $sch == "pointproofs" ]; then
        echo "Running benchmark for $sch ($naive) with n = $n ($r iters)"
        echo " \--> Logging to $logfile ..."
        BenchPointproofs $pp_file $n $r $file $wantNaive 2>&1 | tee -a $logfile
    else
        echo "Running benchmark for $sch with n = $n ($r iters)"
        echo " \--> Logging to $logfile ..."
        BenchFk $pp_file $(($n-1)) $n $r $file 2>&1 | tee -a $logfile
    fi
done <<< "$benchmarks"
