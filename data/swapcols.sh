#%/bin/bash
#
# Tiny one time usage helper script to
# swap columns in numerical text data file.

                                    # Wall clock time in 1st column
./swapcols  2  4  $1   tmp.dat      # Sun "%" to 2nd column
./swapcols  3  5  tmp.dat  $1.dat   # Sun cumulative to 3rd col
rm tmp.dat
