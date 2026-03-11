#include "include/lyrics.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;


string cleanToken(const string& token) {
  // Convert to lowercase and work on a mutable copy
  string lower;
  lower.reserve(token.size());
  for (unsigned char ch : token) {
    // tolower expects an int; cast to unsigned char first to avoid UB
    lower.push_back(static_cast<char>(std::tolower(ch)));
  }

  // Remove leading punctuation characters
  size_t start = 0;
  size_t end = lower.size();
  while (start < end && std::ispunct(static_cast<unsigned char>(lower[start]))) {
    ++start;
  }
  // Remove trailing punctuation characters
  while (end > start && std::ispunct(static_cast<unsigned char>(lower[end - 1]))) {
    --end;
  }

  // Extract the cleaned substring
  string cleaned = lower.substr(start, end - start);

  // If no alphabetic characters remain, discard token
  bool has_alpha = false;
  for (unsigned char ch : cleaned) {
    if (std::isalpha(ch)) {
      has_alpha = true;
      break;
    }
  }
  if (!has_alpha) {
    return "";
  }

  return cleaned;
}

set<string> gatherTokens(const string& text) {
  set<string> tokens;
  istringstream iss(text);
  string raw;
  while (iss >> raw) {
    string cleaned = cleanToken(raw);
    if (!cleaned.empty()) {
      tokens.insert(cleaned);
    }
  }
  return tokens;
}

// Helper to strip a trailing carriage return (for Windows line endings)
static inline void strip_cr(string& s) {
  if (!s.empty() && s.back() == '\r') {
    s.pop_back();
  }
}

int buildIndices(const string& filename,
                 map<string, set<string>>& inverted_index,
                 map<string, string>& artist_index) {
  ifstream infile(filename);
  if (!infile.is_open()) {
    // File missing: no modifications, return 0
    return 0;
  }
  string title;
  string artist;
  string lyrics;
  int count = 0;
  while (true) {
    if (!getline(infile, title)) {
      break;
    }
    if (!getline(infile, artist)) {
      break;
    }
    if (!getline(infile, lyrics)) {
      break;
    }

    // Handle Windows \r endings
    strip_cr(title);
    strip_cr(artist);
    strip_cr(lyrics);

    // Map title to artist
    artist_index[title] = artist;

    // Build token set for lyrics
    set<string> toks = gatherTokens(lyrics);
    for (const auto& tok : toks) {
      inverted_index[tok].insert(title);
    }
    ++count;
  }
  return count;
}

// Internal helper functions for set operations
static set<string> set_union_(const set<string>& a, const set<string>& b) {
  set<string> out;
  set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(out, out.begin()));
  return out;
}
static set<string> set_intersection_(const set<string>& a, const set<string>& b) {
  set<string> out;
  set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter(out, out.begin()));
  return out;
}
static set<string> set_difference_(const set<string>& a, const set<string>& b) {
  set<string> out;
  set_difference(a.begin(), a.end(), b.begin(), b.end(), inserter(out, out.begin()));
  return out;
}

set<string> findQueryMatches(const map<string, set<string>>& index,
                             const string& sentence) {
  istringstream iss(sentence);
  string token;

  // Read the first term; ignore a leading '+' or '-' if present
  if (!(iss >> token)) {
    return {};
  }
  if (!token.empty() && (token[0] == '+' || token[0] == '-')) {
    token = token.substr(1);
  }
  string term = cleanToken(token);
  set<string> current;
  if (!term.empty()) {
    auto it = index.find(term);
    if (it != index.end()) {
      current = it->second;
    }
  }

  // Process remaining terms left-to-right
  while (iss >> token) {
    char op = 0;
    if (!token.empty() && (token[0] == '+' || token[0] == '-')) {
      op = token[0];
      token = token.substr(1);
    }
    string cleaned = cleanToken(token);
    set<string> rhs;
    if (!cleaned.empty()) {
      auto it2 = index.find(cleaned);
      if (it2 != index.end()) {
        rhs = it2->second;
      }
    }

    if (op == '+') {
      current = set_intersection_(current, rhs);
    } else if (op == '-') {
      current = set_difference_(current, rhs);
    } else {
      current = set_union_(current, rhs);
    }
  }
  return current;
}

void searchEngine(const string& filename) {
  map<string, set<string>> inverted_index;
  map<string, string> artist_index;

  ifstream test(filename);
  if (!test.is_open()) {
    cout << "Invalid filename." << endl;
  }

  cout << "Stand by while building indices..." << endl;
  int num_songs = buildIndices(filename, inverted_index, artist_index);

  set<string> uniq_artists;
  for (const auto& [title, artist] : artist_index) {
    uniq_artists.insert(artist);
  }
  cout << "Indexed " << num_songs << " songs containing "
       << inverted_index.size() << " unique terms and "
       << uniq_artists.size() << " artists." << endl;

  string line;
  while (true) {
    cout << "Enter query sentence (press enter to quit): ";
    if (!getline(cin, line)) break;

    if (line.empty()) {
      cout << "Thank you for searching our Lyrics DB!" << endl;
      break;
    }

    set<string> matches = findQueryMatches(inverted_index, line);
    cout << "Found " << matches.size() << " matching songs" << endl;

    vector<pair<string,string>> results;
    results.reserve(matches.size());
    for (const auto& title : matches) {
      auto it = artist_index.find(title);
      results.emplace_back(title, (it != artist_index.end()) ? it->second : "");
    }
    sort(results.begin(), results.end(),
         [](const auto& a, const auto& b){ return a.first < b.first; });

    for (const auto& [title, artist] : results) {
      cout << title << " by " << artist << endl;
    }
  }
}
