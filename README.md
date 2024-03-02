# Operating Systems Midsemester Project

## Overview

This project is a simulation of a virtual memory system implemented in C. It simulates the translation of virtual addresses to physical addresses using a two-level page table, TLB (Translation Lookaside Buffer), and physical memory (DRAM). The simulation tracks various statistics such as page faults, TLB hits, and average memory usage.

## Prerequisites

- C compiler (e.g., GCC)
- Make tool

## Running the Program

1. Clone the repository:
   ```bash
   git clone <repository_url>
   ```
2. Navigate to the project directory:
   ```bash
   cd <os_project>
   ```
3. Compile the code:
   ```bash
   make
   ```
4. Generate random addresses:
   ```bash
   ./seed.sh
   ```
5. Run the program with the address file:
   ```bash
   ./vm_sim ./data/address_file.txt
   ```
6. Clean page table binaries in data dir after running a simulation (for predictable behaviour)
   ```bash
   make clean
   ```
   > Alternatively you could just do `./run.sh` but make sure to give execute permissions to file with `chmod 777 run.sh`

## Menu Options

Upon running the program, you'll be prompted to select various options:

- **Logging address translations**: Choose whether to log each address translation.
- **TLB and Page Replacement Algorithm**: Choose between FIFO and LRU policies.
- **Process Translation Access Pattern**: Choose the access pattern for address translation (Sequential, Random, Locality of Reference, Repeated).
- **Number of Processes to Simulate**: Specify the number of processes to simulate.

## Project Structure

- **main.c**: Entry point of the program containing the main logic.
- **vmTypes.h**: Header file containing custom data types and structures.
- **dTypes.h**: Header file containing constants and macros used in the program.

## Implementation Details

- **Page Tables**: Implemented as two-level page tables consisting of a page directory and page tables.
- **TLB**: Simulated as a cache for frequently accessed translations.
- **Physical Memory**: Represented as DRAM (Dynamic Random Access Memory) storing page frames.
- **Address Translation**: Virtual addresses are translated to physical addresses using page tables and TLB.
- **Page Fault Handling**: Page faults are handled by loading pages from disk into physical memory.

## Performance Metrics

- **Page Fault Rate**: Percentage of address translations resulting in page faults.
- **TLB Hit Rate**: Percentage of TLB accesses resulting in cache hits.
- **Average Time Spent Retrieving Data from Disk**: Average time (in milliseconds) spent reading data from disk.
- **Average Memory Usage**: Average amount of memory (in bytes) used during address translation.
