#!/bin/bash

# This script uses GNU library utility like enhanced `getopt`. 
# Only tested on `Ubuntu 15.04 and 14.04` with `bash`. 
# If you are running on differen platform, you may need to 
# modify this script. Otherwise, you will face errors. 


usage() {
    cnt=$((cnt+1))
    main
}

dev="" # device to inspect 

set_opts() {
    echo a
}

main() {
    set_opts $@
}

main $*

# get basename: $(basename "$0")
