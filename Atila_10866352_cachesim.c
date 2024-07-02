//Written by Doruk Tan Atila - Student ID: 10866352
//19 April 2024
/*
Direct mapped cache simulator for write-back write-allocate architecture for the folllowing:
    4 to 256 blocks cache size
    2 to 32 16-bit words cache block size

TEST FILE MUST FOLLOW THE FORMAT:

! Any comment line starts with '!' - Any read access starts with 'R' - Any write access starts with 'W' - Data MUST be written in hexadecimal

! Below is a CPU read access with 12 bit address bus 16 bit data bus
R xxx yyyy

! Below is a CPU write access with 12 bit address bus 16 bit data bus
W xxx yyyy

! Example
R 000 0001
R 000 0002
W 001 0002
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// CHANGE ACCORDINGLY
#define ADDRESS_BUS_BITS 16
#define DATA_BUS_BITS 16

void parseLine(char *line, unsigned int *address, unsigned int *data);
bool isPowerOfTwo(int num);

int main(unsigned int argc, char *argv[]) {
    //-------------------
    // Command prompt input
    // Check if the correct number of command-line arguments are provided
    if (argc != 4) {
        printf("Error: Please provide the filename, cacheblocks, and blockwords as command-line arguments.\n");
        printf("Usage: %s <tracefilename> <cacheblocks> <blockwords>\n", argv[0]);
        return 1;
    }
    char *filename = argv[1];                       // Get the filename from command-line argument
    unsigned int cacheblocks = atoi(argv[2]);       // Convert the second command-line argument to integer
    unsigned int blockwords = atoi(argv[3]);        // Convert the third command-line argument to integer

    // Open the file
    FILE *memory_trace_file = fopen(filename, "r");

    // Error checking
    if (memory_trace_file == NULL) {
        printf("Error: Unable to open file %s\n", filename);
        return 1;
    }
    if (2 > cacheblocks || 256 < cacheblocks){
        printf("Error: cacheblocks must be between 4 & 256 %s\n", filename);
        return 1;
    }
    if(!isPowerOfTwo(cacheblocks)){
        printf("Error: cacheblocks must be power of 2 %s\n", filename);
        return 1;
    }
    if (2 > blockwords || 32 < blockwords){
        printf("Error: blockwords must be between 2 & 32 %s\n", filename);
        return 1;
    }
    if(!isPowerOfTwo(blockwords)){
        printf("Error: blockwords must be power of 2 %s\n", filename);
        return 1;
    }
    //-------------------

    //-------------------
    // Output parameters
    unsigned int CPUR = 0;              // Total number of CPU read accesses
    unsigned int CPUW = 0;              // Total number of CPU write accesses
    unsigned int NRA = 0;               // Total number of read accesses to the external (main) memory
    unsigned int NWA = 0;               // Total number of write accesses to the external (main) memory
    unsigned int NCRH = 0;              // Number of cache read hits
    unsigned int NCRM = 0;              // Number of cache read misses
    unsigned int NCWH = 0;              // Number of cache write hits
    unsigned int NCWM = 0;              // Number of cache write misses
    //--------------------

    //--------------------
    // Parameters for data & address read/parse
    unsigned int address;                                // Address bits in int
    unsigned int data;                                   // Data bits in int
    char read_buffer[255];
    //--------------------

    //--------------------
    // Parameters for memory mapping
    unsigned int block_offset;
    unsigned int cmbid;
    unsigned int tag_bits;
    unsigned int mmbid;
    unsigned int no_of_bits_blockoffset = log2(blockwords);
    unsigned int no_of_bits_cmbid = log2(cacheblocks);
    unsigned int no_of_bits_tag_bits = ADDRESS_BUS_BITS - (no_of_bits_blockoffset + no_of_bits_cmbid);
    //--------------------

    //--------------------
    // Valid, dirty and tag bits declared and initialized for all cache blocks
    bool *cache_valid_bits = (bool *)malloc(cacheblocks * sizeof(bool));
    for (size_t i = 0; i < cacheblocks; i++) {
        cache_valid_bits[i] = false;
    }    
    bool *cache_dirty_bits = (bool *)malloc(cacheblocks * sizeof(bool));
    unsigned int *cache_tag_bits = (unsigned int *)malloc(cacheblocks * sizeof(unsigned int)); // Initialization is not needed
    //--------------------

    while(fgets(read_buffer, 255, memory_trace_file)) {
        switch (read_buffer[0]){
            case '!':   // Comment
                break;
            
            case 'R':   // Read
                CPUR++;                                         // CPU read access
                parseLine(read_buffer, &address, &data);        // Instruction parsed into address and data
                
                //--------------------
                // Setting mmbid, cmbid and tag bits
                mmbid = address / blockwords;
                cmbid = mmbid % cacheblocks;
                tag_bits = address >> (ADDRESS_BUS_BITS - no_of_bits_tag_bits);
                //--------------------

                //--------------------
                // Case where valid bit is 0 -> Cache read miss, move block from main memory to cache, return data
                if (cache_valid_bits[cmbid] == false){
                    cache_tag_bits[cmbid] = tag_bits;           // Changing tag bits to the block means moving the block
                    cache_valid_bits[cmbid] = true;             // Valid bit set
                    cache_dirty_bits[cmbid] = false;            // Dirty bit reset
                    
                    NCRM++;                                     // Cache read miss 
                    NRA += blockwords;                          // Read from main memory the whole block
                    break;
                }
                //--------------------

                else if (tag_bits != cache_tag_bits[cmbid]){
                    //--------------------
                    // Case where valid bit 1 / Tag NO match / Dirty bit 0 -> Move block from main memory to cache, return data
                    if(cache_dirty_bits[cmbid] == false){
                        cache_tag_bits[cmbid] = tag_bits;       // Changing tag bits to the block means moving the block
                        
                        NCRM++;                                 // Cache read miss
                        NRA += blockwords;                      // Read from main memory the whole block
                        break;
                    //--------------------
                    }
                    //--------------------
                    // Case where valid bit 1 / Tag NO match / Dirty bit 1 -> Write-back block from cache to main memory, move block from main memory to cache, return data
                    else if(cache_dirty_bits[cmbid] == true){
                        // Since dirty bit is false, first write back to main memory then replace the block
                        cache_tag_bits[cmbid] = tag_bits;       // Changing tag bits to the block means moving the block
                        cache_dirty_bits[cmbid] = false;        // Dirty bit reset

                        NCRM++;                                 // Cache read miss
                        NWA += blockwords;                      // Write-back whole block back to main memory
                        NRA += blockwords;                      // Read from main memory the whole block
                        break;
                    }
                    //--------------------
                }

                // Valid bit 1 / Tag match -> Cache hit, return data
                NCRH++;
                break;

            case 'W':   // Write
                CPUW++;                                         // CPU write access
                parseLine(read_buffer, &address, &data);        // Instruction parsed into address and data

                //--------------------
                // Setting mmbid, cmbid and tag bits
                mmbid = address / blockwords;
                cmbid = mmbid % cacheblocks;
                tag_bits = address >> (ADDRESS_BUS_BITS - no_of_bits_tag_bits);
                //--------------------

                //--------------------
                // Case where valid bit is 0 -> Cache write miss, move block from main memory to cache, write data
                if (cache_valid_bits[cmbid] == false){
                    cache_tag_bits[cmbid] = tag_bits;           // Changing tag bits to the block means moving the block
                    cache_valid_bits[cmbid] = true;             // Valid bit set
                    cache_dirty_bits[cmbid] = true;             // Dirty bit set

                    NCWM++;                                     // Cache write miss
                    NRA += blockwords;                          // Read from main memory the whole block
                    break;
                }
                //--------------------

                else if (tag_bits != cache_tag_bits[cmbid]){
                    //--------------------
                    // Case where valid bit 1 / Tag NO match / Dirty bit 0 -> Move block from main memory to cache, write data
                    if(cache_dirty_bits[cmbid] == false){
                        cache_tag_bits[cmbid] = tag_bits;       // Changing tag bits to the block means moving the block
                        cache_dirty_bits[cmbid] = true;         // Dirty bit set

                        NCWM++;                                 // Cache write miss
                        NRA += blockwords;                      // Read from main memory the whole block
                        break;
                    //--------------------
                    }
                    //--------------------
                    // Case where valid bit 1 / Tag NO match / Dirty bit 1 -> Write-back block from cache to main memory, move block from main memory to cache, write data
                    else if(cache_dirty_bits[cmbid] == true){
                        // Since dirty bit is false, first write back to main memory then replace the block
                        cache_tag_bits[cmbid] = tag_bits;       // Changing tag bits to the block means moving the block
                        cache_dirty_bits[cmbid] = true;         // Dirty bit set

                        NCWM++;                                 // Cache write miss
                        NWA += blockwords;                      // Write-back whole block back to main memory
                        NRA += blockwords;                      // Read from main memory the whole block
                        break;
                    }
                    //--------------------
                    break;
                }

                // Valid bit 1 / Tag match -> Cache hit, write data
                NCWH++;
                cache_dirty_bits[cmbid] = true;                 // Setting dirty bit
                break;
            
            default:
                break;
        }

    }
    printf("%u, %u, %u, %u, %u, %u, %u, %u", CPUR, CPUW, NRA, NWA, NCRH, NCRM, NCWH, NCWM);

    free(cache_dirty_bits);
    free(cache_tag_bits);
    free(cache_valid_bits);
    fclose(memory_trace_file); // Close the file
    return 0;

}

void parseLine(char *line, unsigned int *address, unsigned int *data){
    char *ptr;
    char address_[ADDRESS_BUS_BITS / 4 + 1];                            // Array to store address bits in hex format + null terminator
    char data_[DATA_BUS_BITS / 4 + 1];                                  // Array to store data bits in hex format + null teminator

    strncpy(address_, line + 2, sizeof(address_) - 1);                  // Parse address
    address_[sizeof(address_) - 1] = '\0';                              // Null-terminate address
    *address = strtoul(address_, &ptr, 16);                             // Convert address in hex string to integer
    strncpy(data_, line + 2 + sizeof(address_), sizeof(data_) - 1);     // Parse data
    data_[sizeof(data_) - 1] = '\0';                                    // Null-terminate data
    *data = strtoul(data_, &ptr, 16);                                   // Convert address in hex string to integer
}

bool isPowerOfTwo(int num) {
    // Written with the help of ChatGPT
    // A number is a power of 2 if it has only one bit set
    // So, if we AND the number with its (number - 1), it should be zero
    // For example, 8 (binary: 1000) & 7 (binary: 0111) = 0

    if (num <= 0) {
        return false; // 0 and negative numbers are not powers of 2
    }

    return (num & (num - 1)) == 0;
}