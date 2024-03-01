#!/bin/bash

# Output file name
output_file="./data/address_file.txt"

# Number of addresses to generate
num_addresses=1000

# Generate 32-bit addresses and save them to the output file
for ((i = 0; i < num_addresses; i++)); do
    address=$(printf "%x\n" $((RANDOM << 18 | RANDOM)))
    echo $address >> $output_file
done

echo "Generated $num_addresses 32-bit addresses. See $output_file."
