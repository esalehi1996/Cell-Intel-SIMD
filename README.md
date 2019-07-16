# Parallelized MatMul Performance comparison between Intel SIMD and Cell SPU

Parallel Matmul is implemented on Cell processor using the SPUs for the bulk of the computational load and the PPU for most of the control load. A makefile is created to facilitate the compilation process. The Cell source code is in the "Cell/" directory.

The parallel Matmul is also implemented using the SIMD functionality of the Intel processor. The source code for that is in the "SIMD/" directory.
