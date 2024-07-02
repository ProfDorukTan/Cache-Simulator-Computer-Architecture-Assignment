# Cache Simulator

## Overview

This project implements a direct-mapped cache simulator for a write-back, write-allocate architecture. The simulator supports cache sizes ranging from 4 to 256 blocks and cache block sizes ranging from 2 to 32 16-bit words (can be changed in source code).

## Features

- Direct-mapped cache simulation
- Write-back, write-allocate architecture
- Configurable cache size and block size
- Provides detailed statistics on cache performance
- Does not allow illogical cache configurations (powers of 2)


## Prerequisites

To compile and run the cache simulator, you need:
- A C compiler (e.g., GCC)
- A terminal or command prompt

## Installation

1. Clone the repository:
   ```sh
   git clone https://github.com/yourusername/Cache-Simulator.git
   cd Cache-Simulator
    ```

2. Compile the source code:
    ```sh
    gcc -o cachesim Atila_10866352_cachesim.c
    ```
    
## Usage
To run the cache simulator, use the following command:

    ./cachesim <tracefilename> <cacheblocks> <blockwords>

### Example
    ./cachesim bubble_sort_trace_096.txt 4 8

## Output
The output will be in the format:
    
    CPUR, CPUW, NRA, NWA, NCRH, NCRM, NCWH, NCWM
where:

    CPUR: Total number of CPU read accesses
    CPUW: Total number of CPU write accesses
    NRA: Total number of read accesses to the external (main) memory
    NWA: Total number of write accesses to the external (main) memory
    NCRH: Number of cache read hits
    NCRM: Number of cache read misses
    NCWH: Number of cache write hits
    NCWM: Number of cache write misses

### Example Output
    1101154, 384592, 361952, 332456, 1055910, 45244, 384592, 0

## Input File Format
The input file should contain a series of memory access operations, one per line, with the following format:

    Any comment line starts with !
    Any read access starts with R
    Any write access starts with W
    Data must be written in hexadecimal

### Example Input File
    ! Example trace file
    R 0000 0001
    R 0000 0F02
    W 0010 0F02

### Considerations for Different Bus Sizes
The example input file provided uses a 16-bit address and a 16-bit data bus configuration. Ensure that the data bus sizes in your input file match the configuration specified in the source code. If necessary, you can adjust the bus sizes in the source code by modifying the following definitions:

    // CHANGE ACCORDINGLY
    #define ADDRESS_BUS_BITS 16
    #define DATA_BUS_BITS 16
