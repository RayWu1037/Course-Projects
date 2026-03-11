#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

typedef struct WordNode_struct {
    char* myWord;
    struct WordNode_struct* next; 
} WordNode;

typedef struct LadderNode_struct {
    WordNode* topWord;
    struct LadderNode_struct* next; 
} LadderNode;


//------------------- \/\/\/ TOP OF TASK 1 \/\/\/ --------------------
// Opens file and counts number of words with exact length
int countWordsOfLength(char* filename, int wordSize) { 
    //---------------------------------------------------------
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return -1; // cannot open file
    }

    int count = 0;
    char buffer[256]; // enough to hold words of any reasonable size

    // Read each whitespaces-separated token
    while (fscanf(fp, "%255s", buffer) == 1) {
        // If token has exactly wordSize characters
        if ((int)strlen(buffer) == wordSize) {
            count++;
        }
    }

    fclose(fp); 
    //---------------------------------------------------------
    return count;
}

// Opens file and fills 'words' array with valid-length words
bool buildWordArray(char* filename, char** words, int numWords, int wordSize) { 
    //---------------------------------------------------------
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        return false; // cannot open file
    }

    char buffer[256];
    int count = 0;

    // Scan file for valid words
    while (fscanf(fp, "%255s", buffer) == 1) {
        if ((int)strlen(buffer) == wordSize) {
            // store this valid word
            if (count < numWords) {
                words[count] = malloc((wordSize + 1) * sizeof(char));
                if (words[count] == NULL) {
                    fclose(fp);
                    return false; // allocation failure
                }
                strcpy(words[count], buffer);
            }
            count++;
        }
    }

    fclose(fp);

    // Verify that we filled exactly numWords
    if (count != numWords) {
        return false;
    }    
    //---------------------------------------------------------
    return true;
}

// Binary search for aWord in words[loInd ... hiInd]
int findWord(char** words, char* aWord, int loInd, int hiInd) {
    //---------------------------------------------------------
    int mid;
    int cmp;

    while (loInd <= hiInd) {
        mid = (loInd + hiInd) / 2;
        cmp = strcmp(aWord, words[mid]);

        if (cmp == 0) {
            return mid; // found
        }
        else if (cmp < 0) {
            hiInd = mid - 1; // search left half
        }
        else {
            loInd = mid + 1; // search right half
        }
    }
    //---------------------------------------------------------
    return -1;
}

// Free each allocated C-string, then free the array itself
void freeWords(char** words, int numWords) {
    //---------------------------------------------------------
    if (words == NULL) {
        return;
    }

    for (int i = 0; i < numWords; i++) {
        free(words[i]); // safe to free NULL if uninitialized
    }

    free(words); // finally free the array of pointers
    //---------------------------------------------------------
}

//---------------------- ^^^ END OF TASK 1 ^^^ ----------------------


//------------------- \/\/\/ TOP OF TASK 2 \/\/\/ -------------------
// Count total number of character differences between str1 and str2
int strCmpCnt(char* str1, char* str2) {
    //---------------------------------------------------------
    int count = 0;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int maxLen = (len1 > len2) ? len1 : len2;

    for (int i = 0; i < maxLen; i++) {
        char c1 = (i < len1) ? str1[i] : '\0';
        char c2 = (i < len2) ? str2[i] : '\0';
        if (c1 != c2) count++;
    }
    //---------------------------------------------------------
    return count;
}

// Return index of first difference between str1 and str2
int strCmpInd(char* str1, char* str2) {
    //---------------------------------------------------------
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return i;  // first differing index
        }
        i++; 
    }

    // if one string ends before the other
    if (str1[i] != str2[i]) {
        return i; 
    }
    //---------------------------------------------------------
    return -1; 
}

//---------------------- ^^^ END OF TASK 2 ^^^ ----------------------


//------------------- \/\/\/ TOP OF TASK 3 \/\/\/ -------------------
// Create new WordNode and insert it at the front of ladder
void insertWordAtFront(WordNode** ladder, char* newWord) {
    //---------------------------------------------------------
    WordNode* newNode = malloc(sizeof(WordNode));
    if (newNode == NULL) {
        fprintf(stderr, "memory allocation failed in insertWordAtFront\n");
        exit(1);
    }

    newNode->myWord = newWord; // point to existing word (no copy)
    newNode->next = *ladder; // link old head after new node
    *ladder = newNode; // update head pointer
    //---------------------------------------------------------
}

// Count how many WordNodes are in the ladder
int getLadderHeight(WordNode* ladder) {
    //---------------------------------------------------------
    int height = 0;
    WordNode* curr = ladder;

    while (curr != NULL) {
        height++;
        curr = curr->next;
    }
    //---------------------------------------------------------
    return height;
}

// Validate user's input word during game play
bool checkForValidWord(char** words, int numWords, int wordSize, WordNode* ladder, char* aWord) {
    //---------------------------------------------------------
    // 1) Special case: user entered "DONE"
    if (strcmp(aWord, "DONE") == 0) {
        printf("Stopping with an incomplete word ladder...\n");
        return true;
    }

    // 2) Check word length
    if ((int)strlen(aWord) != wordSize) {
        printf("Entered word does NOT have the correct length. Try again...\n");
        return false;
    }

    // 3) Check if word exists in dictionary using binary search
    int idx = findWord(words, aWord, 0, numWords - 1);
    if (idx == -1) {
        printf("Entered word NOT in dictionary. Try again...\n");
        return false;
    }

    // 4) Check single-character difference from previous word
    // ladder->myWord is the most recent word in ladder
    if (ladder == NULL) {
        // Ladder empty, only valid when this is the first word
        printf("Entered word is valid and will be added to the word ladder.\n");
        return true;
    }

    int diff = strCmpCnt(aWord, ladder->myWord);
    if (diff != 1) {
        printf("Entered word is NOT a one-character change from the previous word. Try again...\n");
        return false;
    }
    //---------------------------------------------------------
    // 5) If passed all checks
    printf("Entered word is valid and will be added to the word ladder.\n");
    return true;
}

bool isLadderComplete(WordNode* ladder, char* finalWord) {
    //---------------------------------------------------------
    if (ladder == NULL) {
        return false; // empty ladder can't be complete
    }
    //---------------------------------------------------------
    return (strcmp(ladder->myWord, finalWord) == 0);
}

WordNode* copyLadder(WordNode* ladder) {
    //---------------------------------------------------------
    if (ladder == NULL) {
        return NULL;
    }

    WordNode* newHead = NULL;
    WordNode* newTail = NULL;
    WordNode* curr = ladder;

    while (curr != NULL) {
        WordNode* newNode = malloc(sizeof(WordNode));
        if (newNode == NULL) {
            fprintf(stderr, "Memory allocation failed in copyLadder\n");
            exit(1);
        }

        newNode->myWord = curr->myWord; // reuse existing string pointer
        newNode->next = NULL;

        if (newHead == NULL) {
            newHead = newTail = newNode;
        }
        else {
            newTail->next = newNode;
            newTail = newNode;
        }

        curr = curr->next;
    }
    //---------------------------------------------------------
    return newHead;
}

void freeLadder(WordNode* ladder) {
    //---------------------------------------------------------
    WordNode* curr = ladder;
    while (curr != NULL) {
        WordNode* temp = curr;
        curr = curr->next;
        free(temp);
    }
    //---------------------------------------------------------
}

//---------------------- ^^^ END OF TASK 3 ^^^ ----------------------


//------------------- \/\/\/ TOP OF TASK 4 \/\/\/ -------------------

void displayIncompleteLadder(WordNode* ladder) {
    //-------------------------------------------------------------------
// displayIncompleteLadder() should display the C-strings in the  
//      [ladderWords] array with the first word at the bottom, and 
//      each successive C-string one rung higher on the ladder. 
//      The ladder [height] is the number of words it contains.  
//      To signify the ladder as incomplete, display three lines of  
//      "..." at the top of the ladder. The ladder must be displayed 
//      with an indentation of two whitespaces on every line;
//      Ex: if the start word is "data" and final word is "code" and
//          the incomplete ladder is data->date->gate->gave, then the
//          output display should be as follows (where the quotation  
//          marks are NOT a part of the actual display):
//              "  ..."
//              "  ..."
//              "  ..."
//              "  gave"
//              "  gate"
//              "  date"
//              "  data" 
//-------------------------------------------------------------------
    //---------------------------------------------------------
    if (ladder == NULL) {
        printf("  ...\n  ...\n  ...\n");
        return;
    }

    // First, print the top "..." three lines
    printf("  ...\n");
    printf("  ...\n");
    printf("  ...\n");

    // To print from bottom to top, we can store the pointers first
    WordNode* curr = ladder;
    while (curr != NULL) {
        printf("  %s\n", curr->myWord);
        curr = curr->next;
    }
    //---------------------------------------------------------
}

void displayCompleteLadder(WordNode* ladder) {
//-------------------------------------------------------------------
// displayCompleteLadder(), should display the C-strings in the  
//      [ladderWords] array with the first word at the bottom, and  
//      each successive C-string one rung higher on the ladder. 
//      The ladder [height] is the number of words it contains.  
//      In between each ladder rung, display the symbol '^' to 
//      signify the character that changes between the two rungs of 
//      the ladder. The ladder should be displayed with an indentation 
//      of two whitespaces to the left of every word;
//
//      HINT: call strCmpInd() here
//
//      Ex: if the start word is "data" and final word is "code" 
//          then the output display for a complete ladder should be  
//          as follows (where the quotation marks are NOT a part of  
//          the actual display):
//              "  code"
//              "    ^ "
//              "  cove"
//              "   ^  "
//              "  cave"
//              "  ^   "
//              "  gave"
//              "    ^ "
//              "  gate"
//              "  ^   "
//              "  date"
//              "     ^"
//              "  data" 
//-------------------------------------------------------------------
    //---------------------------------------------------------
    if (ladder == NULL) {
        return;
    }

    // Store nodes in array to easily print bottom to top
    WordNode* stack[1000];
    int count = 0;
    WordNode* curr = ladder;
    while (curr != NULL && count < 1000) {
        stack[count++] = curr;
        curr = curr->next;
    }

    // Print from top (head) to bottom (tail)
    for (int i = 0; i < count; i++) {
        printf("  %s\n", stack[i]->myWord);

        // Print caret '^' line between words (except last)
        if (i < count - 1) {
            int diffIndex = strCmpInd(stack[i]->myWord, stack[i + 1]->myWord);

            // diffIndex spaces after initial "  "
            printf("  ");
            for (int j = 0; j < diffIndex; j++) {
                printf(" ");
            }
            printf("^");

            int len = strlen(stack[i]->myWord);
            for (int j = diffIndex + 1; j < len; j++) {
                printf(" ");
            }
            printf("\n");
        }
    }
    //---------------------------------------------------------
}

//---------------------- ^^^ END OF TASK 4 ^^^ ----------------------


//------------------- \/\/\/ TOP OF TASK 5 \/\/\/ -------------------

void insertLadderAtBack(LadderNode** list, WordNode* newLadder) {
    //---------------------------------------------------------
    LadderNode* newNode = malloc(sizeof(LadderNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed in insertLadderAtBack\n");
        exit(1);
    }

    newNode->topWord = newLadder;
    newNode->next = NULL;

    // if list is empty to new node becomes head
    if (*list == NULL) {
        *list = newNode;
        return;
    }

    // otherwise traverse to the end
    LadderNode* curr = *list;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = newNode;
    //---------------------------------------------------------
}

WordNode* popLadderFromFront(LadderNode** list) {
    //---------------------------------------------------------
    if (list == NULL || *list == NULL) {
        return NULL;
    }

    LadderNode* front = *list;
    WordNode* ladder = front->topWord;

    // move head pointer to next
    *list = front->next;

    // free the popped LadderNode only
    free(front);
    //---------------------------------------------------------
    return ladder;
}

void freeLadderList(LadderNode* myList) {
    //---------------------------------------------------------
    LadderNode* curr = myList;
    while (curr != NULL) {
        LadderNode* temp = curr;
        curr = curr->next;

        // free the LadderNode
        free(temp);
    }
    //---------------------------------------------------------
}

//---------------------- ^^^ END OF TASK 5 ^^^ ----------------------


//------------------- \/\/\/ TOP OF TASK 6 \/\/\/ -------------------

WordNode* findShortestWordLadder(   char** words, 
                                    bool* usedWord, 
                                    int numWords, 
                                    int wordSize, 
                                    char* startWord, 
                                    char* finalWord ) {
    //---------------------------------------------------------
    // Initialize the list of ladders (queue)
    LadderNode* myList = NULL;

    // Create the first ladder containing only the startWord
    WordNode* startLadder = NULL;
    int startInd = findWord(words, startWord, 0, numWords - 1);
    if (startInd == -1) {
        return NULL; // startWord not in dictionary
    }

    insertWordAtFront(&startLadder, words[startInd]);
    usedWord[startInd] = true;
    insertLadderAtBack(&myList, startLadder);

    // BFS loop
    while (myList != NULL) {
        // pop front ladder
        WordNode* currLadder = popLadderFromFront(&myList);
        char* currWord = currLadder->myWord;

        // Generate all possible neighbors
        for (int i = 0; i < wordSize; i++) {
            char temp[wordSize + 1];
            strcpy(temp, currWord);

            for (char c = 'a'; c <= 'z'; c++) {
                if (temp[i] == c) {
                    continue; // skip identical
                }
                temp[i] = c;

                int ind = findWord(words, temp, 0, numWords - 1);
                if (ind == -1 || usedWord[ind]) {
                    continue; // not in dictionary
                }

                // if we found the final word, done
                if (strcmp(temp, finalWord) == 0) {
                    insertWordAtFront(&currLadder, words[ind]);

                    // free any remaining ladders in queue
                    while (myList != NULL) {
                        WordNode* toFree = popLadderFromFront(&myList);
                        freeLadder(toFree);
                    }
                    return currLadder;
                }

                // Otherwise create a new ladder
                WordNode* newLadder = copyLadder(currLadder);
                insertWordAtFront(&newLadder, words[ind]);
                insertLadderAtBack(&myList, newLadder);
                usedWord[ind] = true;
            }
        }

        // Done exploring this ladder, free it
        freeLadder(currLadder);
    }
    //---------------------------------------------------------
    while (myList != NULL) {
        WordNode* toFree = popLadderFromFront(&myList);
        freeLadder(toFree);
    }
    return NULL;
}

//---------------------- ^^^ END OF TASK 5 ^^^ ----------------------


//------------------- \/\/\/ TOP OF OTHERS \/\/\/ -------------------


// randomly set a word from the dictionary word array
void setWordRand(char** words, int numWords, int wordSize, char* aWord) {
    printf("  Picking a random word for you...\n");
    strcpy(aWord,words[rand()%numWords]);
    printf("  Your word is: %s\n",aWord);
}

// interactive user-input to set a word;
//  ensures the word is in the dictionary word array
void setWord(char** words, int numWords, int wordSize, char* aWord) {
    bool valid = false;
    if (strcmp(aWord,"RAND") != 0) printf("  Enter a %d-letter word (enter RAND for a random word): ", wordSize);
    int count = 0;
    while (!valid) {
        if (strcmp(aWord,"RAND") != 0) scanf("%s",aWord);
        count++;
        valid = (strlen(aWord) == wordSize);
        if (valid) {
            int wordInd = findWord(words, aWord, 0, numWords-1);
            if (wordInd < 0) {
                valid = false;
                printf("    Entered word %s is not in the dictionary.\n",aWord);
                printf("  Enter a %d-letter word (enter RAND for a random word): ", wordSize);
            }
        } else if (strcmp(aWord,"RAND") != 0) {
            printf("    Entered word %s is not a valid %d-letter word.\n",aWord,wordSize);
            printf("  Enter a %d-letter word (enter RAND for a random word): ", wordSize);
        }
        if (!valid && (count >= 5 || strcmp(aWord,"RAND") == 0)) { //too many tries, picking random word
            setWordRand(words, numWords, wordSize, aWord);
            valid = true;
        }
    }
}

// helpful debugging function to print a single Ladder
void printLadder(WordNode* ladder) {
    WordNode* currNode = ladder;
    while (currNode != NULL) {
        printf("\t\t\t%s\n",currNode->myWord);
        currNode = currNode->next;
    }
}

// helpful debugging function to print the entire list of Ladders
void printList(LadderNode* list) {
    printf("\n");
    printf("Printing the full list of ladders:\n");
    LadderNode* currList = list;
    while (currList != NULL) {
        printf("  Printing a ladder:\n");
        printLadder(currList->topWord);
        currList = currList->next;
    }
    printf("\n");
}

//---------------------- ^^^ END OF OTHERS ^^^ ----------------------



//-----------------------------------------------------
// The primary application is mostly fully-develop as
//  provided in main(); changes in main() should be
//  limited to updates made for the game play task(s)
//  and testing-related purposes (such as command-line
//  arguments for "TESTING MODE" to call a test case 
//  master function, or something similar)
//-----------------------------------------------------
int main(int argc, char* argv[]) {

    printf("\n");
    printf("--------------------------------------------\n");
    printf("Welcome to the CS 211 Word Ladder Generator!\n");
    printf("--------------------------------------------\n\n");
    

    //-------------- \/\/\/ TOP OF PROGRAM SETTINGS \/\/\/ --------------
    //--- COMMAND-LINE ARGUMENTS AND/OR INTERACTIVE USER-INPUT \/\/\/ ---

    
    // default values for program parameters that may be set with
    //  command-line arguments
    int wordSize = -2114430;
    char dict[100] = "notAfile";
    char startWord[30] = "notAword";
    char finalWord[30] = "notValid";
    bool playMode = false;
    
    printf("\nProcessing command-line arguments...\n");

    //-------------------------------------------------------------------
    // command-line arguments:
    //  [-n wordLen] = sets word length for word ladder;
    //                 if wordLen is not a valid input
    //                 (cannot be less than 2 or greater than 20),
    //                 or missing from command-line arguments,
    //                 then let user set it using interactive user input
    // [-d dictFile] = sets dictionary file;
    //                 if dictFile is invalid (file not found) or
    //                 missing from command-line arguments, then let
    //                 user set it using interactive user input
    // [-s startWord] = sets the starting word;
    //                  if startWord is invalid
    //                  (not in dictionary or incorrect length) or
    //                  missing from command-line arguments, then let
    //                  user set it using interactive user input
    // [-f finalWord] = sets the final word;
    //                  if finalWord is invalid
    //                  (not in dictionary or incorrect length) or
    //                  missing from command-line arguments, then let
    //                  user set it using interactive user input
    // [-p playModeSwitch] = turns playMode ON if playModeSwitch is "ON"
    //                       or leaves playMode OFF if playModeSwitch is
    //                       anything else, including "OFF"
    //-------------------------------------------------------------------

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i],"-n") == 0 && argc > i+1) {
            wordSize = atoi(argv[i+1]);
            ++i;
        } else if (strcmp(argv[i],"-d") == 0 && argc > i+1) {
            strcpy(dict, argv[i+1]);
            ++i;
        } else if (strcmp(argv[i],"-s") == 0 && argc > i+1) {
            strcpy(startWord, argv[i+1]);
            ++i;
        } else if (strcmp(argv[i],"-f") == 0 && argc > i+1) {
            strcpy(finalWord, argv[i+1]);
            ++i;
        } else if (strcmp(argv[i],"-p") == 0 && argc > i+1) {
            playMode = (strcmp(argv[i+1],"ON") == 0);
            ++i;
        }
    }
    
    srand((int)time(0));
    
    // set word length using interactive user-input
    //  if wordSize == -2114430, it was NOT set with command-line args
    while (wordSize < 2 || wordSize > 20) {
        if (wordSize != -2114430) printf("Invalid word size for the ladder: %d\n", wordSize);
        printf("Enter the word size for the ladder: ");
        scanf("%d",&wordSize);
        printf("\n");
    }

    printf("This program is a word ladder building game and a solver that\n");
    printf("finds the shortest possible ");
    printf("word ladder between two %d-letter words.\n\n",wordSize);
    
    // interactive user-input to set the dictionary file;
    //  check that file exists; if not, user enters another file
    //  if file exists, count #words of desired length [wordSize];
    //  if dict == "notAfile", it was NOT set with command-line args
    int numWords = countWordsOfLength(dict,wordSize);
    while (numWords < 0) {
        if (strcmp(dict, "notAfile") != 0) {
            printf("  Dictionary %s not found...\n",dict);
        }
        printf("Enter filename for dictionary: ");
        scanf("%s", dict);
        printf("\n");
        numWords = countWordsOfLength(dict,wordSize);
    }
    
    // end program if file does not have at least two words of desired length
    if (numWords < 2) {
        printf("  Dictionary %s contains insufficient %d-letter words...\n",dict,wordSize);
        printf("Terminating program...\n");
        return -1;
    }
    
    // allocate heap memory for the word array; only words with desired length
    char** words = (char**)malloc(numWords*sizeof(char*));
    
    // [usedWord] bool array has same size as word array [words];
    //  all elements initialized to [false];
    //  later, usedWord[i] will be set to [true] whenever
    //      words[i] is added to ANY partial word ladder;
    //      before adding words[i] to another word ladder,
    //      check for previous usage with usedWord[i]
    bool* usedWord = (bool*)malloc(numWords*sizeof(bool));
    for (int i = 0; i < numWords; ++i) {
        usedWord[i] = false;
    }
    
    // build word array (only words with desired length) from dictionary file
    printf("Building array of %d-letter words... ", wordSize);
    bool status = buildWordArray(dict,words,numWords,wordSize);
    if (!status) {
        printf("  ERROR in building word array.\n");
        printf("  File not found or incorrect number of %d-letter words.\n",wordSize);
        printf("Terminating program...\n");
        return -1;
    }
    printf("Done!\n\n");

    // set the two ends of the word ladder using interactive user-input
    //  make sure start and final words are in the word array,
    //  have the correct length (implicit by checking word array), AND
    //  that the two words are not the same
    // start/final words may have already been set using command-line arguments
    // the start/final word can also be set to "RAND" resulting in a random
    //  assignment from any element of the words array
    if (strcmp(startWord,"RAND")==0) {
        printf("Setting the start word randomly...\n");
        setWordRand(words, numWords, wordSize, startWord);
    } else if (findWord(words, startWord,0, numWords-1) < 0 || strlen(startWord) != wordSize) {
        if (strcmp(startWord,"notAword")==0) {
            printf("Setting the start %d-letter word... \n", wordSize);
        } else {
            printf("Invalid start word %s. Resetting the start %d-letter word... \n", startWord, wordSize);
        }
        setWord(words, numWords, wordSize, startWord);
    }
    printf("\n");
    
    if (strcmp(finalWord,"RAND")==0) {
        printf("Setting the final word randomly...\n");
        setWordRand(words, numWords, wordSize, finalWord);
    } else if (findWord(words, finalWord,0, numWords-1) < 0 || strlen(finalWord) != wordSize) {
        if (strcmp(finalWord,"notValid")==0) {
            printf("Setting the final %d-letter word... \n", wordSize);
        } else {
            printf("Invalid final word %s. Resetting the final %d-letter word... \n", finalWord, wordSize);
        }
        setWord(words, numWords, wordSize, finalWord);
    }
    while (strcmp(finalWord,startWord) == 0) {
        printf("  The final word cannot be the same as the start word (%s).\n",startWord);
        printf("Setting the final %d-letter word... \n", wordSize);
        setWord(words, numWords, wordSize, finalWord);
    }
    printf("\n");
    
    //----------------- ^^^ END OF PROGRAM SETTINGS ^^^ -----------------
    
    
    //-------------- \/\/\/ TOP OF GAME PLAY SECTION \/\/\/ --------------
    
    if (!playMode) {
        printf("\n");
        printf("---------------------------------------------\n");
        printf("No Word Ladder Builder Game; Play Mode is OFF\n");
        printf("---------------------------------------------\n");
        printf("\n");
    } else {
        printf("\n");
        printf("-----------------------------------------------\n");
        printf("Welcome to the CS 211 Word Ladder Builder Game!\n");
        printf("-----------------------------------------------\n");
        printf("\n");

        printf("Your goal is to make a word ladder between two ");
        printf("%d-letter words: \n  %s -> %s\n\n",wordSize, startWord,finalWord);
        
        WordNode* userLadder = NULL;
        int ladderHeight = 0; // initially, the ladder is empty
        int startInd = findWord(words, startWord, 0, numWords-1);
        insertWordAtFront(&userLadder, words[startInd]);
        ladderHeight++; // Now, the ladder has a start word
            
        char aWord[30] = "XYZ";
        printf("\n");
        
        // Let the user build a word ladder interactively & iteratively.
        // First, check that ladder is not too long AND not complete.
        while (strcmp(aWord, "DONE") != 0 && !isLadderComplete(userLadder, finalWord)) {   // modify this line
            printf("The goal is to reach the final word: %s\n",finalWord);
            printf("The ladder is currently: \n");
            displayIncompleteLadder(userLadder);
            printf("Current ladder height: %d\n",ladderHeight);
            printf("Enter the next word (or DONE to stop): ");
            scanf("%s",aWord);
            printf("\n");
            
            // Make sure the entered word is valid for the next ladder rung;
            // if not, repeatedly allow user to enter another word until one is valid
            while (!checkForValidWord(words, numWords, wordSize, userLadder, aWord)) {
                printf("Enter another word (or DONE to stop): ");
                scanf("%s",aWord);
                printf("\n");
            }

            // add the entered word to the ladder (unless it is "DONE")
            if (strcmp(aWord,"DONE") != 0) {
                int currInd = findWord(words, aWord, 0, numWords-1);
                insertWordAtFront(&userLadder, words[currInd]);
                ladderHeight++;
            }
            printf("\n");
        }

        // Check if the built word ladder is complete and
        // display the word ladder appropriately.
        if (isLadderComplete(userLadder, finalWord)) {
            printf("Word Ladder complete!\n\n");
            displayCompleteLadder(userLadder);
            printf("\nWord Ladder height = %d\n\n", ladderHeight);
            printf("Can you find a shorter Word Ladder next time??? \n");
        } else {
            printf("The final Word Ladder is incomplete:\n");
            displayIncompleteLadder(userLadder);
            printf("Word Ladder height = %d\n\n", ladderHeight);
            printf("Can you complete the Word Ladder next time??? \n");
        }
        freeLadder(userLadder);
    }
    
    //----------------- ^^^ END OF GAME PLAY SECTION ^^^ -----------------
    
    
    //-------------- \/\/\/ TOP OF WORD LADDER SOLVER \/\/\/ --------------
    
    printf("\n\n");
    printf("-----------------------------------------\n");
    printf("Welcome to the CS 211 Word Ladder Solver!\n");
    printf("-----------------------------------------\n");
    printf("\n");
    

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // OPTIONAL EXTENTION TO FIND LONGEST WORD LADDER:
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    
    // run the algorithm to find the shortest word ladder
    WordNode* myLadder = findShortestWordLadder(words, usedWord, numWords, wordSize, startWord, finalWord);

    // display word ladder and its height if one was found
    if (myLadder == NULL) {
        printf("There is no possible word ladder from %s to %s\n",startWord,finalWord);
    } else {
        printf("Shortest Word Ladder found!\n\n");
        displayCompleteLadder(myLadder);
        //printLadder(myLadder);
    }
    printf("\nWord Ladder height = %d\n\n",getLadderHeight(myLadder));

    //----------------- ^^^ END OF WORD LADDER SOLVER ^^^ -----------------
    
    
    //-------------- \/\/\/ TOP OF CLEAN-UP \/\/\/ --------------
        
    // free the heap-allocated memory for the shortest ladder
    freeLadder(myLadder);
    // free the heap-allocated memory for the words array
    freeWords(words,numWords);
    free(usedWord);
    
    //----------------- ^^^ END OF CLEAN-UP ^^^ -----------------

    
    return 0;
}
