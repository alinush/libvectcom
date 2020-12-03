#!/bin/bash
set -e

scriptdir=$(cd $(dirname $0); pwd -P)
sourcedir=$(cd $scriptdir/../..; pwd -P)

. $scriptdir/shlibs/os.sh

if [ $# -lt 2 ]; then
    echo "Usage: $0 <old-name-without-lib> <new-name-without-lib>"
    echo 
    echo "Example: $0 polycrypto vectcom"
    exit 1
fi

old_name=$1
new_name=$2

sed_cmd=sed
if [ "$OS" = "OSX" ]; then
    sed_cmd=gsed
fi

find $sourcedir \( -type d -name .git -prune \) -o -type f -print0 | xargs -0 $sed_cmd -i "s/lib$old_name/lib$new_name/g"
find $sourcedir \( -type d -name .git -prune \) -o -type f -print0 | xargs -0 $sed_cmd -i "s/$old_name/$new_name/g"

OLD_NAME=`echo $old_name | tr [a-z] [A-Z]`
NEW_NAME=`echo $new_name | tr [a-z] [A-Z]`

find $sourcedir \( -type d -name .git -prune \) -o -type f -print0 | xargs -0 $sed_cmd -i "s/$OLD_NAME/$NEW_NAME/g"
