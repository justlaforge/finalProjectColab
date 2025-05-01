README.txt
==========

CSCI 473 – Intro to Parallel Systems – SP25  
Assignment 7 – Final Project  
Title: Parallel 2D Heat Transfer Simulation using 9-Point Stencil

Authors:
Justin Michael LaForge  
Kyle William Wallace

Overview:
---------
This project implements a 2D stencil-based heat transfer simulation using four parallel programming models:
1. Pthreads
2. OpenMP
3. MPI
4. Hybrid (MPI + OpenMP + Pthreads)

The simulation performs iterative heat distribution updates on a matrix, where the left and right sides are heat sources (value = 1.0) and the top and bottom are freezing sources (value = 0.0). The simulation is compared to a serial version for correctness and performance evaluation.

Directory Structure:
--------------------
./code/                        - Source code and helper scripts  
  ├── make-2d.c                - Generates initial matrix with boundary conditions  
  ├── print-2d.c               - Prints matrix from file in human-readable format  
  ├── stencil-2d.c             - Serial implementation of 9-point stencil  
  ├── stencil-2d-pth.c         - Pthreads implementation  
  ├── stencil-2d-omp.c         - OpenMP implementation  
  ├── stencil-2d-mpi.c         - MPI implementation  
  ├── stencil-2d-hybrid.c      - Hybrid MPI + OpenMP + Pthreads implementation  
  ├── utilities.h              - Header for shared utilities  
  ├── utilities.c              - Implementation of shared utility functions  
  ├── Makefile                 - Makefile to compile all implementations  
  └── sbatch.bash              - SLURM batch script for running experiments on Expanse

./presentation/                - Final project presentation (PDF + video link)  
./report/                      - Final project report (PDF + LaTeX source ZIP)  
./data/                        - Input/output matrix data used for validation and experiments  
./README.txt                   - This documentation file

Compiling the Code:
-------------------
Navigate to the ./code directory and run:

    make clean
    make all

This will compile all source files into corresponding executables.

Running the Programs:
---------------------
1. **Serial Version**:
    ./stencil-2d -n <iterations> -i <input_file> -o <output_file> -v <debug_level>

2. **Pthreads / OpenMP / MPI / Hybrid**:
    ./stencil-2d-XX -t <iterations> -i <input_file> -o <output_file> -p <num_threads>

Where:
  - `XX` is one of `pth`, `omp`, `mpi`, `hybrid`
  - `<debug_level>`: 
      0 = silent,  
      1 = minimal debug,  
      2 = verbose (prints matrix per iteration)

Example:
    ./stencil-2d-omp -t 100 -i input-5k.raw -o output-5k.raw -p 8

Input Format:
-------------
The input matrix files are in raw float format, with fixed values:
- Left/Right walls = 1.0 (heat)
- Top/Bottom walls = 0.0 (freeze)
- Interior = 0.0 initially

Use `make-2d` to generate an input file:

    ./make-2d 5000 5000 input-5k.raw

Use `print-2d` to view a matrix:

    ./print-2d input-5k.raw 5000 5000

Experiments:
------------
Each implementation was tested across:
  - Matrix sizes: 5k, 10k, 20k, 40k
  - Threads/Processes: 1, 2, 4, 8, 16
  - 20 experiments per implementation × 5 implementations = 100 experiments

Metrics gathered:
  - T_overall, T_computation, T_other
  - Speedup, Efficiency (overall and computation)
  - Visual matrix plots and time-lapse GIFs

Performance plots and comparisons are included in the report.

Important Notes:
----------------
- Ensure input files match the matrix dimensions given to the programs.
- Results were verified by comparing output matrices across all implementations.
- Timing measurements include overall runtime, computation-only time, and derived overhead.

Group Contribution:
-------------------
Justin Michael LaForge: Serial, Pthreads, Hybrid, Heatmap, Report, Slides  
Kyle William Wallace: OpenMP, MPI, Hybrid, Data Collection, Experiments, Report, Slides


References:
-----------
- Ghost Cell Pattern: https://fredrikbk.com/publications/ghost_cell_pattern.pdf  
- Iterative Stencil Loops: https://en.wikipedia.org/wiki/Iterative_Stencil_Loops  
- Pthread Synchronization: https://medium.com/@jaydesai36/barrier-synchronization-in-threads-3c56f947047  
- Quinn’s Block Macros and Parallel Design Techniques (Lecture Materials)

