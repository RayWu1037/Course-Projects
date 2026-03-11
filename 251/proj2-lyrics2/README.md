# Lyrics Search Engine 2 (CS 251 Project 2)

This project is a C++ lyrics search engine. It tokenizes and cleans lyrics files, builds an inverted index mapping words to songs, and matches user queries to songs based on the index. The command-line interface (`lyrics_main.cpp`) lets users enter queries and receive results.

The project contains modules and tests for:

- **build_indices** – read lyric files and build a word-to-song index.
- **clean_token** and **gather_tokens** – normalize tokens by removing punctuation and duplicates.
- **find_query_matches** – given a query, find songs containing all query words.
- **search_engine** – interactive driver that ties everything together.

Provided `build`, `data`, `include`, and `tests` folders contain compiled objects, sample lyrics files, the `lyrics.h` header, and unit tests, respectively. This assignment is part of the CS 251 programming class.
