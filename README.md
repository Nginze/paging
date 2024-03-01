### @TODO: Implement main program

* Initialize page table, TLB and physical memory (DRAM)
* Find way to simulate processes accessing memory (allocating and deallocating memory)
* Perform vpn translation to ppn
* Use various page replacement algorithms (LRU and FIFO)
* Use various TLB replacement algorithms (LRU and FIFO)
* Report statistics

* Once base works, Implement multilevel page table
* Develop CLI program to interact with the system
Process Queue ???


// #define VIRTUAL_MEMORY_SIZE (1 << 14)                            // 16KB
// #define PHYSICAL_MEMORY_SIZE (1 << 13)                           // 8KB
// #define PAGE_SIZE (1 << 2)                                       // 4B
// #define PAGE_MASK 0xFF00                                         // Masks everything but the page number
// #define OFFSET_MASK 0xFF                                         // Masks everything but the offset
// #define MAX_PAGE_TABLE_ENTRIES (VIRTUAL_MEMORY_SIZE / PAGE_SIZE) // 2^18 entries
// #define MAX_FRAME_ENTRIES (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)     // 2^16 entries
// #define MAX_TLB_ENTRIES 8                                        // 16 entries
// #define NUM_SIMULATED_PROCESSES 10                               // 10 processes

// #define ADDRESS_BUFFER_LEN 10 // 10 bytes
// #define DISK_BUFFER_LEN 256    // 256 bytes

// // page table , translation lookaside buffer and main memory repr
// vmTable_t *pageDirectory;
// vmTable_t *tlbCache;
// int **dram;

// // file descriptors to address file (with 14 bit virtual addresses) and disk file (with 13 bit physical addresses)
// FILE *address_file;
// FILE *disk_file;

// // read buffers for address and disk files @TODO: specify buffer size
// char addressReadBuffer[ADDRESS_BUFFER_LEN];
// char diskReadBuffer[DISK_BUFFER_LEN];

// int nextTLBentry = 0; // Tracks the next available index of entry into the TLB
// int nextPage = 0;     // Tracks the next available page in the page table
// int nextFrame = 0;    // Tracks the next available frame TLB or Page Table

// // Generating length of time for a function
// clock_t start, end;
// double cpu_time_used;
// int functionCallCount = 0;

// int virtual_addr;
// int page_number;
// int offset_number;

// signed char translatedValue;

// // hoisting function prototypes
// void translateAddress();
// void tlbInsertFIFO();
// void tlbInsertLRU(int pageNumber, int frameNumber);
// void readFromDisk(int pageNumber);
// int getOldestEntry(int tlbSize);
// double getAvgTimeInBackingStore();

// int main(int argc, char const *argv[])
// {

//     // Initialize the PD, TLB, and main memory
//     tlbCache = createVmTable(MAX_TLB_ENTRIES);
//     pageDirectory = createVmTable(MAX_PAGE_TABLE_ENTRIES); // @TODO: Change to PD (Page Directory)
//     dram = createDRAM(MAX_FRAME_ENTRIES, PAGE_SIZE);

//     // open file with randomly generated addresses
//     address_file = fopen("./data/address_file.txt", "r");
//     if (address_file == NULL)
//     {
//         fprintf(stderr, "Error opening address file");
//         return -1;
//     }

//     disk_file = fopen("./data/disk_file.bin", "rb");
//     if (disk_file == NULL)
//     {
//         fprintf(stderr, "Error opening disk file");
//         return -1;
//     }

//     // read and allocate addresses to X amount of processes
//     while (fgets(addressReadBuffer, ADDRESS_BUFFER_LEN, address_file) != NULL)
//     {
//         int virtual_addr = atoi(addressReadBuffer);
//         // 32-bit masking function to extract page number
//         page_number = getPageNumber(PAGE_MASK, virtual_addr, 8);

//         // 32-bit masking function to extract page offset
//         offset_number = getOffset(OFFSET_MASK, virtual_addr);

//         translateAddress();
//     }

//     printf("\n-----------------------------------------------------------------------------------\n");
//     // calculate and print out the stats
//     // printf("\nResults Using %s Algorithm: \n", algoName);
//     // printf("Number of translated addresses = %d\n", translationCount);
//     double pfRate = (double)pageDirectory->pageFaultCount / (double)1000;
//     double TLBRate = (double)tlbCache->tlbHitCount / (double)1000;

//     printf("Page Faults = %d\n", pageDirectory->pageFaultCount);
//     printf("Page Fault Rate = %.3f %%\n", pfRate * 100);
//     printf("TLB Hits = %d\n", tlbCache->tlbHitCount);
//     printf("TLB Hit Rate = %.3f %%\n", TLBRate * 100);
//     printf("Average time spent retrieving data from backing store: %.3f millisec\n", getAvgTimeInBackingStore());
//     printf("\n-----------------------------------------------------------------------------------\n");

//     // close address file and disk file
//     fclose(address_file);
//     fclose(disk_file);

//     // deallocate dynamically allocated memory for tables and main memory
//     freeVmTable(&tlbCache);
//     freeVmTable(&pageDirectory);
//     freeDRAM(&dram, MAX_FRAME_ENTRIES);

//     return 0;
// }

// void translateAddress()
// {
//     // First try to get page from TLB
//     int frame_number = -1; // Assigning -1 to frame_number means invalid initially

//     /*
//         Look through the TLB to see if memory page exists.
//         If found, extract frame number and increment hit count
//     */
//     for (int i = 0; i < tlbCache->length; i++)
//     {
//         if (tlbCache->pageNumArr[i] == page_number)
//         {
//             frame_number = tlbCache->frameNumArr[i];
//             tlbCache->tlbHitCount++;
//             break;
//         }
//     }

//     /*
//         We now need to see if there was a TLB miss.
//         If so, increment the tlb miss count and go through
//         the page table to see if the page number exists. If not on page table,
//         read secondary storage (dram) and increment page table fault count.
//     */
//     if (frame_number == -1)
//     {
//         tlbCache->tlbMissCount++; // Increment the miss count
//         // walk the contents of the page table
//         for (int i = 0; i < nextPage; i++)
//         {
//             if (pageDirectory->pageNumArr[i] == page_number)
//             {                                                 // If the page is found in those contents
//                 frame_number = pageDirectory->frameNumArr[i]; // Extract the frame_number
//                 break;                                        // Found in pageDirectory
//             }
//         }
//         // NOTE: Page Table Fault Encountered
//         if (frame_number == -1)
//         {                                    // if the page is not found in those contents
//             pageDirectory->pageFaultCount++; // Increment the number of page faults
//             // page fault, call to readFromDisk to get the frame into physical memory and the page table
//             start = clock();
//             readFromDisk(page_number);
//             cpu_time_used += (double)(clock() - start) / CLOCKS_PER_SEC;
//             functionCallCount++;
//             frame_number = nextFrame - 1; // And set the frame_number to the current nextFrame index
//         }
//     }

//     tlbInsertLRU(page_number, frame_number);

//     // Frame number and offset used to get the signed translatedValue stored at that address
//     translatedValue = dram[frame_number][offset_number];

//     // FOR TESTING -- printf("\nFrame Number: %d; Offset: %d; ", frame_number, offset_number);

//     // if (display_choice == 'y')
//     // {
//     // output the virtual address, physical address and translatedValue of the signed char to the console
//     printf("\nVirtual address: %d\t\tPhysical address: %d\t\tValue: %d", virtual_addr, (frame_number << 8) | offset_number, translatedValue);
//     // }
// }

// void tlbInsertFIFO()
// {
// }

// void tlbInsertLRU(int pageNumber, int frameNumber)
// {
//     bool freeSpotFound = false;
//     bool alreadyThere = false;
//     int replaceIndex = -1;

//     // SEEK -- > Find the index to replace and increment age for all other entries
//     for (int i = 0; i < MAX_TLB_ENTRIES; i++)
//     {
//         if ((tlbCache->pageNumArr[i] != pageNumber) && (tlbCache->pageNumArr[i] != 0))
//         {
//             // If entry is not in TLB and not a free spot, increment its age
//             tlbCache->entryAgeArr[i]++;
//         }
//         else if ((tlbCache->pageNumArr[i] != pageNumber) && (tlbCache->pageNumArr[i] == 0))
//         {
//             // Available spot in TLB found
//             if (!freeSpotFound)
//             {
//                 replaceIndex = i;
//                 freeSpotFound = true;
//             }
//         }
//         else if (tlbCache->pageNumArr[i] == pageNumber)
//         {
//             // Entry is already in TLB -- Reset its age
//             if (!alreadyThere)
//             {
//                 tlbCache->entryAgeArr[i] = 0;
//                 alreadyThere = true;
//             }
//         }
//     }

//     // REPLACE
//     if (alreadyThere)
//     {
//         return;
//     }
//     else if (freeSpotFound)
//     {                                                    // If we found a free spot, insert
//         tlbCache->pageNumArr[replaceIndex] = pageNumber; // Insert into free spot
//         tlbCache->frameNumArr[replaceIndex] = frameNumber;
//         tlbCache->entryAgeArr[replaceIndex] = 0;
//     }
//     else
//     { // No more free spots -- Need to replace the oldest entry
//         replaceIndex = getOldestEntry(MAX_TLB_ENTRIES);
//         tlbCache->pageNumArr[replaceIndex] = pageNumber; // Insert into oldest entry
//         tlbCache->frameNumArr[replaceIndex] = frameNumber;
//         tlbCache->entryAgeArr[replaceIndex] = 0;
//     }
// }

// void readFromDisk(int pageNumber)
// {
//     // first seek to byte PAGE_READ_SIZE in the backing store
//     // SEEK_SET in fseek() seeks from the beginning of the file
//     if (fseek(disk_file, pageNumber * DISK_BUFFER_LEN, SEEK_SET) != 0)
//     {
//         fprintf(stderr, "Error seeking in backing store\n");
//     }

//     // now read PAGE_READ_SIZE bytes from the backing store to the fileReadBuffer
//     if (fread(diskReadBuffer, sizeof(signed char), DISK_BUFFER_LEN, disk_file) == 0)
//     {
//         fprintf(stderr, "Error reading from backing store\n");
//     }

//     // Load the bits into the first available frame in the physical memory 2D array
//     for (int i = 0; i < DISK_BUFFER_LEN; i++)
//     {
//         dram[nextFrame][i] = diskReadBuffer[i];
//     }

//     // Then we want to load the frame number into the page table in the next frame
//     pageDirectory->pageNumArr[nextPage] = pageNumber;
//     pageDirectory->frameNumArr[nextPage] = nextFrame;

//     // increment the counters that track the next available frames
//     nextFrame++;
//     nextPage++;
// }

// // Finds the oldest entry in TLB and returns the replacement Index
// int getOldestEntry(int tlbSize)
// {
//     int i, max, index;

//     max = tlbCache->entryAgeArr[0];
//     index = 0;

//     for (i = 1; i < tlbSize; i++)
//     {
//         if (tlbCache->entryAgeArr[i] > max)
//         {
//             index = i;
//             max = tlbCache->entryAgeArr[i];
//         }
//     }
//     return index;
// }

// // Just computes the average time complexity of accessing the backing store
// double getAvgTimeInBackingStore()
// {
//     double temp = (double)cpu_time_used / (double)functionCallCount;
//     return temp * 1000000;
// }
