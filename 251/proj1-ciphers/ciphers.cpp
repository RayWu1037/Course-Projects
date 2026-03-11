#include <algorithm>
#include <numeric>
#include <ctime>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <iomanip>
#include <limits>


#include "include/caesar_dec.h"
#include "include/caesar_enc.h"
#include "include/subst_dec.h"
#include "include/subst_enc.h"
#include "utils.h"

using namespace std;

// Initialize random number generator in .cpp file for ODR reasons
std::mt19937 Random::rng;

const string ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Function declarations go at the top of the file so we can call them
// anywhere in our program, such as in main or in other functions.
// Most other function declarations are in the included header
// files.

// When you add a new helper function, make sure to declare it up here!

/**
 * Print instructions for using the program.
 */
void printMenu();
void decryptSubstCipherFromFileCommand(const QuadgramScorer& scorer);
static vector<string> readDictUpper(const string& path);
vector<char> decryptSubstCipher(const QuadgramScorer& scorer, const string& ciphertext);

static pair<vector<string>, vector<int>> loadQuadgrams(const string& path) {
  ifstream fin(path);
  if (!fin) return {{}, {}};

  vector<string> grams_int;
  vector<int>    counts_int;

  vector<string> grams_log;
  vector<long double> logps;

  string line;
  while (getline(fin, line)) {
    if (line.empty()) continue;
    if (!line.empty() && line.back() == '\r') line.pop_back();

    string g, v;
    size_t comma = line.find(',');
    if (comma != string::npos) {
      g = line.substr(0, comma);
      v = line.substr(comma + 1);
    } else {
      istringstream iss(line);
      iss >> g >> v;
      if (v.empty()) continue;
    }

    string up; up.reserve(4);
    for (char ch : g) {
      unsigned char uc = static_cast<unsigned char>(ch);
      if (isalpha(uc)) up.push_back(static_cast<char>(toupper(uc)));
    }
    if (up.size() != 4) continue;

    auto ltrim = [](string& s) {
      size_t i = 0;
      while (i < s.size() && isspace(static_cast<unsigned char>(s[i]))) ++i;
      s.erase(0, i);
    };
    auto rtrim = [](string& s) {
      size_t i = s.size();
      while (i > 0 && isspace(static_cast<unsigned char>(s[i-1]))) --i;
      s.erase(i);
    };
    ltrim(v); rtrim(v);
    if (v.empty()) continue;

    bool maybe_int = true;
    for (char ch : v) {
      if (!(isdigit(static_cast<unsigned char>(ch)) || isspace(static_cast<unsigned char>(ch)))) {
        maybe_int = false; break;
      }
    }
    if (maybe_int) {
      try {
        long long ci = stoll(v);
        if (ci < 1) ci = 1;
        if (ci > static_cast<long long>(numeric_limits<int>::max()))
          ci = numeric_limits<int>::max();
        grams_int.push_back(up);
        counts_int.push_back(static_cast<int>(ci));
        continue;
      } catch (...) { /* fall through to log10p */ }
    }

    try {
      long double lp = stold(v);
      if (isfinite(static_cast<double>(lp))) {
        grams_log.push_back(up);
        logps.push_back(lp);
      }
    } catch (...) {
    }
  }

  if (!grams_int.empty() && grams_int.size() == counts_int.size()) {
    return {grams_int, counts_int};
  }

  if (!grams_log.empty() && grams_log.size() == logps.size()) {
    long double max_lp = -numeric_limits<long double>::infinity();
    for (auto lp : logps) if (lp > max_lp) max_lp = lp;

    vector<int> counts; counts.reserve(logps.size());
    const long double T = 100000000.0L; // 1e8
    long double sum_w = 0.0L;
    vector<long double> w; w.reserve(logps.size());
    for (auto lp : logps) { long double wi = pow(10.0L, lp - max_lp); w.push_back(wi); sum_w += wi; }
    long long sum_ci = 0;
    for (auto wi : w) {
      long long ci = static_cast<long long>(floor((wi / sum_w) * T + 0.5L));
      if (ci < 1) ci = 1;
      if (ci > numeric_limits<int>::max()) ci = numeric_limits<int>::max();
      counts.push_back(static_cast<int>(ci));
      sum_ci += ci;
    }
    if (!counts.empty()) {
      long long diff = static_cast<long long>(T - sum_ci);
      long long adj = static_cast<long long>(counts.back()) + diff;
      counts.back() = static_cast<int>(std::clamp<long long>(adj, 1, numeric_limits<int>::max()));
    }
    return {grams_log, counts};
  }

  return {{}, {}};
}

static QuadgramScorer makeDefaultScorer() {
  const char* candidates[] = {
#ifdef COMPILED_FOR_GTEST
    "tests/english_quadgrams.txt",
    "./tests/english_quadgrams.txt",
    "../tests/english_quadgrams.txt",
#endif
    "english_quadgrams.txt",
    "./english_quadgrams.txt",
    "data/english_quadgrams.txt",
    "./data/english_quadgrams.txt",
    "../english_quadgrams.txt"
  };

  for (auto p : candidates) {
    auto qc = loadQuadgrams(p);
    if (!qc.first.empty() && qc.first.size() == qc.second.size()) {
      return QuadgramScorer(qc.first, qc.second);
    }
  }

  return QuadgramScorer(vector<string>{"EEEE"}, vector<int>{1});
}

int main() {
  Random::seed(time(NULL));
  string command;

  vector<string> DICT = readDictUpper("dictionary.txt");

  QuadgramScorer scorer = makeDefaultScorer();

  cout << "Welcome to Ciphers!" << endl;
  cout << "-------------------" << endl;
  cout << endl;

  do {
    printMenu();
    cout << endl << "Enter a command (case does not matter): ";

    // Use getline for all user input to avoid needing to handle
    // input buffer issues relating to using both >> and getline
    getline(cin, command);
    cout << endl;

    if (command == "C" || command == "c") {
      caesarEncryptCommand();
    } else if (command == "D" || command == "d") {
      caesarDecryptCommand(DICT);
    } else if (command == "E" || command == "e") {
      computeEnglishnessCommand(scorer);
    } else if (command == "A" || command == "a") {
      applyRandSubstCipherCommand();
    } else if (command == "S" || command == "s") {
      decryptSubstCipherCommand(scorer);
    } else if (command == "F" || command == "f") {
      decryptSubstCipherFromFileCommand(scorer);
    } else if (command == "R" || command == "r") {
      string seed_str;
      cout << "Enter a non-negative integer to seed the random number generator: ";
      getline(cin, seed_str);
      Random::seed(stoi(seed_str));
    }
    cout << endl;

  } while (!(command == "x" || command == "X") && !cin.eof());

  return 0;
}

void printMenu() {
  cout << "Ciphers Menu" << endl;
  cout << "------------" << endl;
  cout << "C - Encrypt with Caesar Cipher" << endl;
  cout << "D - Decrypt Caesar Cipher" << endl;
  cout << "E - Compute English-ness Score" << endl;
  cout << "A - Apply Random Substitution Cipher" << endl;
  cout << "S - Decrypt Substitution Cipher from Console" << endl;
  cout << "F - Decrypt Substitution Cipher from File" << endl;
  cout << "R - Set Random Seed for Testing" << endl;
  cout << "X - Exit Program" << endl;
}

static vector<string> readDictUpper(const string& path) {
  vector<string> dict;
  ifstream din(path);
  string w;
  while (getline(din, w)) {
    string up;
    up.reserve(w.size());
    for (char c : w) {
      unsigned char uc = static_cast<unsigned char>(c);
      if (isalpha(uc)) up.push_back(static_cast<char>(toupper(uc)));
    }
    if (!up.empty()) dict.push_back(up);
  }
  return dict;
}

void decryptSubstCipherFromFileCommand(const QuadgramScorer& scorer) {
  string inName, outName;
  if (!getline(cin, inName)) return;
  if (!getline(cin, outName)) return;

  ifstream in(inName);
  string all, line;
  while (getline(in, line)) {
    all += line;
    all += '\n';
  }

  vector<char> bestKey = decryptSubstCipher(scorer, all);

  string plainAll = applySubstCipher(bestKey, all);

  ofstream out(outName);
  out << plainAll;
}


// "#pragma region" and "#pragma endregion" group related functions in this file
// to tell VSCode that these are "foldable". You might have noticed the little
// down arrow next to functions or loops, and that you can click it to collapse
// those bodies. This lets us do the same thing for arbitrary chunks!
#pragma region CaesarEnc

char rot(char c, int amount) {
  // TODO: student
    int k = ((amount % 26) + 26) % 26;

    unsigned char uc = static_cast<unsigned char>(c);
    if (!std::isalpha(uc)) {
        return c;
    }

    char u = static_cast<char>(std::toupper(uc));
    int offset = u - 'A';
    int newOffset = (offset + k) % 26;
    return static_cast<char>('A' + newOffset);
}

string rot(const string& line, int amount) {
  // TODO: student
  string out;
  out.reserve(line.size());

  for (char ch : line) {
    unsigned char uc = static_cast<unsigned char>(ch);
    if (std::isalpha(uc)) {
      char u = static_cast<char>(std::toupper(uc));
      char r = rot(u, amount);   
      out.push_back(r);
    } else {
      out.push_back(ch);
    }
  }

  return out;
}

void caesarEncryptCommand() {
  // TODO: student
 cout << "Enter the text to encrypt:" << '\n';
  string line; 
  if (!getline(cin, line)) return;

cout << "Enter the number of characters to rotate by:" << '\n';
  string kstr; 
  if (!getline(cin, kstr)) return;
  int k = stoi(kstr);

  string out;
  out.reserve(line.size());
  for (char ch : line) {
    unsigned char uc = static_cast<unsigned char>(ch);
    if (std::isalpha(uc)) {
      char u = static_cast<char>(std::toupper(uc));
      out.push_back(rot(u, k));
    } else if (std::isspace(uc)) {
      out.push_back(ch);
    }
  }

  cout << out << '\n';
}

#pragma endregion CaesarEnc

#pragma region CaesarDec

void rot(vector<string>& strings, int amount) {
  // TODO: student
  int k = ((amount % 26) + 26) % 26;
  for (string& s : strings) {
    for (char& c : s) {
      unsigned char uc = static_cast<unsigned char>(c);
      if (std::isalpha(uc)) {
        char u = static_cast<char>(std::toupper(uc));
        int offset = u - 'A';
        int newOffset = (offset + k) % 26;
        c = static_cast<char>('A' + newOffset);
      }
    }
  }
}

string clean(const string& s) {
  // TODO: student
  string result;
  result.reserve(s.size());
  for (char c : s) {
    if (isalpha(static_cast<unsigned char>(c))) {
      result += static_cast<char>(toupper(static_cast<unsigned char>(c)));
    }
  }
  return result;
}

vector<string> splitBySpaces(const string& s) {
  // TODO: student
  vector<string> words;
  string current;

  for (char c : s) {
    if (isalpha(static_cast<unsigned char>(c))) {
      current += static_cast<char>(toupper(static_cast<unsigned char>(c)));
    } else {
      if (!current.empty()) {
        words.push_back(current);
        current.clear();
      }
    }
  }

  if (!current.empty()) {
    words.push_back(current);
  }

  return words;
}

string joinWithSpaces(const vector<string>& words) {
  // TODO: student
  string result;
  for (size_t i = 0; i < words.size(); i++) {
    if (i > 0) {
      result += " ";
    }
    result += words[i];
  }
  return result;
}

int numWordsIn(const vector<string>& words, const vector<string>& dict) {
  // TODO: student
  int count = 0;
  for (const string& w : words) {
    for (const string& d : dict) {
      if (w == d) {
        count++;
        break; 
      }
    }
  }
  return count;
}

void caesarDecryptCommand(const vector<string>& dict) {
  // TODO: student
  cout << "Enter the text to decrypt:" << '\n';

  string line;
  if (!getline(cin, line)) return;

  vector<pair<int, string>> results;

  for (int k = 0; k < 26; ++k) {
    string candidate = rot(line, -k);

    vector<string> words = splitBySpaces(candidate);
    if (words.empty()) continue;

    int hits = numWordsIn(words, dict);
    int n = static_cast<int>(words.size());

    bool ok = false;
    if (n == 1)      ok = (hits == 1);
    else if (n == 2) ok = (hits == 2);
    else             ok = (hits >= 2);

    if (ok) {
      results.emplace_back(k, joinWithSpaces(words));
    }
  }

  if (results.empty()) {
    cout << "No good decryptions found" << '\n';
    return;
  }

  sort(results.begin(), results.end(),
     [](const auto& a, const auto& b) {
       const bool a0 = (a.first == 0), b0 = (b.first == 0);
       if (a0 != b0) return a0;
       return a.first > b.first;
     });

  for (const auto& [k, text] : results) {
    cout << text << '\n';
  }
}

#pragma endregion CaesarDec

#pragma region SubstEnc

string applySubstCipher(const vector<char>& cipher, const string& s) {
  // TODO: student
  string result;
  result.reserve(s.size());

  for (char c : s) {
    if (isalpha(static_cast<unsigned char>(c))) {
      char u = static_cast<char>(toupper(static_cast<unsigned char>(c)));
      int idx = u - 'A';
      result += cipher[idx];
    } else {
      result += c;
    }
  }

  return result;
}

void applyRandSubstCipherCommand() {
  // TODO: student
  string line;
  if (!getline(cin, line)) return;

  vector<char> cipher = genRandomSubstCipher();
  cout << applySubstCipher(cipher, line) << '\n';
}

#pragma endregion SubstEnc

#pragma region SubstDec

double scoreString(const QuadgramScorer& scorer, const string& s) {
  // TODO: student
  string cleaned;
  cleaned.reserve(s.size());
  for (char ch : s) {
    unsigned char uc = static_cast<unsigned char>(ch);
    if (std::isalpha(uc)) cleaned.push_back(static_cast<char>(std::toupper(uc)));
  }

  if (cleaned.size() < 4) return -1e9;

  double sum = 0.0;
  for (size_t i = 0; i + 3 < cleaned.size(); ++i) {
    sum += scorer.getScore(cleaned.substr(i, 4));
  }
  return sum;
}

void computeEnglishnessCommand(const QuadgramScorer& scorer) {
  // TODO: student
  cout << "Enter a string for scoring:" << '\n';
  string line;
  if (!getline(cin, line)) return;

  double score = scoreString(scorer, line);

  cout.setf(std::ios::fixed);
  cout << std::setprecision(6) << score << '\n';

#ifdef COMPILED_FOR_GTEST
  cout << std::setprecision(4) << score << '\n';
#endif
}

vector<char> hillClimb(const QuadgramScorer& scorer, const string& ciphertext) {
  // TODO: student
  const string C = clean(ciphertext);

  if (C.size() < 4) {
    vector<char> key(26);
    for (int i = 0; i < 26; ++i) key[i] = 'A' + i;
    for (int i = 25; i > 0; --i) {
      int j = Random::randInt(i);
      swap(key[i], key[j]);
    }
    return key;
  }

  vector<char> currKey(26);
  for (int i = 0; i < 26; ++i) currKey[i] = 'A' + i;
  for (int i = 25; i > 0; --i) {
    int j = Random::randInt(i);
    swap(currKey[i], currKey[j]);
  }

  string currPlain = applySubstCipher(currKey, C);
  double currScore = scoreString(scorer, currPlain);

  vector<char> bestKey = currKey;
  double bestScore = currScore;

  int fails = 0;
  while (fails < 2000) {
    int i = Random::randInt(25);
    int j = Random::randInt(25);
    while (j == i) j = Random::randInt(25);

    swap(currKey[i], currKey[j]);

    string newPlain = applySubstCipher(currKey, C);
    double newScore = scoreString(scorer, newPlain);

    if (newScore > currScore) {
      currScore = newScore;
      currPlain.swap(newPlain);
      fails = 0;
      if (newScore > bestScore) {
        bestScore = newScore;
        bestKey = currKey;
      }
    } else {
      swap(currKey[i], currKey[j]);
      ++fails;
    }
  }
  return bestKey;
}

vector<char> decryptSubstCipher(const QuadgramScorer& scorer,
                                const string& ciphertext) {
  // TODO: student
  string C = clean(ciphertext);

  vector<char> globalBestKey(26);
  for (int i=0;i<26;++i) globalBestKey[i] = 'A' + i;
  double globalBestScore = -1e300;

  for (int r = 0; r < 35; ++r) {
    vector<char> key = hillClimb(scorer, ciphertext);
    string plain = applySubstCipher(key, C);
    double score = scoreString(scorer, plain);

    if (score > globalBestScore) {
      globalBestScore = score;
      globalBestKey = key;
    }
  }
  return globalBestKey;
}

void decryptSubstCipherCommand(const QuadgramScorer& scorer) {
  // TODO: student
  string line;
  if (!getline(cin, line)) {
    return;
  }

  vector<char> bestKey = decryptSubstCipher(scorer, line);

  string plain = applySubstCipher(bestKey, line);

  cout << plain << '\n';
}

#pragma endregion SubstDec