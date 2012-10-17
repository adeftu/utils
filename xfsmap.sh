#!/usr/bin/env bash

# @Description:
# For a given file (or all files in a folder) located on an XFS partition,
# writes to stdout the pairs of block offsets (in units of 512-byte blocks),
# in the following format:
# 
# x1_1
# x2_1
# 
# x1_2
# x2_2
# 
# ...
#
# @Requires:
# * xfs_bmap
# 
# @Parameters:
# * <directory | file> : To print block mapping for.

[ $# -ne 1 ] && echo "Usage:" $0 "<directory | file>" && exit

XFS_BMAP=`which xfs_bmap`
[ -z $XFS_BMAP ] && echo "xfs_bmap could not be found" && exit 1

# Receive a file name and outputs the offsets (in units of 512-byte blocks) 
# of the extents occupied on disk
print_offsets() {
    $XFS_BMAP "$1" | tail -n +2 | while IFS='\n' read line; do
	regex='[^[:space:]]+[[:space:]]+[^[:space:]]+[[:space:]]+([[:digit:]]+)\.\.([[:digit:]]+)'
	if [[ "$line" =~ $regex ]]; then
	    echo "${BASH_REMATCH[1]}"
	    echo "${BASH_REMATCH[2]}"
	    echo
	fi
    done
}

# Walk a directory recursively and call print_offsets() for each file
walk_dir() {
    ls -a "$1" | while IFS= read i; do
	[ "$i" == ".." -o "$i" == "." ] && continue
	entry="$1/$i"
	if [ -d "$entry" ]; then
	    walk_dir "$entry"
	elif [ -f "$entry" ]; then
	    print_offsets "$entry"
	fi
    done
}

# Main
if [ -d "$1" ]; then
    walk_dir "$1"
elif [ -f "$1" ]; then
    print_offsets "$1"
else
    echo "Directory or file does not exist" && exit 1
fi
