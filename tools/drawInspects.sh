#!/bin/bash

# usage of this script
# print usage of this script
usage() {
    echo "
USAGE: $0 [option lists]

Options: 
    --set-xcol | --set-ycol [num]
        Column number of the input file to be used as X or Y axis

    -w | --set-width [num]
        Width of the output image for JPEG type

    -l | --set-height {num]
        Height of the output image for JPEG type

    --set-x-start | --set-y-start [num]
        Start point for xrange or yrange

    --set-x-end | --set-y-end [num]
        End point for xrang eor yrange

    --set-output-dir [path]
        Set output image's directory 

    -o | --set-output-file [path]
        Set output image's name or full path 

    -t | --set-image-type [jpeg | eps | ps]
        Set output image's type 

    --set-xlabel | --set-ylabel [name]
        Set name of the x or y label

    --set-separator [separator]
        Set separator to be used as parsing the input file 

    --set-point-size [num]
        Set point size of the image 

    -f | --file [path]
        Set file path for input file 

    -h | --help 
        Show this usage messages
        "
}

# comments for variables
: '
FDB_INSPECT_HOME=           # Home directory where the fdb inspector is installed 
FDB_INSPECT_RESULT=         # Result files of fdb inspector 
PLOT_IMAGE_DIR=             # OUTPUT directory for plotted image
PLOT_X_START=               # begin of xrange
PLOT_X_END=                 # end of xrange
PLOT_Y_START=               # begin of yrange
PLOT_Y_END=                 # end of yrange
PLOT_SIZE_WIDTH=            # width of output image
PLOT_SIZE_HEIGHT=           # height of output image
PLOT_IMAGE_TYPE=            # image type of output image
PLOT_GRAPH_TYPE=            # graph type of output image
PLOT_XLABLE=                # xlabel of image
PLOT_YLABEL=                # ylabel of image 
' # comments for variables


RETURN_ON_SUCCESS=0
RETURN_ON_ERROR=1
RETURN_ON_INVALID_ARGS=2
RETURN_ON_COMMAND_NOT_FOUND=3

: '
Additional functions for plotting the graph using gnuplot.
From here it would be hard to read. Refactoring would be in a year. 
'

# Set variables for this script
set_default() {
    PLOT_SIZE_WIDTH="1960"; 
    PLOT_SIZE_HEIGHT="1024";
    PLOT_SIZE_POINT="0.2";
    PLOT_SEPARATOR=" ";
    PLOT_IMAGE_TYPE="jpeg";
    PLOT_XLABEL="Timestamp (second)";
    PLOT_YLABEL="Logical Sector Number";
    FDB_INSPECT_RESULT="./result.inspect";
    PLOT_Y_START="";
    PLOT_Y_END="";
    PLOT_X_END="";
    PLOT_X_START="";
    PLOT_X_COLUMN=4;
    PLOT_Y_COLUMN=8;
}

args_sanity_check() { 
    echo "Arguments sanity check"
    return ${RETURN_ON_SUCCESS}
}

set_plot_style() { 

# set image options 
    DEFAULT_SCRIPT="
    set xlabel \"${PLOT_XLABEL}\";
    set ylabel \"${PLOT_YLABEL}\";
    set xrange [${PLOT_X_START}:${PLOT_X_END}];
    set yrange [${PLOT_Y_START}:${PLOT_Y_END}];
    set pointsize ${PLOT_SIZE_POINT};
    set datafile separator \"${PLOT_SEPARATOR}\";
    set output \"${PLOT_OUTPUT_FILE}.${PLOT_IMAGE_TYPE}\"; "

    case "${PLOT_IMAGE_TYPE}" in
        "eps" )
            PLOT_TYPE="set terminal postscript eps;"
            ;;
        "ps" )
            PLOT_TYPE="set terminal postscript enhanced color;"
            ;;
        "jpg"|"jpeg") # default type is jpeg
            PLOT_TYPE="set te jpeg giant size ${PLOT_SIZE_WIDTH},${PLOT_SIZE_HEIGHT};"
            ;;
        *) 
            echo "Set your image type"
            exit 3
            ;;
    esac
    
# set style options
    PLOT_STYLE="" # initialize 
    for i in `seq 10`
    do

        PLOT_STYLE="${PLOT_STYLE} 
    set style line ${i} lc ${i}; "

    done
    PLOT_STYLE="${PLOT_TYPE}
                ${DEFAULT_SCRIPT}
                ${PLOT_STYLE}
    "
}


# create plot script for GNUPLOT
set_plot_script() { 
    PLOT_VARIABLES=("NOT_FDB" "DOC" "BNODE" "DBHEADER"  "SB" )
    PLOT_SCRIPT="plot " # initialize 

    for i in `seq ${#PLOT_VARIABLES[@]}`
    do
        ptype=${PLOT_VARIABLES[$((i-1))]};
        if [ ${i} -ne 1 ]
        then
            PLOT_SCRIPT="${PLOT_SCRIPT}, ";
        fi
        PLOT_SCRIPT="${PLOT_SCRIPT} \"< grep ${ptype} ${FDB_INSPECT_RESULT}\" u ${PLOT_X_COLUMN}:${PLOT_Y_COLUMN} ls ${i} ti \"${ptype}\""
    done
    PLOT_SCRIPT="${PLOT_SCRIPT};"
}

plot() { 
    echo "Plot basic type graph" ;
    SCRIPT_FILE=./inspect.plot

    set_plot_script;    # set gnuplot script
    set_plot_style;     # set line or point style 

    echo "${PLOT_STYLE} ${PLOT_SCRIPT}" | tee ${SCRIPT_FILE}
    gnuplot ${SCRIPT_FILE}

    return $?; # return gnuplot's return value
}


set_args() {
    echo "Set arguments" ;
    set_default;

    while getopts ":-:o:f:t:hw:l:" opt $@
    do
        echo "OPTION is ${opt}${OPTARG}"
        case ${opt} in
            -)
                IFS='=' read -ra token <<< "${OPTARG}"
                if [ ${#token[@]} -gt 1 ]
                then
                    option=${token[0]}
                    ARGS=${token[1]}
                else
                    shift $((OPTIND-1))
                    option=${OPTARG}
                    ARGS=${1}
                    OPTIND=2
                fi
                case ${option} in
                    set-xcol)
                        PLOT_X_COLUMN=${ARGS}
                        ;;
                    set-ycol)
                        PLOT_Y_COLUMN=${ARGS}
                        ;;
                    set-width)
                        PLOT_SIZE_WIDTH=${ARGS}
                        ;;
                    set-height)
                        PLOT_SIZE_HEIGHT=${ARGS}
                        ;;
                    set-x-start)
                        PLOT_X_START=${ARGS}
                        ;;
                    set-y-start)
                        PLOT_Y_START=${ARGS}
                        ;;
                    set-x-end)
                        PLOT_X_END=${ARGS}
                        ;;
                    set-y-end)
                        PLOT_Y_END=${ARGS}
                        ;;
                    set-output-dir)
                        PLOT_IMAGE_DIR=${ARGS}
                        ;;
                    set-image-type)
                        PLOT_IMAGE_TYPE=${ARGS}
                        ;;
                    set-xlabel)
                        PLOT_XLABEL=${ARGS}
                        ;;
                    set-ylabel)
                        PLOT_YLABEL=${ARGS}
                        ;;
                    set-separator)
                        PLOT_SEPARATOR=${ARGS}
                        ;;
                    set-point-size)
                        PLOT_SIZE_POINT=${ARGS}
                        ;;
                    set-output-file)
                        PLOT_OUTPUT_FILE=${ARGS}
                        ;;
                    file)
                        FDB_INSPECT_RESULT=${ARGS}
                        ;;
                    help|*)
                        echo "You used ${opt} ${OPTARG}"
                        usage
                        exit
                        ;;
                esac
                ;;
            f) 
                FDB_INSPECT_RESULT="${OPTARG}"
                ;;
            w)
                PLOT_SIZE_WIDTH=${OPTARG}
                ;;
            l)
                PLOT_SIZE_HEIGHT=${OPTARG}
                ;;
            o)
                PLOT_OUTPUT_FILE=${OPTARG}
                ;;
            t)
                PLOT_IMAGE_TYPE=${OPTARG}
                ;;
            "?")
                if [ -n ${OPTARG} ]
                then
                    echo "You used ${opt} ${OPTARG}"
                    usage
                    exit 1;
                else
                    echo "You used ${opt} ${OPTARG}"
                fi
                ;;
            h|*)
                echo "You used ${opt} ${OPTARG}"
                usage 
                exit 1;
                ;;
        esac
    done

    if [ -z "${PLOT_OUTPUT_FILE}" ]
    then
        PLOT_OUTPUT_FILE=$(basename ${FDB_INSPECT_RESULT})
    fi

    args_sanity_check;
    return $?
}

# starting point of this script
main() {
    set_args $@;
    ret_val=$?;

    if [ ${ret_val} -ne ${RETURN_ON_SUCCESS} ] || ! [ -f ${FDB_INSPECT_RESULT} ]
    then
        usage;
        return ${ret_val}
    fi

    plot;

    return $?
}

# call main() function instead of processing in a nature
main $@

echo "return val is $?"



