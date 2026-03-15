# satellite-network-sim
Binary tree–based satellite communication simulator in C with Min-Heap node management, encoding/decoding routines, and distance computation.

## What it does
Models a satellite network where communication channels are optimized 
using Huffman encoding. Satellites are represented as nodes in a binary 
tree built with a Min-Heap, with higher-frequency satellites receiving 
shorter codes for efficient transmission.

The program supports 4 operations:
- **-c1** — Build and display the Huffman tree level by level
- **-c2** — Decode a binary message using the Huffman tree
- **-c3** — Encode a list of satellite names into binary codes
- **-c4** — Find the lowest common ancestor of a set of satellites

## How to Build & Run

**Build:**
```bash
make build
```

**Run:**
```bash
./tema2 -c1 input.txt output.txt
./tema2 -c2 input.txt output.txt
./tema2 -c3 input.txt output.txt
./tema2 -c4 input.txt output.txt
```

**Clean:**
```bash
make clean
```

## Input Format
Each input file starts with `N` (number of satellites), followed by 
`N` lines of `frequency name` pairs. Tasks 2, 3, and 4 have additional 
input depending on the operation.

## Tech Stack
C, Min-Heap, Huffman Binary Tree, GCC

## Project Structure
├── satellite-sim.c      # Full implementation
└── Makefile     # Build configuration
