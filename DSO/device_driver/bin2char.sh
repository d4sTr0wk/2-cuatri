#!/bin/bash
#
# Convert decimal number writen as binary to ASCII character

# Check if the number of arguments is correct
if [ $# -ne 1 ]; then
    echo "Usage: $0 <binary number>"
    exit 1
fi

# Convert the binary number to decimal
dec=$(echo "ibase=2; $1" | bc)

oct=$(printf '%03o' $dec)

if [ -n "$oct" ]; then
        printf "\\$oct\n"
else
        char=$(printf \\$(printf '%03o' $dec))
        echo "$char"
fi

