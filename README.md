# RISC-V Pagetable C Example Functions
This is a C repository that contains helper functions for RISC-V Pagetables. Currently, only Sv32 is supported, but all Sv modes will be supported in the future.

Currently, this code compiles only for a m32 gcc build, as the risc-v cross-compiler does not support posix_memalign. In the future, this function will be replaced by a custom memory allocator to allow building on RISC-V. At this time, this repository only serves as an EXAMPLE of how to implement these functions on a RISC-V system, not as a true implementation.

Also, it does not support 64 bit binaries at this time, as the Sv32 emulation at this time is dependent on system address size.

This code is not production code, and serves only as an example on how to navigate RISC-V page table structures. It has no guarantees of correctness, nor does it claim to represent the best way to deal with these structures.

Contributions and suggestions are welcome.

# How to build
Navigate to the root of the repository, create a build directory, and run ../make m32. The binary can then be executed as ./pgtbl.

# Things that are TODO
* Sv39 and up
* Svnapot
* Svpbmt
* A and D bit
* Access types when performing page table walk to check for page fault
