#pragma once

#include <map>
#include <set>
#include <string>

using namespace std;

/**
 * Cleans the provided `token` by returning a new string with all leading and
 * trailing punctuation removed. If the `token` contains no letters, returns
 * the empty string to indicate that it should be discarded.
 *
 * Assumes that `token` contains no spaces.
 */
string cleanToken(const string& token);

/**
 * Splits the input text into words ("tokens"), cleans each token, and returns
 * the set of all tokens in the text.
 */
set<string> gatherTokens(const string& text);

/**
 * Builds two indices from the contents of the given file.
 * The lines in the file come in triples: a song title, the artist, and its
 * corresponding lyrics.
 *
 * To build the inverted index: for each lyrics, the function extracts and
 * cleans all tokens, then associates each token with the song title in the
 * provided `inverted_index`. Thus, `inverted_index` maps tokens to the songs
 * they appear in.
 *
 * The second index (`artist_index`), should map a song title to the artist
 * that perfoms it.
 */
int buildIndices(const string& filename,
                 map<string, set<string>>& inverted_index,
                 map<string, string>& artist_index);

/**
 * Finds matching songs for a given query sentence by looking up each token in
 * the provided index. The sentence is parsed from left to right, and each token
 * lookup is combined using either set union (no modifier), set intersection
 * (+), or set difference (-).
 *
 * Assumes the sentence is "well-formed" with at least one search term.
 */
set<string> findQueryMatches(const map<string, set<string>>& index,
                             const string& sentence);

/**
 * Given a filename, builds the indices from the file and repeatedly prompts the
 * user for search queries. For each query, outputs the number of matching songs
 * and a list of the artists and titles for each song that was found. Exits when
 * the user presses enter without entering a query.
 *
 * The output of each artist and title for each line should be of the form:
 *   "<title> by <artist>"
 */
void searchEngine(const string& filename);
