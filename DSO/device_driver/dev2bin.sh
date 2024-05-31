#!/bin/bash
#
# Convert decimal number writen as binary to ASCII character
# Check if the number of arguments is correct
if [ $# -ne 1 ]; then
    echo "Usage: $0 <binary number>"
    exit 1
fi

# Convert the binary number to decimal
ascii_value=$(echo -n "$1" | od -An -tuC)
dec=$(echo "ibase=10;obase=2; $ascii_value" | bc)

echo $dec
