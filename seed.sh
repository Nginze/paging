#!/bin/bash

# Define the number of addresses to generate
NUM_ADDRESSES=1000

# Ensure the data directory exists
mkdir -p ./data

# Generate random 32-bit integer addresses and write them to the file
for ((i=0; i<NUM_ADDRESSES; i++))
do
    # Generate a random 32-bit integer address
    ADDRESS=$((RANDOM<<16|RANDOM))

    # Write the address to the file
    echo $ADDRESS >> ./data/address_file.txt
done

echo "Addresses generated and saved to ./data/address_file.txt"
