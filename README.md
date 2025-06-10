MyCacheSim: A Basic Cache Simulator

Project Overview:
MyCacheSim is a C++ implementation of a basic cache simulator. It models the behavior of a single-level cache, taking various parameters such as cache size, associativity, block size, replacement policy, and write policy. The simulator processes memory access trace files and outputs key performance statistics, including hit rates and total simulation runtime.

This project was developed as a learning exercise, inspired by the "Project 1: Cache Simulator" assignment from the UC San Diego CSE 240A (Fall 2007) - Principles of Computer Architecture course.

Features Implemented:
Configurable Cache Parameters: Supports user-defined cache configurations.

Replacement Policies:
Random Replacement
Least Recently Used (LRU)

Write Policies:
Write-Around (No Write Allocate)
Write-Allocate (Write-Back)

Detailed Statistics Output: Calculates and reports:
Total Hit Rate
Load Hit Rate
Store Hit Rate
Total Run Time (in cycles)
Average Memory Access Latency (AMAL)

Inspiration / Acknowledgment:
This project was directly inspired by the cache simulator assignment for CSE 240A (Principles of Computer Architecture) at UC San Diego, specifically the Fall 2007 offering as detailed at https://cseweb.ucsd.edu/classes/fa07/cse240a/project1.html.

My implementation of the cache simulator is entirely my own work. While the problem statement and the general format for configuration and trace files are derived from the UCSD project, I have rephrased the problem description in this README to avoid direct reproduction of copyrighted material.

Important Note regarding Trace Files:
The trace files (gzip.trace, gcc.trace, etc.) used for testing and development of this simulator were originally provided as part of the UC San Diego CSE 240A course materials. Due to potential copyright considerations and the large size of these files, I have not included them in this repository. Users can download these files directly from the original UCSD course website.

Getting Started:

Prerequisites:
A C++ compiler (e.g., g++)
make (for building with the provided Makefile, if applicable)

Building the Project:
Clone this repository:
Bash

git clone https://github.com/As-star-at/Cache-Simulator.git
cd MyCacheSim
Compile the source code. Assuming your main source file is cache_sim.cpp (or main.cpp), you can use:
Bash

g++ cache_sim.cpp -o cache_sim
(Or, if you have a Makefile):
Bash

make
Running the Simulator
The simulator expects two command-line arguments: a configuration file and a trace file.

Bash

./cache_sim <config_file> <trace_file>
<config_file>: Path to the cache configuration file (e.g., configs/mega.conf).
<trace_file>: Path to the memory access trace file (e.g., /path/to/your/downloaded/gzip.trace).
The simulator will write its output to a file with a .out extension, based on the input trace file name. For example, if you run ./cache_sim config.conf gcc.trace, the output will be in gcc.trace.out.

File Formats
Configuration File (.conf example: configs/mega.conf)
The configuration file defines the parameters for the cache. Each line specifies a different parameter in a specific order.
(Based on the UCSD project link)

Line size (Block Size): Size for the cache line/block in bytes (non-negative power of 2, e.g., 1, 2, 4, 8, ...).
Associativity: The associativity of the cache. 1 for direct-mapped, 0 for fully-associative, otherwise a non-negative power of 2 (e.g., 2, 4, 8).
Data size (Cache Size): Total size of the data in the cache in KB (non-negative power of 2, e.g., 64, 128).
Replacement Policy: 0 for Random, 1 for LRU.
Miss Penalty: Number of cycles penalized on a cache miss (positive integer).
Write Allocate: 0 for write-around (no write allocate), 1 for write-allocate.
Example mega.conf:

64      # Line size = 64 bytes
4       # 4-way Associativity
256     # Data size = 256KB
1       # LRU Replacement Policy
100     # Miss Penalty = 100 cycles
1       # Write-Allocate Policy

Trace File (.trace example: gzip.trace)
The trace file describes memory access patterns. Each line in the trace file represents a single memory operation.
(Based on the UCSD project link)
Each line contains:

Access Type: 'l' for a load (read), 's' for a store (write).
Memory Address: The 32-bit effective memory address in hexadecimal format.
Number of Non-Memory Ops: The number of CPU cycles that pass between the current memory access and the next one (integer).
Example Trace File Lines (hypothetical, format based on UCSD link):

l 0x8048000 5
s 0x8048004 2
l 0x8048008 10

Simulator Output Format:
Total Hit Rate: Percentage of all memory operations that were hits (truncated to 4 decimal places).
Load Hit Rate: Percentage of load operations that were hits (truncated to 4 decimal places).
Store Hit Rate: Percentage of store operations that were hits (truncated to 4 decimal places).
Total Run Time: The total simulated run time of the program in cycles.
Average Memory Access Latency (AMAL): The average number of cycles needed to complete a memory access (to 4 decimal places).

What I Learned:
Building this cache simulator was a valuable exercise in understanding the core principles of computer architecture and memory hierarchies. 

Key takeaways include:
1. Cache Organization: Gained practical understanding of how cache lines, sets, tags, and indices work together.
2. Replacement Policies: Explored the implementation details and performance implications of LRU and Random replacement.
3. Write Policies: Understood the differences and trade-offs between write-through/write-around and write-back/write-allocate.
4. Performance Metrics: Learned to calculate and interpret critical cache performance metrics like hit rates and AMAL.
5. C++ for System-Level Simulation: Improved C++ programming skills in data structure management, bitwise operations, and file I/O for simulating hardware behavior.

Known Issues & Future Work:
1. The function for fully associative cache does not yield appropriate results. (Needs debugging/refinement)
2. The code works specifically for the format of trace files provided by UC San Diego (e.g., 'l'/'s' for ops, fixed address/non-mem op lengths). Expanding code for varied formats would be an improvement.
3. Code logic and formatting could be largely improved for better readability and maintainability.
