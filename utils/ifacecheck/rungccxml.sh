#!/bin/bash

#
# Runs gccxml on the wxWidgets include folder, in order to build the XML
# file to fetch to ifacecheck to check the coherency of the wxWidgets
# interface headers with the "real" ones.


## CONSTANTS
############


gccxmloutput="wxapi.xml"             # the file where we store the gccXML output
preprocoutput="wxapi-preproc.txt"    # the file where we store the preprocessor's #define values
allheaders="/tmp/wx-all.h"           # the header which includes all wx public headers (autogenerated)

# the list of all wxWidgets public headers
listcmd="ls wx/*.h wx/aui/*.h wx/html/*.h wx/protocol/*.h wx/richtext/*.h wx/stc/*.h wx/xml/*.h wx/xrc/*.h"



## MAIN
#######

if [[ ! -z "$1" ]]; then
    echo "This script does not accept arguments."
    echo "Usage: $0"
    echo "Creates a '$gccxmloutput' file which you can pass to ifacecheck."
    exit 1
fi

me=$(basename $0)
current=$(pwd)/${0%%/$me}            # current path

gccxmloutput="$current/$gccxmloutput"

cd ../../include              # go to wx include folder

# now filter it
headerlist=`$listcmd | grep -v wxshl | grep -v wx_cw | grep -v setup | grep -v xti | grep -v dde.h | grep -v fmappriv`

cd $current                          # return to the original path

# create the header file to pass to gccxml
echo "Creating the $allheaders dummy file which #includes all wxWidgets interface header files..."
if [[ -f $allheaders ]]; then rm $allheaders; fi
for f in $headerlist; do
    echo "#include <$f>" >> $allheaders
done

# filter the configure flags to pass to gccxml
wx_top_builddir="/home/aybike/experiment/wxWidgets-wasm"
top_srcdir="../.."
flags=" -I/home/aybike/experiment/build_deps/system/include"

# NOTE: it's important to define __WXDEBUG__ because some functions of wx
#       are declared (and thus parsed by gcc) only if that symbol is defined.
#       So we remove it from $flags (in case it's defined) and then readd it.
flags=`echo "$flags" | sed -e 's/-pthread//g' | sed -e 's/__WXDEBUG__//g'`

# append some other flags:
flags="-I . -I ../../include $flags -D__WXDEBUG__ -D__WXWASM__ -DWXBUILDING $allheaders"

# run gccxml with the same flag used for the real compilation of wx sources:
echo "Running gccxml on the $allheaders file... results in $gccxmloutput"
if [[ -f "$gccxmloutput" ]]; then rm $gccxmloutput; fi
gccxml $flags -fxml=$gccxmloutput
if [[ $? != 0 ]]; then
    echo "Errors running gccxml... aborting."
    exit
fi

# now get the list of the #defined values for wx headers, so that the result
# can be passed to ifacecheck to aid the comparison
echo "Running gccxml's preprocessor on the $allheaders file... results in $preprocoutput"
gccxml -E -dM $flags >$preprocoutput
if [[ $? != 0 ]]; then
    echo "Errors running gccxml preprocessor... aborting."
    exit
fi

# cleanup
rm $allheaders
