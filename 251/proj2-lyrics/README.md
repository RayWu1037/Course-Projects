### Lyrics Search Engine (CS 251 Project 2)

This project implements a text search engine in C++ for song lyrics. It tokenizes and cleans words from a corpus of lyrics, builds an inverted index mapping tokens to their occurrences, and processes search queries to return matching songs.

Key components include:

- **build_indices**: Construct an inverted index from the lyrics data files.
- **clean_token** and **gather_tokens**: Normalize and extract tokens from input text.
- **find_query_matches**: Find matching documents for a given search query using the index.
- **search_engine**: Command-line interface to interact with the lyrics search engine.

The project includes test suites for each component and example data files under the `data/` directory. It was developed for the CS 251 Data Structures course to practice file I/O, tokenization, and use of maps and sets in C++.
