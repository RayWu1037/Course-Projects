# Lists (CS 251 Project 3)

This project implements custom list data structures in C++ as part of the CS 251 programming course. There are two main components:

- **circvector** – a circular vector implementation that supports dynamic resizing and allows efficient push, pop, and indexed access operations.  It is defined in `circvector.h` and tested by `circvector_tests.cpp`.
- **linkedlist** – a doubly linked list implementation with support for insertion, deletion, and traversal operations.  It is defined in `linkedlist.h` and tested by `linkedlist_tests.cpp`.

A command-line driver program, `list_main.cpp`, provides basic interactions with the list data structures for demonstration and debugging.  The compiled test binary `list_tests` runs the unit tests for both data structures.

The `build` directory contains the compiled object files used to build the test executables.  All source files and headers are included in this folder.

This assignment reinforces understanding of dynamic memory management, object-oriented design, and unit testing in C++.
