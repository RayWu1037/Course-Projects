/*
 * CS 211 - Project 02: Food Web (Dynamic Memory)
 * File: main.c
 * Author: <Ruiyi Wu>
 * Date: <2025-09-28>
 *
 * Summary: Builds a food web, allows adding and removing species and relations,
 *          and prints analyses.
 * Notes:   No realloc - grow and shrink with malloc + copy + free. Name Length <= 19.
 *          
 * Readme:  Implemented mutation tasks first (3,4,7), then modes & cleanup(2,5), then 
 *          analysis and printing(6), and finally added comments(Task 1).
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// Data type: an organism with a short name and a dynamic prey list
typedef struct Org_struct {
    char name[20];
    int* prey; //dynamic array of indices  
    int numPrey;
} Org;

// return nonzero iff a == b ignoring case.
static int equalsIgnoreCase(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

// 1..19 chars, not "DONE", alpha first, then [A-Za-z0-9_-].
static int isMeaningfulName(const char* s) {
    if (!s) {
        return 0;
    }
    size_t len = strlen(s);
    if (len == 0 || len >= 20) {
        return 0;
    }
    if (equalsIgnoreCase(s, "DONE")) {
        return 0;
    }
    if (!isalpha((unsigned char)s[0])) {
        return 0;
    }
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '_' || c == '-')) {
            return 0;
        }
    }
    return 1;
    
}

// case-insensitive membership test in web[0..n).
static int nameExists(const Org* web, int n, const char* s) {
    for (int i = 0; i < n; ++i) {
        if (equalsIgnoreCase(web[i].name, s)) {
            return 1;
        }
    }
    return 0;
}

// nonzero iff targetPrey already in web[predator].prey.
static int appearsInPrey(const Org* web, int numOrgs, int predator, int targetPrey) {
    for (int k = 0; k < web[predator].numPrey; ++k) {
        if (web[predator].prey[k] == targetPrey) {
            return 1;
        }
    }
    return 0;
}

// append a validated, unique organism to *pWeb.
void addOrgToWeb(Org** pWeb, int* pNumOrgs, char* newOrgName) {
    if (!isMeaningfulName(newOrgName)) {
        printf("Invalid organism name. No organism added to the food web.\n");
        return;
    }
    if (nameExists(*pWeb, *pNumOrgs, newOrgName)) {
        printf("Duplicate organism name. No organism added to the food web.\n");
        return;
    }

    int oldN = *pNumOrgs;
    Org* oldArr = *pWeb;
   
    Org* newArr = (Org*)malloc(sizeof(Org) * (oldN + 1)); 
    if (!newArr) {
        return;
    }

    for (int i = 0; i < oldN; ++i) {
        newArr[i] = oldArr[i];
    }

    strcpy(newArr[oldN].name, newOrgName); 
    newArr[oldN].prey = NULL;
    newArr[oldN].numPrey = 0;

    if (oldArr) {
        free(oldArr);
    }
    *pWeb = newArr;
    *pNumOrgs = oldN + 1;
}

// add preyInd to predator's prey[] if valid & not duplicate.
bool addRelationToWeb(Org* web, int numOrgs, int predInd, int preyInd) {
    if (predInd < 0 || predInd >= numOrgs || preyInd < 0 || preyInd >= numOrgs || predInd == preyInd) {
        printf("Invalid predator and/or prey index. No relation added to the food web.\n");
        return false;
    }
    if (appearsInPrey(web, numOrgs, predInd, preyInd)) {
        printf("Duplicate predator/prey relation. No relation added to the food web.\n");
        return false;
    }
        
    int oldM = web[predInd].numPrey;
    int* oldPrey = web[predInd].prey;

    int* newPrey = (int*)malloc(sizeof(int) * (oldM + 1));
    if (!newPrey) {
        return false;
    }

    for (int i = 0; i < oldM; ++i) {
        newPrey[i] = oldPrey[i];
    }
    newPrey[oldM] = preyInd;

    if (oldPrey) {
        free(oldPrey);
    }
    web[predInd].prey = newPrey;
    web[predInd].numPrey = oldM + 1;
    return true;
}

// remove organism at index; scrub references; shrink arrays.
bool removeOrgFromWeb(Org** pWeb, int* pNumOrgs, int index) {
    int n = *pNumOrgs;
    Org* arr = *pWeb;

    if (index < 0 || index >= n) {
        printf("Invalid extinction index. No organism removed from the food web.\n");
        return false;
    }

    for (int i = 0; i < n; ++i) {
        if (arr[i].numPrey == 0) {
            continue;
        }

        int keep = 0;
        for (int j = 0; j < arr[i].numPrey; ++j) {
            int v = arr[i].prey[j];
            if (v == index) {

            } 
            else {
                if (v > index) v -= 1;
                arr[i].prey[keep++] = v;
            }
        }

        if (keep == 0) {
            free(arr[i].prey);
            arr[i].prey = NULL;
            arr[i].numPrey = 0;
        } 
        else if (keep < arr[i].numPrey) {
            int* smaller = (int*)malloc(sizeof(int) * keep);
            if (smaller) {
                for (int t = 0; t < keep; ++t) smaller[t] = arr[i].prey[t];
                free(arr[i].prey);
                arr[i].prey = smaller;
            }
            arr[i].numPrey = keep;
        }
    }

    if (arr[index].prey) {
        free(arr[index].prey);
        arr[index].prey = NULL;
        arr[index].numPrey = 0;
    }

    if (n - 1 == 0) {
        free(arr);
        *pWeb = NULL;
        *pNumOrgs = 0;
        return true;
    }

    Org* newArr = (Org*)malloc(sizeof(Org) * (n - 1));
    if (!newArr) {
        return false;
    }

    int t = 0;
    for (int i = 0; i < n; ++i) {
        if (i == index) {
            continue;
        }
        newArr[t++] = arr[i];
    }
    free(arr);
    *pWeb = newArr;
    *pNumOrgs = n - 1;
    return true;
}

// free every prey[] then free the web array.
void freeWeb(Org* web, int numOrgs) {
    if (!web) {
        return;
    }
    for (int i = 0; i < numOrgs; ++i) {
        if (web[i].prey) free(web[i].prey);
        web[i].prey = NULL;
        web[i].numPrey = 0;
    }
    free(web);
}

// list each organism and what it eats.
void printWeb(Org* web, int numOrgs) {
    for (int i = 0; i < numOrgs; ++i) {
        if (web[i].numPrey == 0) {
            printf("  (%d) %s\n", i, web[i].name);
        } 
        else {
            printf("  (%d) %s eats ", i, web[i].name);
            for (int j = 0; j < web[i].numPrey; ++j) {
                int p = web[i].prey[j];
                printf("%s", web[p].name);
                if (j + 1 < web[i].numPrey) printf(", ");
            }
            printf("\n");
        }
    }
}

// true iff no organism lists idx as prey.
static bool isApex(const Org* web, int n, int idx) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < web[i].numPrey; ++j) {
            if (web[i].prey[j] == idx) {
                return false;
            }
        }
    }
    return true;
}

// longest chain length from any producer to u.
static int heightDFS(int u, const Org* web, int n, int* memo, int* vis) {
    if (memo[u] != -1) {
        return memo[u];
    }
    if (web[u].numPrey == 0) {
        memo[u] = 0;
        return 0;
    }
    int best = 0;
    for (int i = 0; i < web[u].numPrey; ++i) {
        int v = web[u].prey[i];
        if (vis[v]) {
            continue;
        }
        vis[v] = 1;
        int got = 1 + heightDFS(v, web, n, memo, vis);
        vis[v] = 0;
        if (got > best) {
            best = got;
        }
    }
    memo[u] = best;
    return best;
}

// print the analysis report; each name on its own line with 2-space indent.
void displayAll(Org* web, int numOrgs, bool modified) {
    // Overview
    if (modified) printf("UPDATED ");
    printf("Food Web Predators & Prey:\n");
    printWeb(web,numOrgs); 
    printf("\n");

    // one name per line, two-space indent.
    if (modified) printf("UPDATED ");
    printf("Apex Predators:\n");
    for (int i = 0; i < numOrgs; ++i) {
        if (isApex(web, numOrgs, i)) {
            printf("  %s\n", web[i].name);
        }
    }
    printf("\n");

    // Producers
    if (modified) printf("UPDATED ");
    printf("Producers:\n");
    for (int i = 0; i < numOrgs; ++i) {
        if (web[i].numPrey == 0) {
            printf("  %s\n", web[i].name);
        }
    }
    printf("\n");

    // print all with max numPrey.
    if (modified) printf("UPDATED ");
    printf("Most Flexible Eaters:\n");
    int maxPrey = 0;
    for (int i = 0; i < numOrgs; ++i) {
        if (web[i].numPrey > maxPrey) {
            maxPrey = web[i].numPrey;
        }
    }
    for (int i = 0; i < numOrgs; ++i){
        if (web[i].numPrey == maxPrey) {
            printf("  %s\n", web[i].name);
        }
    }
    printf("\n");

    // print all with max in-degree.
    if (modified) printf("UPDATED ");
    printf("Tastiest Food:\n");
    int* cnt = (int*)calloc(numOrgs, sizeof(int));
    if (cnt) {
        for (int i = 0; i < numOrgs; ++i) {
            for (int j = 0; j < web[i].numPrey; ++j) {
                cnt[web[i].prey[j]]++;
            }
        }
        int best = 0;
        for (int i = 0; i < numOrgs; ++i) {
            if (cnt[i] > best) {
                best = cnt[i];
            }
        }
        for (int i = 0; i < numOrgs; ++i) {
            if (cnt[i] == best) {
                printf("  %s\n", web[i].name);
            }
        }
        free(cnt);
    }
    printf("\n");

    // always print "  name: h"
    if (modified) printf("UPDATED ");
    printf("Food Web Heights:\n");
    int* memo = (int*)malloc(sizeof(int)*numOrgs);
    int* vis = (int*)calloc(numOrgs, sizeof(int));
    if (memo && vis) { 
        for (int i = 0; i < numOrgs; ++i) {
            memo[i] = -1;
        }
        for (int i = 0; i < numOrgs; ++i) {
            memset(vis, 0, sizeof(int)*numOrgs);
            vis[i] = 1;
            int h = heightDFS(i, web, numOrgs, memo, vis);
            vis[i] = 0;
            printf("  %s: %d\n", web[i].name, h);
        }
    }
    if (memo) {
        free(memo);
    }
    if (vis) {
        free(vis);
    }
    printf("\n");

    if (modified) printf("UPDATED ");
    printf("Vore Types:\n");
    
    printf("  Producers:\n");
    for (int i = 0; i < numOrgs; ++i) {
        if (web[i].numPrey == 0) {
            printf("    %s\n", web[i].name);
        }
    }

    printf("  Herbivores:\n");
    for (int i = 0; i < numOrgs; ++i) {
        if (web[i].numPrey > 0) {
            bool eatProd = false, eatNonProd = false;
            for (int j = 0; j < web[i].numPrey; ++j) {
                int v = web[i].prey[j];
                if (web[v].numPrey == 0) {
                    eatProd = true;
                }
                else {
                    eatNonProd = true;
                }
            }
            if (eatProd && !eatNonProd) {
                printf("    %s\n", web[i].name);
            }
        }
    }

    printf("  Omnivores:\n");
    for (int i = 0; i < numOrgs; ++i) {
        if (web[i].numPrey > 0) {
            bool eatProd = false, eatNonProd = false;
            for (int j = 0; j < web[i].numPrey; ++j) {
                int v = web[i].prey[j];
                if (web[v].numPrey == 0) {
                    eatProd = true;
                }
                else {
                    eatNonProd = true;
                }
            }
            if (eatProd && eatNonProd) {
                printf("    %s\n", web[i].name);
            }
        }
    }

    printf("  Carnivores:\n");
    for (int i = 0; i < numOrgs; ++i) {
        if (web[i].numPrey > 0) {
            bool eatProd = false, eatNonProd = false;
            for (int j = 0; j < web[i].numPrey; ++j) {
                int v = web[i].prey[j];
                if (web[v].numPrey == 0) {
                    eatProd = true;
                }
                else {
                    eatNonProd = true;
                }
            }
            if (!eatProd && eatNonProd) {
                printf("    %s\n", web[i].name);
            }
        }
    }
    printf("\n");
}

// parse -b/-d/-q and set booleans; false on invalid and duplicate.
bool setModes(int argc, char* argv[], bool* pBasicMode, bool* pDebugMode, bool* pQuietMode) {
    *pBasicMode = false;
    *pDebugMode = false;
    *pQuietMode = false;

    bool seenB=false, seenD=false, seenQ=false;

    for (int i = 1; i < argc; ++i) {
        char* a = argv[i];
        if (!a || a[0] != '-') return false;

        if (a[1]=='b') {if (seenB) return false; seenB=true; *pBasicMode=true;}
        else if (a[1]=='d') {if (seenD) return false; seenD=true; *pDebugMode=true;}
        else if (a[1]=='q') {if (seenQ) return false; seenQ=true; *pQuietMode=true;}
        else return false; // invalid flag
    }
    return true;
}

// helper for settings display.
void printONorOFF(bool mode) {
    if (mode) {
        printf("ON\n");
    } else {
        printf("OFF\n");
    }
}


int main(int argc, char* argv[]) {  
    bool basicMode = false;
    bool debugMode = false;
    bool quietMode = false;

    // Parse command-line flags; exit on invalid or duplicate.
    if (!setModes(argc, argv, &basicMode, &debugMode, &quietMode)) {
        printf("Invalid command-line argument. Terminating program...\n");
        return 1;
    }

    // Show settings for reproducibility.
    printf("Program Settings:\n");
    printf("  basic mode = ");
    printONorOFF(basicMode);
    printf("  debug mode = ");
    printONorOFF(debugMode);
    printf("  quiet mode = ");
    printONorOFF(quietMode);
    printf("\n");

    // Build initial web: names until "DONE".
    int numOrgs = 0;
    printf("Welcome to the Food Web Application\n\n");
    printf("--------------------------------\n\n");

    Org* web = NULL;

    printf("Building the initial food web...\n");
    
    if (!quietMode) printf("Enter the name for an organism in the web (or enter DONE): ");
    char tempName[20] = "";
    scanf("%s",tempName); 
    if (!quietMode) printf("\n");
    while (strcmp(tempName,"DONE") != 0) {
        addOrgToWeb(&web,&numOrgs,tempName);
        if (debugMode) {
            printf("DEBUG MODE - added an organism:\n");
            printWeb(web,numOrgs);
            printf("\n");
        }
        if (!quietMode) printf("Enter the name for an organism in the web (or enter DONE): ");
        scanf("%s",tempName); 
        if (!quietMode) printf("\n");
    }
    if (!quietMode) printf("\n");

    // Read initial predator-prey relations as index pairs; stop on invalid.
    if (!quietMode) printf("Enter the pair of indices for a predator/prey relation.\n");
    if (!quietMode) printf("Enter any invalid index when done (-1 2, 0 -9, 3 3, etc.).\n");
    if (!quietMode) printf("The format is <predator index> <prey index>: ");
        
    int predInd, preyInd;
    scanf("%d %d",&predInd, &preyInd);
    if (!quietMode) printf("\n");

    while (predInd >= 0 && preyInd >= 0 && predInd < numOrgs &&  preyInd < numOrgs && predInd != preyInd) {
        addRelationToWeb(web,numOrgs,predInd,preyInd);
        if (debugMode) {
            printf("DEBUG MODE - added a relation:\n");
            printWeb(web,numOrgs);
            printf("\n");
        }
        if (!quietMode) printf("Enter the pair of indices for a predator/prey relation.\n");
        if (!quietMode) printf("Enter any invalid index when done (-1 2, 0 -9, 3 3, etc.).\n");
        if (!quietMode) printf("The format is <predator index> <prey index>: ");
        
        scanf("%d %d",&predInd, &preyInd);  
        if (!quietMode) printf("\n");
    }
    printf("\n");

    // Analyze the initial web.
    printf("--------------------------------\n\n");
    printf("Initial food web complete.\n");
    printf("Displaying characteristics for the initial food web...\n");
    
    displayAll(web,numOrgs,false);

    // Interactive modification loop.
    if (!basicMode) {
        printf("--------------------------------\n\n");
        printf("Modifying the food web...\n\n");
        char opt = '?';

        while (opt != 'q') {
            if (!quietMode) {
                printf("Web modification options:\n");
                printf("   o = add a new organism (expansion)\n");
                printf("   r = add a new predator/prey relation (supplementation)\n");
                printf("   x = remove an organism (extinction)\n");
                printf("   p = print the updated food web\n");
                printf("   d = display ALL characteristics for the updated food web\n");
                printf("   q = quit\n");
                printf("Enter a character (o, r, x, p, d, or q): ");
            }
            scanf(" %c", &opt);
            if (!quietMode) printf("\n\n");

            if (opt == 'o') {
                char newName[20];
                if (!quietMode) printf("EXPANSION - enter the name for the new organism: ");
                scanf("%s",newName);
                if (!quietMode) printf("\n");
                printf("Species Expansion: %s\n", newName);
                addOrgToWeb(&web,&numOrgs,newName);
                printf("\n");

                if (debugMode) {
                    printf("DEBUG MODE - added an organism:\n");
                    printWeb(web,numOrgs);
                    printf("\n");
                }

            } else if (opt == 'x') {
                int extInd;
                if (!quietMode) printf("EXTINCTION - enter the index for the extinct organism: ");
                scanf("%d",&extInd);
                if (!quietMode) printf("\n");
                if (extInd >= 0 && extInd < numOrgs) {
                    printf("Species Extinction: %s\n", web[extInd].name);
                    removeOrgFromWeb(&web,&numOrgs,extInd);
                } else {
                    printf("Invalid index for species extinction\n");
                }
                printf("\n");
                
                if (debugMode) {
                    printf("DEBUG MODE - removed an organism:\n");
                    printWeb(web,numOrgs);
                    printf("\n");
                }

            } else if (opt == 'r') {
                if (!quietMode) printf("SUPPLEMENTATION - enter the pair of indices for the new predator/prey relation.\n");
                if (!quietMode) printf("The format is <predator index> <prey index>: ");
                scanf("%d %d",&predInd, &preyInd);
                if (!quietMode) printf("\n");

                if (addRelationToWeb(web,numOrgs,predInd,preyInd)) {
                    printf("New Food Source: %s eats %s\n", web[predInd].name, web[preyInd].name);
                };
                printf("\n");
                if (debugMode) {
                    printf("DEBUG MODE - added a relation:\n");
                    printWeb(web,numOrgs);
                    printf("\n");
                }

            } else if (opt == 'p') {
                printf("UPDATED Food Web Predators & Prey:\n");
                printWeb(web,numOrgs);
                printf("\n");
                
            } else if (opt == 'd') {
                printf("Displaying characteristics for the UPDATED food web...\n\n");
                displayAll(web,numOrgs,true);

            }
            printf("--------------------------------\n\n");
        
        }
        
    }

    // Free all dynamic memory and exit,
    freeWeb(web,numOrgs);
    return 0;
}