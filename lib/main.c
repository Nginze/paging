#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "vmTypes.h"
#include "dTypes.h"

/*
    File:           main.c
    Date:           1 March 2024
    Course:         Operating Systems
    Assignment:     Midsemester Project
*/

#define VIRTUAL_ADDRESS_SPACE (1 << 16)                         // 2^16 KB
#define PHYSICAL_ADDRESS_SPACE (1 << 16)                        // 2^16 KB
#define PAGE_SIZE (1 << 8)                                      // 256B Size of each page
#define FRAME_SIZE (1 << 8)                                     // 256B Size of each frame
#define PAGE_TABLE_SIZE (VIRTUAL_ADDRESS_SPACE / PAGE_SIZE)     // size of the page table
#define TOTAL_FRAME_COUNT (PHYSICAL_ADDRESS_SPACE / FRAME_SIZE) // total number of frames in physical memory
#define PAGE_DIRECTORY_MASK 0xF000                              // mask to reveal page number
#define PAGE_TABLE_MASK 0x0F00                                  // mask to reveal page number
#define OFFSET_MASK 0xFF                                        // mask to reveal page offset
#define PAGE_DIRECTORY_SHIFT 12                                 // bitmask shift amount for page number
#define SHIFT 8                                                 // bitmask shift amount for page number
#define TLB_SIZE 16                                             // size of the TLB (cache)
#define MAX_ADDR_LEN 10                                         // buffer len for random addresses (32 bit)
#define PAGE_READ_SIZE 256                                      // buffer for reading page binaries from disk file
#define NUM_OF_SIMULATED_PROCESSES 2                            // number of processes to simulate

// Simulation vars and structures
vmTable_t *tlbTable;      // translation lookup buffer for caching
vmTable_t *pageDirectory; // page directory (1st level page table)
vmTable_t *pageTable;     // page directory (1st level page table)
int **dram;               // dynamic random access memory (physical memory)

// read buffers for address and disk access
char addressReadBuffer[MAX_ADDR_LEN];
signed char fileReadBuffer[PAGE_READ_SIZE];

// parameters for a specific virtual address instance
int virtual_addr;
int page_directory_index;
int page_number;
int offset_number;
signed char translatedValue;

// pointer to track next available slot in TLB, Page Tables and DRAM
int nextTLBentry = 0;
int nextPage = 0;
int nextFrame = 0;

// pointers to file descriptors for reading addresses and disk file
FILE *address_file;
FILE *backing_store;

// CLI & Menu vars and structures
char algo_choice;               // LRU or FIFO policy
char access_pattern;            // Random Access or Sequential or Repeated or Locality of Reference
char display_choice;            // option to log translation at every instance
int num_of_simulated_processes; // number of processes to simulate
int translationCount;           // translation count for statistics
int totalMemoryUsed;            // total memory used for statistics

clock_t start, end;        // start and end time
double cpu_time_used;      // execution time
int functionCallCount = 0; // number of function calls made

// hoist function prototypes
void translateAddress();
void readFromStore(int pageNumber);
void tlbFIFOinsert(int pageNumber, int frameNumber);
void tlbLRUinsert(int pageNumber, int frameNumber);
int getOldestEntry(int tlbSize);
double getAvgTimeInBackingStore();
void writeStructToFile(const char *filename, const vmTable_t *table);
void readStructFromFile(const char *filename, vmTable_t *table);
void simulateProcessQueue();
void assignAddresses(Process *process);

int main(int argc, char *argv[])
{
    // initialize cache, page tables and physical memory
    tlbTable = createVmTable(TLB_SIZE);
    pageDirectory = createVmTable(PAGE_TABLE_SIZE);
    pageTable = createVmTable(PAGE_TABLE_SIZE);
    dram = createDRAM(TOTAL_FRAME_COUNT, FRAME_SIZE);
    // Process_t **processQueue = (Process_t **)malloc(NUM_OF_SIMULATED_PROCESSES * sizeof(Process_t *));

    // prep useful statistics
    char *algoName;
    translationCount = 0;
    num_of_simulated_processes = NUM_OF_SIMULATED_PROCESSES;

    // ensure input address file with random addresses is passed
    if (argc != 2)
    {
        fprintf(stderr, "\033[1;31mUsage: ./vm_sim [input file]\n\033[0m");
        return -1;
    }

    // ensure disk file is open and ready for reading (binary)
    backing_store = fopen("./data/disk_file.bin", "rb");

    if (backing_store == NULL)
    {
        fprintf(stderr, "\033[1;31mError opening BACKING_STORE.bin %s\n\033[0m", "BACKING_STORE.bin");
        return -1;
    }

    // ensure address file is open and ready for reading
    address_file = fopen(argv[1], "r");

    if (address_file == NULL)
    {
        fprintf(stderr, "\033[1;31mError opening %s. Expecting [InputFile].txt or equivalent.\n\033[0m", argv[1]);
        return -1;
    }

    printf("\033[1;33m\nWelcome to The OS Paginator\033[0m");
    printf("\033[1;32m\nNumber of logical pages: %d\nPage size: %d Bytes\nPage Table Size: %d\nTLB Size: %d entries\nNumber of Physical Frames: %d\nPhysical Memory Size: %d Bytes\033[0m", PAGE_TABLE_SIZE, PAGE_READ_SIZE, PAGE_TABLE_SIZE, TLB_SIZE, FRAME_SIZE, PAGE_READ_SIZE * FRAME_SIZE);

    do
    {
        printf("\033[1;34m\nDo you want to log address translations ? [y/n]: \033[0m");
        scanf("\n%c", &display_choice);
    } while (display_choice != 'n' && display_choice != 'y');

    do
    {
        printf("\033[1;34mSelect TLB and Page Replacement Algo [1: FIFO, 2: LRU]: \033[0m");
        scanf("\n%c", &algo_choice);
    } while (algo_choice != '1' && algo_choice != '2');

    do
    {
        printf("\033[1;34mSelect a Process Translation Access  Pattern [1: Sequential, 2: Random, 3: Locality of Reference, 4: Repeated]: \033[0m");
        scanf("\n%c", &access_pattern);
    } while (access_pattern != '1' && access_pattern != '2' && access_pattern != '3' && access_pattern != '4');

    do
    {
        printf("\033[1;34mEnter the number of processes to simulate (workload) [Min: 1, Max: 10]: \033[0m");
        scanf("\n%d", &num_of_simulated_processes);
    } while (num_of_simulated_processes < 1 || num_of_simulated_processes > 10);

    simulateProcessQueue();

    // Determining stdout algo name for Menu
    if (algo_choice == '1')
    {
        algoName = "FIFO";
    }
    else
    {
        algoName = "LRU";
    }

    printf("\033[1;36m\n-----------------------------------------------------------------------------------\n\033[0m");
    printf("\033[1;33m\nResults Using %s Replacement Algorithm: \n\033[0m", algoName);
    printf("\033[1;32mNumber of translated addresses = %d\n\033[0m", translationCount);
    double pfRate = (double)pageTable->pageFaultCount / (double)translationCount;
    double TLBRate = (double)tlbTable->tlbHitCount / (double)translationCount;
    double avgMemoryUsage = (double)totalMemoryUsed / (double)translationCount;

    printf("\033[1;31mPage Faults = %d\n\033[0m", pageTable->pageFaultCount);
    printf("\033[1;31mPage Fault Rate = %.3f %%\n\033[0m", pfRate * 100);
    printf("\033[1;32mTLB Hits = %d\n\033[0m", tlbTable->tlbHitCount);
    printf("\033[1;32mTLB Hit Rate = %.3f %%\n\033[0m", TLBRate * 100);
    printf("\033[1;34mAverage time spent retrieving data from disk: %.3f millisec\n\033[0m", getAvgTimeInBackingStore());
    printf("\033[1;34mAverage memory usage: %.3f Bytes\n\033[0m", avgMemoryUsage);
    printf("\033[1;36m\n-----------------------------------------------------------------------------------\n\033[0m");

    // close the address file and disk file
    fclose(address_file);
    fclose(backing_store);

    // deallocate all tables used
    freeVmTable(&tlbTable);
    freeVmTable(&pageTable);
    freeDRAM(&dram, TOTAL_FRAME_COUNT);

    return 0;
}

void translateAddress()
{
    int frame_number = -1; // indicates frame hasn't been found

    // check if page is in TLB
    for (int i = 0; i < tlbTable->length; i++)
    {
        if (tlbTable->pageNumArr[i] == page_number)
        {
            frame_number = tlbTable->frameNumArr[i];
            tlbTable->tlbHitCount++;
            break;
        }
    }

    // tlb miss has occured so we need to check the page table
    if (frame_number == -1)
    {
        tlbTable->tlbMissCount++;

        // go through page table
        for (int i = 0; i < nextPage; i++)
        {
            // If the page is found in those contents
            if (pageTable->pageNumArr[i] == page_number)
            {
                frame_number = pageTable->frameNumArr[i];
                break;
            }
        }

        // page fault has occured so we need to read page contents from disk (binary file in data directory)
        if (frame_number == -1)
        {
            // read page from disk and load into physical memory (clock how long operation took)
            pageTable->pageFaultCount++;
            start = clock();
            readFromStore(page_number);
            cpu_time_used += (double)(clock() - start) / CLOCKS_PER_SEC;
            functionCallCount++;
            frame_number = nextFrame - 1;
            totalMemoryUsed += FRAME_SIZE; // Increment the total memory used
        }
    }

    // update tlb and pagetables using selected eviction policy at menu start
    if (algo_choice == '1')
    {
        tlbFIFOinsert(page_number, frame_number);
    }
    else
    {
        tlbLRUinsert(page_number, frame_number);
    }

    // finally grab data from dram using frame number and offset
    translatedValue = dram[frame_number][offset_number];

    // program option to log translations to console
    if (display_choice == 'y')
    {
        printf("\n\033[1;31mVirtual(16bit):\033[0m x0%d\t\t\033[1;32mPhysical(16bit):\033[0m x0%d\t\t\033[1;34mDRAM:\033[0m %d", virtual_addr, (frame_number << SHIFT) | offset_number, translatedValue);
    }
}

void readFromStore(int pageNumber)
{
    // seek to the correct page in the disk file
    if (fseek(backing_store, pageNumber * PAGE_READ_SIZE, SEEK_SET) != 0)
    {
        fprintf(stderr, "Error seeking in backing store\n");
    }

    // read the page into the buffer from the disk
    if (fread(fileReadBuffer, sizeof(signed char), PAGE_READ_SIZE, backing_store) == 0)
    {
        fprintf(stderr, "Error reading from backing store\n");
    }

    // load the page into the next available frame in physical memory
    for (int i = 0; i < PAGE_READ_SIZE; i++)
    {
        dram[nextFrame][i] = fileReadBuffer[i];
    }

    // update the page table with the new page number and frame number
    pageTable->pageNumArr[nextPage] = pageNumber;
    pageTable->frameNumArr[nextPage] = nextFrame;

    // increment the counters that track the next available frames
    nextFrame++;
    nextPage++;
}

void tlbFIFOinsert(int pageNumber, int frameNumber)
{
    int i;

    // check if page is already cached
    for (i = 0; i < nextTLBentry; i++)
    {
        if (tlbTable->pageNumArr[i] == pageNumber)
        {
            break;
        }
    }

    if (i == nextTLBentry) // grab next available spot in tlb
    {
        if (nextTLBentry < TLB_SIZE) // if tlb has more space
        {
            tlbTable->pageNumArr[nextTLBentry] = pageNumber;
            tlbTable->frameNumArr[nextTLBentry] = frameNumber;
        }
        else
        {
            // use FIFO eviction policy to replace the oldest entry
            tlbTable->pageNumArr[nextTLBentry - 1] = pageNumber;
            tlbTable->frameNumArr[nextTLBentry - 1] = frameNumber;

            // shift all entries to the left to create space for new entry
            for (i = 0; i < TLB_SIZE - 1; i++)
            {
                tlbTable->pageNumArr[i] = tlbTable->pageNumArr[i + 1];
                tlbTable->frameNumArr[i] = tlbTable->frameNumArr[i + 1];
            }
        }
    }

    // increment the number of entries in the tlb
    else
    {

        for (i = i; i < nextTLBentry - 1; i++)
        {
            // shift all entries to the left to create space for new entry
            tlbTable->pageNumArr[i] = tlbTable->pageNumArr[i + 1];
            tlbTable->frameNumArr[i] = tlbTable->frameNumArr[i + 1];
        }
        if (nextTLBentry < TLB_SIZE)
        {
            // if tlb has more space
            tlbTable->pageNumArr[nextTLBentry] = pageNumber;
            tlbTable->frameNumArr[nextTLBentry] = frameNumber;
        }
        else
        {
            // use FIFO eviction policy to replace the oldest entry
            tlbTable->pageNumArr[nextTLBentry - 1] = pageNumber;
            tlbTable->frameNumArr[nextTLBentry - 1] = frameNumber;
        }
    }
    if (nextTLBentry < TLB_SIZE)
    {
        // If there is still room in the arrays, increment the number of entries
        nextTLBentry++;
    }
}

void tlbLRUinsert(int pageNumber, int frameNumber)
{

    // initialize flags and index
    bool freeSpotFound = false;
    bool alreadyThere = false;
    int replaceIndex = -1;

    for (int i = 0; i < TLB_SIZE; i++)
    {
        if ((tlbTable->pageNumArr[i] != pageNumber) && (tlbTable->pageNumArr[i] != 0))
        {
            tlbTable->entryAgeArr[i]++;
        }
        else if ((tlbTable->pageNumArr[i] != pageNumber) && (tlbTable->pageNumArr[i] == 0))
        {
            // free spot found
            if (!freeSpotFound)
            {
                replaceIndex = i;
                freeSpotFound = true;
            }
        }
        else if (tlbTable->pageNumArr[i] == pageNumber)
        {
            // page already in TLB
            if (!alreadyThere)
            {
                tlbTable->entryAgeArr[i] = 0;
                alreadyThere = true;
            }
        }
    }

    // if the page is already in the TLB, update the age and return
    if (alreadyThere)
    {
        return;
    }
    else if (freeSpotFound)
    {
        tlbTable->pageNumArr[replaceIndex] = pageNumber;
        tlbTable->frameNumArr[replaceIndex] = frameNumber;
        tlbTable->entryAgeArr[replaceIndex] = 0;
    }
    else
    {
        replaceIndex = getOldestEntry(TLB_SIZE);
        tlbTable->pageNumArr[replaceIndex] = pageNumber; // insert into oldest entry
        tlbTable->frameNumArr[replaceIndex] = frameNumber;
        tlbTable->entryAgeArr[replaceIndex] = 0;
    }
}

int getOldestEntry(int tlbSize)
{
    int i, max, index;

    max = tlbTable->entryAgeArr[0];
    index = 0;

    for (i = 1; i < tlbSize; i++)
    {
        if (tlbTable->entryAgeArr[i] > max)
        {
            index = i;
            max = tlbTable->entryAgeArr[i];
        }
    }
    return index;
}

double getAvgTimeInBackingStore()
{
    double temp = (double)cpu_time_used / (double)functionCallCount;
    return temp * 1000000;
}

void writeStructToFile(const char *filename, const vmTable_t *table)
{
    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        printf("Error opening file %s\n", filename);
        return;
    }

    fwrite(table, sizeof(vmTable_t), 1, file);

    fclose(file);
}

void readStructFromFile(const char *filename, vmTable_t *table)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("Error opening file %s\n", filename);
        return;
    }

    fread(table, sizeof(vmTable_t), 1, file);

    fclose(file);
}

void assignAddresses(Process *process)
{
    FILE *address_file = fopen("./data/address_file.txt", "r"); // Open the address file for reading
    if (address_file == NULL)
    {
        fprintf(stderr, "Error opening address file\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < process->num_addresses; i++)
    {
        char addressReadBuffer[MAX_ADDR_LEN]; // Buffer to store the read address
        if (fgets(addressReadBuffer, MAX_ADDR_LEN, address_file) == NULL)
        {
            fprintf(stderr, "Error reading address from file\n");
            exit(EXIT_FAILURE);
        }
        process->addresses[i] = atoi(addressReadBuffer); // Convert the read string to an integer
    }

    fclose(address_file); // Close the file after reading
}

// Function to translate an address for a process
void translateAddressForProcess(Process *process, int addressIndex)
{
    virtual_addr = process->addresses[addressIndex];

    // 32-bit masking function to extract page number
    page_number = getPageNumber(PAGE_TABLE_MASK, virtual_addr, SHIFT);

    // 32-bit masking function to extract page offset
    offset_number = getOffset(OFFSET_MASK, virtual_addr);

    translateAddress();
    translationCount++;
}

// Function to simulate a process queue
void simulateProcessQueue()
{
    // Create and assign addresses to each process
    Process *processQueue[num_of_simulated_processes];
    for (int i = 0; i < num_of_simulated_processes; i++)
    {
        processQueue[i] = createProcess(i, PAGE_READ_SIZE); // Assuming each process has X amount of addresses
        assignAddresses(processQueue[i]);
    }

    for (int i = 0; i < num_of_simulated_processes; i++)
    {
        printf("\n\033[1;35m==================== Simulating Translation For Process %d ====================\033[0m\n", processQueue[i]->process_id);

        switch (access_pattern)
        {
        case '1': // Sequential Access
            for (int j = 0; j < processQueue[i]->num_addresses; j++)
            {
                translateAddressForProcess(processQueue[i], j);
            }
            break;
        case '2': // Random Access
            for (int j = 0; j < processQueue[i]->num_addresses; j++)
            {
                int randomIndex = rand() % processQueue[i]->num_addresses;
                translateAddressForProcess(processQueue[i], randomIndex);
            }
            break;
        case '3': // Locality of Reference
            int rangeStart = rand() % processQueue[i]->num_addresses;
            int rangeSize = 10; // or some other number
            for (int j = 0; j < rangeSize; j++)
            {
                translateAddressForProcess(processQueue[i], (rangeStart + j) % processQueue[i]->num_addresses);
            }
            break;
        case '4': // Repeated Access
            int hotAddressIndex = rand() % processQueue[i]->num_addresses;
            for (int j = 0; j < 10; j++) // or some other number
            {
                translateAddressForProcess(processQueue[i], hotAddressIndex);
            }
            break;
        default:
            fprintf(stderr, "Invalid access pattern\n");
            exit(EXIT_FAILURE);
        }
    }

    // Free memory allocated for processes
    for (int i = 0; i < num_of_simulated_processes; i++)
    {
        freeProcess(processQueue[i]);
    }
}
