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
  // TODO student
  return "";
}

set<string> gatherTokens(const string& text) {
  // TODO student
  return {};
}

int buildIndices(const string& filename,
                 map<string, set<string>>& inverted_index,
                 map<string, string>& artist_index) {
  // TODO student
  return 0;
}

set<string> findQueryMatches(const map<string, set<string>>& index,
                             const string& sentence) {
  // TODO student
  return {};
}

void searchEngine(const string& filename) {
  // TODO student
}
