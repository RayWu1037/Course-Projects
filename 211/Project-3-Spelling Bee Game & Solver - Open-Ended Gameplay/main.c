/*
 * CS 211 - Project 3: Spelling Bee Game + Solver
 * Author: Ruiyi Wu
 * Date: 10/18/2025
 * 
 * Description:
 *   Implements a Spelling Bee game and solver. Includes:
 *      - Dynamic WordList
 *      - Dictionary loading & filtering
 *      - Hive building and input validation
 *      - Gameplay scaffold
 *      - Brute-force solver and optimized solver
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

const int MIN_HIVE_SIZE = 2;
const int MAX_HIVE_SIZE = 12;
const int MIN_WORD_LENGTH = 4;

// Dynamic array of char*
typedef struct WordList_struct {
    char** words;
    int numWords;
    int capacity;
} WordList;

// Global pointer so isValidWord() can check dictionary membership
static WordList* gDictionary = NULL;
int findWord(WordList* thisWordList, char* aWord, int loInd, int hiInd);

// allocate and initialize an empty WordList
WordList* createWordList() {
    WordList* newList = malloc(sizeof(WordList));
    newList->capacity = 4;
    newList->numWords = 0;
    newList->words = malloc(newList->capacity * sizeof(char*));

    return newList;
}

// malloc wrapper that exits on failure
static void* xmalloc(size_t n) {
    void* p = malloc(n);
    if (!p) {
        fprintf(stderr, "Out of memory.\n");
        exit(1);
    }
    return p;
}

// strdup implement with xmalloc
static char* xstrdup(const char* s) {
    size_t n = strlen(s);
    char* p = (char*)xmalloc(n + 1);
    memcpy(p, s, n + 1);
    return p;
}

// Append a copy of WordList.
// If capacity is full, double the pointer-array capacity.
// Move only the pointer array, not the strings themselves.
void appendWord(WordList* thisWordList, char* newWord) {
    if (thisWordList->numWords >= thisWordList->capacity){
        int newCap = thisWordList->capacity * 2;
        char** newArr = (char**)xmalloc((size_t)newCap * sizeof(char*));
        for(int i = 0; i < thisWordList->numWords; ++i) {
            newArr[i] = thisWordList->words[i];
        }
        free(thisWordList->words);
        thisWordList->words = newArr;
        thisWordList->capacity = newCap;
    }
    thisWordList->words[thisWordList->numWords++] = xstrdup(newWord);
}

// qsort comparator for char* elements
static int cmpStrPtr(const void* a, const void* b) {
    const char* const* sa = (const char* const*)a;
    const char* const* sb = (const char* const*)b;
    return strcmp(*sa, *sb);
}

/* Load dictionary:
 * - lower-case all lines
 * - keep only alpha [a-z] words of lengh >= minlength 
 * - push into dictionaryList and sort it at the end
 * Return the maximum word length on success; -1 on failure */
int buildDictionary(char* filename, WordList* dictionaryList, int minLength) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return -1;
    }

    char buf[256];
    int maxLen = 0;
    while (fgets(buf, (int)sizeof(buf), fp)) {
        size_t n = strlen(buf);
        // strip trailing newlines
        while (n && (buf[n - 1] == '\n' || buf[n - 1] == '\r')) {
            buf[--n] = '\0';
        }
        if (n == 0) {
            continue;
        }
        // lower-case
        for (size_t i = 0; i < n; ++i) {
            buf[i] = (char)tolower((unsigned char)buf[i]);
        }
        // alpha-only check
        int alpha = 1;
        for (size_t i = 0; i < n; ++i) {
            if (buf[i] < 'a' || buf[i] > 'z') {
                alpha = 0;
                break;
            }
        }
        if (!alpha) {
            continue;
        }
        // length filter
        if ((int)n >= minLength) {
            appendWord(dictionaryList, buf);
            if ((int)n > maxLen) {
                maxLen = (int)n;
            }
        }
    }
    fclose(fp);

    if (dictionaryList->numWords <= 0) {
        return -1;
    }

    // sort dictionary lexicographically
    qsort(dictionaryList->words, (size_t)dictionaryList->numWords, sizeof(char*), cmpStrPtr);

    return maxLen;
}

// free each words[i], then the array, then the struct
void freeWordList(WordList* list) {
    if (!list) return;
    if (list->words) {
        for (int i = 0; i < list->numWords; ++i) {
            free(list->words[i]);
            list->words[i] = NULL;
        }
        free(list->words);
        list->words = NULL;
    }
    free(list);
}

// Find first index of aLet in str; return -1 if not found
int findLetter(char* str, char aLet) {
    for (int i = 0; str[i]; i++) if (str[i] == aLet) {
        return i;
    }
    return -1;
}

// In-place ascending sort for a small letter buffer
static void sortLetters(char* s) {
    size_t n = strlen(s);
    for (size_t i = 0; i + 1 < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            if (s[j] < s[i]) {
                char t = s[i];
                s[i] = s[j];
                s[j] = t;
            }
        }
    }
}

// lower-case, keep only [a-z], deduplicate, then sort
void buildHive(char* str, char* hive) {
    int seen[26] = {0}, k = 0;
    for (int i = 0; str[i]; ++i) {
        char c = (char)tolower((unsigned char)str[i]);
        if (c < 'a' || c > 'z') {
            continue;
        }
        int id = c - 'a';
        if (!seen[id]) {
            seen[id] = 1;
            hive[k++] = c;
        }
    }
    hive[k] = '\0';
    sortLetters(hive);
}

// Count distinct lower-case letters in str
int countUniqueLetters(char* str) {
    int seen[26] = {0}, k = 0;
    for (int i = 0; str[i]; ++i) {
        unsigned char c = (unsigned char)str[i];
        if (c < 'a' || c > 'z') {
            continue;
        }
        if (!seen[c - 'a']) {
            seen[c - 'a'] = 1; 
            ++k;
        }
    }
    return k;
}

// Select words with exactly hiveSize distinct letters
WordList* findAllFitWords(WordList* dictionaryList, int hiveSize) {
    WordList* fitWords = createWordList();
    for (int i = 0; i < dictionaryList->numWords; ++i) {
        char* w = dictionaryList->words[i];
        if (countUniqueLetters(w) == hiveSize) {
            appendWord(fitWords, w);
        }
    }
    return fitWords;
}

/* Validate a word for the current hive:
 * - length >= MIN_HIVE_SIZE
 * - contains required letter
 * - all letters are from hive */
bool isValidWord(char* word, char* hive, char reqLet) {
    if (!word || !hive) {
        return false;
    }

    int L = (int)strlen(word);
    if (L < MIN_WORD_LENGTH) {
        return false;
    }
    if (findLetter(word, reqLet) < 0) {
        return false;
    }

    for (int i = 0; word[i]; ++i) {
        if (findLetter(hive, word[i]) < 0) {
            return false;
        }
    }

    if (gDictionary && gDictionary->numWords > 0) {
        if (findWord(gDictionary, word, 0, gDictionary->numWords - 1) < 0) {
            return false;
        }
    }

    return true;
}

// Check if str uses all letters from hive at least once
bool isPangram(char* str, char* hive) {
    for (int i = 0; hive[i]; ++i) {
        if (findLetter(str, hive[i]) < 0) {
            return false;
        }
    }
    return true;
}

// Pretty-print hive and mark required letter with a caret
void printHive(char* hive, int reqLetInd) {
    printf("  Hive: \"%s\"\n", hive);
    printf("         ");
    for (int i = 0; i < reqLetInd; i++) {
        printf(" ");
    }
    printf("^");
    for (int i = reqLetInd + 1; i < strlen(hive); i++) {
        printf(" ");
    }
    printf(" (all words must include \'%c\')\n\n", hive[reqLetInd]);
}

// per row: marker, score, word
// score: 4-letter=1, else=length; pangram adds +hiveSize
// total score at bottom
void printList(WordList* thisWordList, char* hive) {
    printf("  Word List:\n");
    int totScore = 0;
    int hiveSize = (int)strlen(hive);

    for (int i = 0; i < thisWordList->numWords; ++i) {
        char* w = thisWordList->words[i];
        int len = (int)strlen(w);

        bool pang = isPangram(w, hive);
        bool perf = pang && (len == hiveSize);

        int sc = (len == 4 ? 1 : len);
        if (pang) sc += hiveSize;

        printf("%3s (%2d) %s\n", perf ? "***" : (pang ? "*" : ""), sc, w);

        totScore += sc;
    }

    printf("  Total Score: %d\n", totScore);
}

// Brute-force solver: add every isValidWord to solvedList
void bruteForceSolve(WordList* dictionaryList, WordList* solvedList, char* hive, char reqLet) {
    if (!dictionaryList || !solvedList || !hive) {
        return;
    }
    for (int i = 0; i < dictionaryList->numWords; ++i) {
        char* w = dictionaryList->words[i];
        if (isValidWord(w, hive, reqLet)) {
            appendWord(solvedList, w);
        }
    }
}

// Return true if partWord is a prefix of fullWord
bool isPrefix(char* partWord, char* fullWord) {
    size_t lp = strlen(partWord), lf = strlen(fullWord);
    if (lp > lf) {
        return false;
    }
    return strncmp(partWord, fullWord, lp) == 0;
}

// >=0 : exact match index
//  -1 : aWord is a prefix of some word in range
// -99 : neither a word nor a prefix of any word
int findWord(WordList* thisWordList, char* aWord, int loInd, int hiInd) {
    if (hiInd < loInd) {
        if (loInd < thisWordList->numWords && isPrefix(aWord, thisWordList->words[loInd])) {
            return -1;
        }
        else {
            return -99;
        }
    }

    int mdInd = (hiInd + loInd) / 2;
    int cmp = strcmp(thisWordList->words[mdInd], aWord);

    if (cmp == 0) {
        return mdInd;
    }

    if (cmp < 0) {
        return findWord(thisWordList, aWord, mdInd + 1, hiInd);
    }

    return findWord(thisWordList, aWord, loInd, mdInd - 1);
}

// Buid strings from hive letters; use findWord to guide search.
// Uses dictionary maximum word length to avoid buffer overflow on tryWord.
void findAllMatches(WordList* dictionaryList, WordList* solvedList, char* tryWord, char* hive, char reqLet) {
    int n = (int)strlen(hive);
    if (n == 0) {
        return;
    }

    // compute dictionary maximum word length
    int maxLen = 0;
    for (int i = 0; i < dictionaryList->numWords; ++i) {
        int L = (int)strlen(dictionaryList->words[i]);
        if (L > maxLen) maxLen = L;
    }

    // if empty, star from first hive letter
    if (tryWord[0] == '\0') {
        tryWord[0] = hive[0];
        tryWord[1] = '\0';
    }

    while (tryWord[0] != '\0') {
        int index = findWord(dictionaryList, tryWord, 0, dictionaryList->numWords - 1);

        if (index >= 0) {
            if (isValidWord(tryWord, hive, reqLet)) {
                appendWord(solvedList, tryWord);
            }
            size_t L = strlen(tryWord);
            tryWord[L] = hive[0];
            tryWord[L + 1] = '\0';
            continue;
        }

        if (index == -1) {
            // prefix only
            size_t L = strlen(tryWord);
            tryWord[L] = hive[0];
            tryWord[L + 1] = '\0';
            continue;
        }

        // no match: dial last character or backtrack
        size_t L = strlen(tryWord);
        int advanced = 0;
        while (L > 0 && !advanced) {
            char* pc = &tryWord[L - 1];
            int pos = findLetter(hive, *pc);
            if (pos >= 0 && pos + 1 < n) {
                *pc = hive[pos + 1];
                advanced = 1;
            }
            else {
                tryWord[L - 1] = '\0';
                --L;
            }
        }
        if (!advanced) {
            break;
        }
    }

}

// Provided for you, to determine the program settings based on parameters
bool setSettings(int argc, char* argv[], bool* pRandMode, int* pNumLets, char dictFile[100], bool* pPlayMode, bool* pBruteForceMode, bool* pSeedSelection) {
    *pRandMode = false;
    *pNumLets = 0;
    strcpy(dictFile, "dictionary.txt");
    *pPlayMode = false;
    *pBruteForceMode = true;
    *pSeedSelection = false;
    srand((int)time(0));
    //--------------------------------------
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-r") == 0) {
            ++i;
            if (argc == i) {
                return false;
            }
            *pRandMode = true;
            *pNumLets = atoi(argv[i]);
            if (*pNumLets < MIN_HIVE_SIZE || *pNumLets > MAX_HIVE_SIZE) {
                return false;
            }
        }
        else if (strcmp(argv[i], "-d") == 0) {
            ++i;
            if (argc == i) {
                return false;
            }
            strcpy(dictFile, argv[i]);
            FILE* filePtr = fopen(dictFile, "r");
            if (filePtr == NULL) {
                return false;
            }
            fclose(filePtr);
        }
        else if (strcmp(argv[i], "-s") == 0) {
            ++i;
            if (argc == i) {
                return false;
            }
            *pSeedSelection = true;
            int seed = atoi(argv[i]);
            srand(seed);
        }
        else if (strcmp(argv[i], "-p") == 0) {
            *pPlayMode = true;
        }
        else if (strcmp(argv[i], "-o") == 0) {
            *pBruteForceMode = false;
        }
        else {
            return false;
        }
    }
    return true;
}

// Tiny helpers
void printONorOFF(bool mode) {
    if (mode) {
        printf("ON\n");
    }
    else {
        printf("OFF\n");
    }
}

void printYESorNO(bool mode) {
    if (mode) {
        printf("YES\n");
    }
    else {
        printf("NO\n");
    }
}

// 1) parse args and load dictionary
// 2) random mode or user mode
// 3) optional PLAY mode
// 4) brute-force or optimized; print solution list and totals
// 5) free all dynamic memory
int main(int argc, char* argv[]) {

    printf("\n----- Welcome to the CS 211 Spelling Bee Game & Solver! -----\n\n");

    bool randMode = false;
    int hiveSize = 0;
    char dict[100] = "dictionary.txt";
    bool playMode = false;
    bool bruteForce = true;
    bool seedSelection = false;
    char hive[MAX_HIVE_SIZE + 1] = {};
    hive[0] = '\0';
    int reqLetInd = -1;
    char reqLet = '\0';

    // parse argument
    if (!setSettings(argc, argv, &randMode, &hiveSize, dict, &playMode, &bruteForce, &seedSelection)) {
        printf("Invalid command-line argument(s).\nTerminating program...\n");
        return 1;
    }
    else {
        printf("Program Settings:\n");
        printf("  random mode = ");
        printONorOFF(randMode);
        printf("  play mode = ");
        printONorOFF(playMode);
        printf("  brute force solution = ");
        printONorOFF(bruteForce);
        printf("  dictionary file = %s\n", dict);
        printf("  hive set = ");
        printYESorNO(randMode);
        printf("\n\n");
    }

    // build word array (only words with desired minimum length or longer) from dictionary file
    printf("Building array of words from dictionary... \n");
    WordList* dictionaryList = createWordList();
    int maxWordLength = buildDictionary(dict, dictionaryList, MIN_WORD_LENGTH);
    gDictionary = dictionaryList;
    if (maxWordLength == -1) {
        printf("  ERROR in building word array.\n");
        printf("  File not found or incorrect number of valid words.\n");
        printf("Terminating program...\n");
        return 0;
    }
    printf("   Word array built!\n\n");


    printf("Analyzing dictionary...\n");

    if (dictionaryList->numWords < 0) {
        printf("  Dictionary %s not found...\n", dict);
        printf("Terminating program...\n");
        return 0;
    }

    // end program if file has zero words of minimum desired length or longer
    if (dictionaryList->numWords == 0) {
        printf("  Dictionary %s contains insufficient words of length %d or more...\n", dict, MIN_WORD_LENGTH);
        printf("Terminating program...\n");
        return 0;
    }
    else {
        printf("  Dictionary %s contains \n  %d words of length %d or more;\n", dict, dictionaryList->numWords, MIN_WORD_LENGTH);
    }

    // random or user mode
    if (randMode) {
        printf("==== SET HIVE: RANDOM MODE ====\n");
        //find number of words in words array that use hiveSize unique letters
        WordList* fitWords = findAllFitWords(dictionaryList, hiveSize);
        int numFitWords = fitWords->numWords;
        if (numFitWords == 0) {
            freeWordList(fitWords);
            return 0;
        }
        //pick one at random
        int pickOne = rand() % numFitWords;
        char* chosenFitWord = fitWords->words[pickOne];

        //and alaphabetize the unique letters to make the letter hive
        buildHive(chosenFitWord, hive);
        freeWordList(fitWords);

        reqLetInd = rand() % hiveSize;
        reqLet = hive[reqLetInd];

    }
    else {
        printf("==== SET HIVE: USER MODE ====\n");

        printf("  Enter a single string of lower-case,\n  unique letters for the letter hive... ");

        char input[256];
        while (1) {
            if (scanf("%255s", input) != 1) {
                return 0;
            }
            
            int len = (int)strlen(input);
            if (len < MIN_HIVE_SIZE || len > MAX_HIVE_SIZE) {
                printf("  HIVE ERROR: \"%s\" has invalid length;\n  valid hive size is between %d and %d, inclusive\n\n", input, MIN_HIVE_SIZE, MAX_HIVE_SIZE);
                printf("  Enter a single string of lower-case,\n  unique letters for the letter hive... ");
                continue;
            }
            
            bool alpha = true;
            for (int i = 0; input[i]; ++i) {
                if (input[i] < 'a' || input[i] > 'z') {
                    alpha = false;
                    break;
                }
            }
            if (!alpha) {
                printf("  HIVE ERROR: \"%s\" contains invalid letters;\n  valid characters are lower-case alpha only\n\n", input);
                printf("  Enter a single string of lower-case,\n  unique letters for the letter hive... ");
                continue;
            }
            int seen[26] = {0};
            bool dup = false;
            for (int i = 0; input[i]; ++i) {
                int id = input[i] - 'a';
                if (seen[id]) {
                    dup = true;
                    break;
                }
                seen[id] = 1;
            }
            if (dup) {
                printf("  HIVE ERROR: \"%s\" contains duplicate letters\n\n", input);
                printf("  Enter a single string of lower-case,\n  unique letters for the letter hive... ");
                continue;
            }

            buildHive(input, hive);
            break;
        }    

        hiveSize = (int)strlen(hive);

        reqLetInd = -1;
        reqLet = '\0';

        printf("  Enter the letter from \"%s\"\n  that is required for all words: ", hive);
        char reqBuf[32];
        while(1) {
            if (scanf("%31s", reqBuf) != 1) return 0;

            char c = (char)tolower((unsigned char)reqBuf[0]);
            reqLetInd = findLetter(hive, c);

            if (reqLetInd < 0) {
                printf("  HIVE ERROR: \"%s\" does not contain the character \'%c\'\n\n",hive,c);
                printf("  Enter the letter from \"%s\"\n  that is required for all words: ", hive);
                continue;
            }
            reqLet = c;
            break;
        }
    }

    // print hive and required maeker
    printHive(hive, reqLetInd);

    if (playMode) {
        printf("==== PLAY MODE ====\n");
    //---------------------------------------------------------------------
    //              BEGINNING OF OPEN-ENDED GAMEPLAY SECTION
    //---------------------------------------------------------------------
        char* userWord = (char*)malloc((maxWordLength + 1) * sizeof(char));
        WordList* userWordList = createWordList();

        printf("............................................\n");
        printHive(hive, reqLetInd);

        while (1) {
            printf("  Enter a word (enter DONE to quit): ");
            if (scanf("%s", userWord) != 1) {
                break;
            }
            printf("\n");

            if (strcmp(userWord, "DONE") == 0){
                break;
            }

            for (int i = 0; userWord[i]; ++i) {
                userWord[i] = (char)tolower((unsigned char)userWord[i]);
            }
            
            bool dup = false;
            for (int i = 0; i < userWordList->numWords; ++i) {
                if (strcmp(userWordList->words[i], userWord) == 0) {
                    dup = true;
                    break;
                }
            }
            if (dup) {
                printf("  Already found that word.\n");
            }
            else if (!isValidWord(userWord, hive, reqLet)) {
                printf("  Invalid word (not in dictionary or violates hive rules).\n");
            }
            else {
                appendWord(userWordList, userWord);
                bool pang = isPangram(userWord, hive);
                bool perf = pang && ((int)strlen(userWord) == (int)strlen(hive));
                if (perf) {
                    printf("  *** Perfect pangram! ***\n");
                }
                else if (pang) {
                    printf("  * Pangram! *\n");
                }
                else {
                    printf("  Nice!\n");
                }
            }

            //prints the list and the hive, and gets the next input
            printf("\n");
            printList(userWordList, hive);
            printf("............................................\n");
            printHive(hive, reqLetInd);

        }
        freeWordList(userWordList);
        free(userWord);

    //---------------------------------------------------------------------    
    //                 END OF OPEN-ENDED GAMEPLAY SECTION
    //---------------------------------------------------------------------
    }
        
    // solve
    printf("==== SPELLING BEE SOLVER ====\n");

    printf("  Valid words from hive \"%s\":\n", hive);
    printf("                         ");
    for (int i = 0; i < reqLetInd; i++) {
        printf(" ");
    }
    printf("^\n");

    WordList* solvedList = createWordList();

    if (bruteForce) { //find all words that work... (1) brute force
        bruteForceSolve(dictionaryList, solvedList, hive, reqLet);
    }
    else {
        char* tryWord = (char*)malloc(sizeof(char) * (maxWordLength + 1));

        tryWord[0] = hive[0];
        tryWord[1] = '\0';
        findAllMatches(dictionaryList, solvedList, tryWord, hive, reqLet);
        free(tryWord);

    }

    // compute longest solution length
    int longestSolvedWordLen = 0;
    for (int i = 0; i < solvedList->numWords; i++) {
        if (strlen(solvedList->words[i]) > longestSolvedWordLen) {
            longestSolvedWordLen = strlen(solvedList->words[i]);
        }
    }

    // Helpful variables
    int numValidWords = 0;
    int numPangrams = 0;
    int numPerfectPangrams = 0;
    int totScore = 0;
    int score = 0;
    bool isBingo = true;

    // print solution list + compute totals
    numValidWords = 0;
    numPangrams = 0;
    numPerfectPangrams = 0;
    totScore = 0;
    isBingo = true;

    int hiveLen = (int)strlen(hive);
    for (int i = 0; i < solvedList->numWords; ++i) {
        char* w = solvedList->words[i];
        int len = (int)strlen(w);
        bool pang = isPangram(w, hive);
        bool perf = pang && (len == hiveLen);
        int sc = (len == 4 ? 1 : len) + (pang ? hiveLen : 0);

        printf("%3s (%2d) %s\n", perf ? "***" : (pang ? "*" : ""), sc, w);

        ++numValidWords;
        if (pang) {
            ++numPangrams;
        }
        if (perf) {
            ++numPerfectPangrams;
        }
        totScore += sc;
    }

    for (int i = 0; hive[i]; ++i) {
        bool has = false;
        for (int j = 0; j < solvedList->numWords; ++j) {
            if (solvedList->words[j][0] == hive[i]) {
                has = true;
                break;
            }
        }
        if (!has) {
            isBingo = false;
            break;
        }
    }
    
    // Additional results are printed here:
    printf("\n");
    printf("  Total counts for hive \"%s\":\n", hive);
    printf("                         ");
    for (int i = 0; i < reqLetInd; i++) {
        printf(" ");
    }
    printf("^\n");
    printf("    Number of Valid Words: %d\n", numValidWords);
    printf("    Number of ( * ) Pangrams: %d\n", numPangrams);
    printf("    Number of (***) Perfect Pangrams: %d\n", numPerfectPangrams);
    printf("    Bingo: ");
    printYESorNO(isBingo);
    printf("    Total Score Possible: %d\n", totScore);

    //---------------------------------------------------------------------
    /* TODO Extra credit: Display frequency table
    - Display a table showing the number of words starting with each letter and duration in a table.
    - For example, with the hive abcde and required letter e:
             Frequency Table:
             let  4  5  6  7  8
             -------------------
              a:  3  1  1  1  0
              b:  3  1  4  0  0
              c:  3  1  0  0  1
              d:  3  0  3  0  0
              e:  0  1  0  0  0
       The table shows that there are 3 words that start with a and are 4 letters long, 
       and 1 each that are 5, 6, and 7 letters long and start with a.
       Note that the lengths 4-8 are shown here because the longest word is 8 letters long,
       but that will change depending on the hive
    */
    int hiveSz = (int)strlen(hive);
    int minLen = MIN_WORD_LENGTH;
    int maxLen = longestSolvedWordLen;
    if (maxLen < minLen) {
        maxLen = minLen;
    }

    int cols = maxLen -minLen + 1;

    int **freq = (int **)calloc((size_t)hiveSz, sizeof(int *));
    for (int i = 0; i < hiveSz; ++i) {
        freq[i] = (int *)calloc((size_t)cols, sizeof(int));
    }

    // fill table
    for (int i = 0; i < solvedList->numWords; ++i) {
        char *w = solvedList->words[i];
        int L = (int)strlen(w);
        if (L < minLen || L > maxLen) {
            continue;
        }

        int r = findLetter(hive, w[0]);
        if (r >= 0) {
            int c = L - minLen;
            ++freq[r][c];
        }
    }

    printf("  Frequency Table:\n");
    printf("  let");
    for (int L = minLen; L <= maxLen; ++L) {
        printf("%3d", L);
    }
    printf("\n  ");
    int dashes = 3 + 3 * (cols);
    for (int k = 0; k < dashes; ++k) {
        putchar('-');
    }
    printf("\n");

    for (int r = 0; r < hiveSz; ++r) {
        printf("  %c:", hive[r]);
        for (int c = 0; c < cols; ++c) {
            printf("%3d", freq[r][c]);
        }
        printf("\n");
    }

    // free table
    for (int i = 0; i < hiveSz; ++i) {
        free(freq[i]);
    }
    free(freq);

    // free and exit
    freeWordList(dictionaryList);
    freeWordList(solvedList);
    printf("\n\n");
    return 0;
}
