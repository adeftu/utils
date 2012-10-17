#!/usr/bin/env bash

# @Description:
# Reads input from stdin in the following format:
# 
# x1_1
# x2_1
# 
# x1_2
# x2_2
# 
# ...
#
# and plots segments between each pair (x1_i,0),(x2_i,0) using Gnuplot.
# 
# @Requires:
# * Gnuplot
# 
# @Parameters:
# * [-X <maximum Ox range in 512-byte blocks>] : Set the maximum value for X (used to scale nicely).

GNUPLOT=`which gnuplot`
[ -z $GNUPLOT ] && echo "gnuplot could not be found" && exit 1

RANGE_MAX_X="*"

if [ $# -gt 0 ]; then
    if [ "$1" == "-X" -a -n "$2" ]; then
	RANGE_MAX_X="$2".0
    else
	echo "Usage:" $0 "[-X <maximum Ox range in 512-byte blocks>]" && exit
    fi
fi

# Main
(cat <<EOF
reset
set style data lines
unset key
set yrange [-1:1]
set xrange [0:$RANGE_MAX_X]
plot '-' using 1:(0) lw 7
EOF
tee) | $GNUPLOT -p
