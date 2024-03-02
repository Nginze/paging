#!/bin/bash

# Compile the code
make

# Generate the random addresses
./seed.sh

# Run the program
./vm_sim ./data/address_file.txt