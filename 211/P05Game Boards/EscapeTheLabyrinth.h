#include <utility>
#include <random>
#include <set>
#include "grid.h"
#include "maze.h"
using namespace std;

/* Change constant kYourNetID to store your netID 
 *    - your submission will be manually inspected to ensure
 *      you have used the exact string that is your netID
 *    - thus, it is vital you understand what your netID is
 *    - ex: Professor Reckinger's email is scotreck@uic.edu, so
 *          Professor Reckinger's netID is scotreck     
 *    - ex: Student Sparky's email is sspark211@uic.edu, so
 *          Student Sparky's netID is sspark211 
 * WARNING: Once you've set set this constant and started 
 * exploring your maze, do NOT edit the value of kYourNetID. 
 * Changing kYourNetID will change which maze you get back, 
 * which might invalidate all your hard work!
 */
const string kYourNetID = "wruiy";

/* Change these constants to contain the paths out of your mazes. */
const string kPathOutOfRegularMaze = "SENSSENNESSNNWSSSE";
const string kPathOutOfTwistyMaze = "SSSSEWNESN";

bool isPathToFreedom(MazeCell *start, const string& moves) {
    if (start == nullptr) {
        return false;
    }
    
    bool haveSpellbook = false;
    bool havePotion    = false;
    bool haveWand      = false;

    auto pickUp = [&](MazeCell* cell) {
        if (cell == nullptr) return;
        if (cell->whatsHere == "Spellbook") {
            haveSpellbook = true;
        }
        else if (cell->whatsHere == "Potion") {
            havePotion = true;
        }
        else if (cell->whatsHere == "Wand") {
            haveWand = true;
        }
    };

    MazeCell* curr = start;

    pickUp(curr);

    for (char ch : moves) {
        switch (ch) {
        case 'N':
            if (curr->north == nullptr) return false;
            curr = curr->north;
            break;
        case 'S':
            if (curr->south == nullptr) return false;
            curr = curr->south;
            break;
        case 'E':
            if (curr->east == nullptr) return false;
            curr = curr->east;
            break;
        case 'W':
            if (curr->west == nullptr) return false;
            curr = curr->west;
            break;
        default:
            return false;
        }

        pickUp(curr);
    }
    return haveSpellbook && havePotion && haveWand;
}
