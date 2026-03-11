// gameboard.h
//
// GameBoard class starter code for course project
// University of Illinois at Chicago
// CS 211 - Programming Practicum
// Original Author: Professor Scott Reckinger
//
// Author: Ruiyi Wu
// Date: 2025-12-03
// Description: Completed movement logic and collision handing for hero and baddies.

#ifndef _GAMEBOARD_H
#define _GAMEBOARD_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <ctime>
#include <stdexcept>

#include "boardcell.h"
#include "grid.h"

using namespace std;

class GameBoard {
	private: 
	    Grid<BoardCell*> board;
        size_t numRows;
        size_t numCols;
        size_t HeroRow; // Hero's position row
	    size_t HeroCol; // Hero's position column
        int numMonsters;
        int numSuperMonsters;
        int numAbysses;
        int numBats;
        bool wonGame; // false, unless the Hero reached the exit successfully

		
	public: 
		/* default constructor */
        GameBoard() {
            numMonsters = 4;
            numSuperMonsters = 2;
            numAbysses = 50;
            numBats = 2;
            wonGame = false;
            
            this -> numRows = 15;
            this -> numCols = 40;
            
            Grid<BoardCell*> boardnew(numRows, numCols);
            board = boardnew;
            
            blankBoard();
        }
        
        /* param constructor */
        GameBoard(size_t numRows, size_t numCols) {
            numMonsters = 4;
            numSuperMonsters = 2;
            numAbysses = 20;
            numBats = 3;
            wonGame = false;
            
            this -> numRows = numRows;
            this -> numCols = numCols;
            
            Grid<BoardCell*> boardnew(numRows, numCols);
            board = boardnew;
            
            blankBoard();
        }
        
        /* destructor */
        virtual ~GameBoard() {
            for (size_t row = 0; row < board.numrows(); row++) {
                for (size_t col = 0; col < board.numcols(row); col++) {
                    delete board(row, col);
                }
            }
        }

        void blankBoard() {
            for (size_t row = 0; row < board.numrows(); row++) {
                for (size_t col = 0; col < board.numcols(row); col++) {
                    board(row, col) = new Nothing(row,col);
                }
            }
        }

        char getCellDisplay(size_t r, size_t c) {
            return board(r,c)->display();
        }

        void setCell(BoardCell* myCell, size_t r, size_t c) {
            board(r,c) = myCell;
        }
    
        void freeCell(size_t r, size_t c) {
            delete board(r,c);
        }

        // fills board with by randomly placing...
        //  - Hero (H) in the first three columns
        //  - EscapeLadder (*) in last three columns
        //  - 3 vertical Walls (+), each 1/2 of board height, in middle segment
        //  - Abyss cells (#), quantity set by numAbysses, in middle segment
        //  - Baddies [Monsters (m), Super Monsters (M), & Bats (~)] in middle segment;
        //    number of Baddies set by numMonsters, numSuperMonsters, & numBats
        void setupBoard(int seed) {
            srand(seed);
            size_t r,c;

            r = rand() % numRows;
            c = rand() % 3;
            delete board(r,c);
            board(r,c) = new Hero(r,c);
            HeroRow = r;
            HeroCol = c;

            r = rand() % numRows;
            c = numCols - 1 - (rand() % 3);
            delete board(r,c);
            board(r,c) = new EscapeLadder(r,c);
            
            int sizeMid = numCols - 6;

            c = 3 + (rand() % sizeMid);
            for (r = 0; r < numRows/2; ++r) {
                delete board(r,c);
                board(r,c) = new Wall(r,c);
            }
            size_t topc = c;

            while (c == topc || c == topc-1 || c == topc+1) {
                c = 3 + (rand() % sizeMid);
            }
            for (r = numRows-1; r > numRows/2; --r) {
                delete board(r,c);
                board(r,c) = new Wall(r,c);           
            }
            size_t botc = c;

            while (c == topc || c == topc-1 || c == topc+1 || c == botc || c == botc-1 || c == botc+1) {
                c = 3 + (rand() % sizeMid);
            }
            for (r = numRows/4; r < 3*numRows/4; ++r) {
                delete board(r,c);
                board(r,c) = new Wall(r,c);
            }

            for (int i = 0; i < numMonsters; ++i) {
                r = rand() % numRows;
                c = 3 + (rand() % sizeMid);
                while (board(r,c)->display() != ' ') {
                    r = rand() % numRows;
                    c = 3 + (rand() % sizeMid);
                }
                delete board(r,c);
                board(r,c) = new Monster(r,c);  
                board(r,c)->setPower(1);        
            }

            for (int i = 0; i < numSuperMonsters; ++i) {
                r = rand() % numRows;
                c = 3 + (rand() % sizeMid);
                while (board(r,c)->display() != ' ') {
                    r = rand() % numRows;
                    c = 3 + (rand() % sizeMid);
                }
                delete board(r,c);
                board(r,c) = new Monster(r,c); 
                board(r,c)->setPower(2);               
            }

            for (int i = 0; i < numBats; ++i) {
                r = rand() % numRows;
                c = 3 + (rand() % sizeMid);
                while (board(r,c)->display() != ' ') {
                    r = rand() % numRows;
                    c = 3 + (rand() % sizeMid);
                }
                delete board(r,c);
                board(r,c) = new Bat(r,c); 
            }

            for (int i = 0; i < numAbysses; ++i) {
                r = rand() % numRows;
                c = 3 + (rand() % sizeMid);
                while (board(r,c)->display() != ' ') {
                    r = rand() % numRows;
                    c = 3 + (rand() % sizeMid);
                }
                delete board(r,c);
                board(r,c) = new Abyss(r,c);              
            }
        }

        // neatly displaying the game board 
		void display( ) {
            cout << '-';
            for (size_t col = 0; col < board.numcols(0); col++) {
                cout << '-';
            }
            cout << '-';
            cout << endl;
            for (size_t row = 0; row < board.numrows(); row++) {
                cout << '|';
                for (size_t col = 0; col < board.numcols(row); col++) {
                    cout << board(row,col)->display();
                }
                cout << '|';
                cout << endl;
            }
            cout << '-';
            for (size_t col = 0; col < board.numcols(0); col++) {
                cout << '-';
            }
            cout << '-';
            cout << endl;
            
        }
		
        bool getWonGame() {
            return wonGame;
        }
        
        // distributing total number of monsters so that 
        //  ~1/3 of num are Super Monsters (M), and
        //  ~2/3 of num are Regular Monsters (m)
        void setNumMonsters(int num) {
            numSuperMonsters = num/3;
            numMonsters = num - numSuperMonsters;
        }

        void setNumAbysses(int num) {
            numAbysses = num;
        }

        void setNumBats(int num) {
            numBats = num;
        }

        size_t getNumRows() {
            return numRows;
        }

        size_t getNumCols() {
            return numCols;
        }

        
        //---------------------------------------------------------------------------------
        // void getHeroPosition(size_t& row, size_t& col)
        //
        // getter for Hero's position, which are private data members
        //      int HeroRow;
	    //      int HeroCol;
        // note: row and col are passed-by-reference
        //---------------------------------------------------------------------------------
        void getHeroPosition(size_t& row, size_t& col) {
            row = HeroRow;
            col = HeroCol;
        }

        
        //---------------------------------------------------------------------------------
        // void setHeroPosition(size_t row, size_t col)
        //
        // setter for Hero's position, which are private data members
        //      int HeroRow;
	    //      int HeroCol;
        //---------------------------------------------------------------------------------
        void setHeroPosition(size_t row, size_t col) {
            HeroRow = row;
            HeroCol = col;
        }

        
        //---------------------------------------------------------------------------------
        // findHero()
        //
        // updater for Hero's position, which are private data members
        //      int HeroRow;
	    //      int HeroCol;
        // this function should find Hero in board and update
        //      HeroRow and HeroCol with the Hero's updated position;
        // if Hero cannot be found in board, then set Hero's position to (-1,-1)
        //---------------------------------------------------------------------------------
        void findHero() {
            bool found = false;
            for ( size_t r = 0; r < numRows && !found; ++r) {
                for (size_t c = 0; c < numCols && !found; ++c) {
                    BoardCell* cell = board(r, c);
                    if (cell != nullptr && cell->isHero()) {
                        setHeroPosition(r, c);
                        found = true;
                    }
                }
            }

            if (!found) {
                size_t off = static_cast<size_t>(-1);
                setHeroPosition(off, off);
            }
        }

        
        //---------------------------------------------------------------------------------
        // bool makeMoves(char HeroNextMove)
        // 
        // This is the primary gameplay operation for a single round of the game. 
        // A LOT happens in this function... 
        // General steps:
        //  - Allow user to input their next move 
        //  - Get the attempted move position for the Hero
        //  - Move the hero, catching/resolving any potential collision exceptions...
        //      - attempted move out-of-bounds: change row &/or col to stay on board
        //      - attempted move into a barrier: change row &/or col to stay off barrier
        //      - attempted move to the exit: remove hero from board, hero escaped!
        //      - attempted move into a hole: remove hero from board, hero did not escape
        //      - attempted move to a baddie: remove hero from board, hero did not escape
        //      - attempted move to an empty space: actual move is the attempted move
        //  - For each baddie (regular Monster, super Monster, or Bat) on the board...
        //      - check that this baddies hasn't already moved this round
        //      - get its attempted move position
        //      - move the baddie, catching/resolving any potential collision exceptions...
        //          - attempted move out-of-bounds: change row &/or col to stay on board
        //          - attempted move into a barrier: change row &/or col to stay off barrier
        //          - attempted move to the exit: change row &/or col to stay off exit
        //          - attempted move into a hole: remove baddie from board
        //          - attempted move to hero: remove hero from board, hero did not escape
        //          - attempted move to a baddie: ignore attempted move, no position change
        //          - attempted move to an empty space: actual move is the attempted move 
        // 
        // Note: all baddies (and the hero) fall into holes if their attempted 
        //       move ENDs on a hole (i.e. even Bats are drawn into the 
        //       Abyss if their attempted move takes them to an Abyss cell); 
        //       BUT, baddies can travel over holes, as long as their attempted 
        //       move does NOT END on a hole; this only applies, in practice, 
        //       to super monsters and bats, since their step sizes can be more 
        //       than 1 (monsters and the hero are limited to steps of size 1)
        //
        // Note: also, whereas baddies (and the hero) can never move onto a 
        //       barrier (i.e. a wall), they can step over barriers as long
        //       as the attempted move does NOT END on a barrier; this only 
        //       applies, in practice, to super monsters and bats, since their 
        //       step sizes can be more than 1 (monsters and the hero are limited 
        //       to steps of size 1)
        //
        // Note: to prevent a single baddie from making multiple moves
        //       during a single round of the game, whenever a baddie 
        //       has moved, it should be marked as "already moved" in 
        //       some way; this can be done using the [moved] data member
        //       of the BoardCell base class, which has setter/getter 
        //       functions provided. The [moved] data members must be 
        //       reset for all BoardCells at the beginning of each round.
        //
        // Note: the [myRow] and [myCol] data members for BoardCell derived
        //       class objects must be updated whenever a move is made; 
        //       additionally, [HeroRow] and [HeroCol] data members for the 
        //       GameBoard must be updated whenever the Hero has moved, 
        //       which can be done easily with a call to findHero()
        //
        // Note: many actual moves made by non-static board cell objects 
        //       (i.e. Heros, Monsters, Bats) involve constructing and/or 
        //       destructing objects; be careful with memory management; 
        //       specifically, make sure to free (delete) the memory for 
        //       BoardCell derived class objects when you are done with it
        //
        // return true if Hero is still on board at the end of the round
        // return false if Hero is NOT on board at the end of the round
        //---------------------------------------------------------------------------------
        bool makeMoves(char HeroNextMove) {
            // this function should use some try/catch statements for handling collision exceptions;
            // some sample code is provided to get you started...
            for (size_t r = 0; r < numRows; ++r) {
                for (size_t c = 0; c < numCols; ++c) {
                    if (board(r, c) != nullptr) {
                        board(r, c)->setMoved(false);
                    }
                }
            }
            // locate the hero's current position
            findHero();

            // If the hero is already gone
            size_t offPos = static_cast<size_t>(-1);
            // determine where hero proposes to move to
            if (HeroRow == offPos || HeroCol == offPos) {
                return false;
            }

            // Compute the hero's attempted move and handle collisions
            BoardCell* heroCell = board(HeroRow, HeroCol);
            
            size_t dummyR, dummyC;
            heroCell->setNextMove(HeroNextMove);
            heroCell->attemptMoveTo(dummyR, dummyC, HeroRow, HeroCol);

            int dr = 0;
            int dc = 0;
            switch (HeroNextMove) {
                case 'q': dr = -1; dc = -1; break;
                case 'w': dr = -1; dc = 0; break;
                case 'e': dr = -1; dc = 1; break;
                case 'a': dr = 0; dc = -1; break;
                case 's': dr = 0; dc = 0; break;
                case 'd': dr = 0; dc = 1; break;
                case 'z': dr = 1; dc = -1; break;
                case 'x': dr = 1; dc = 0; break;
                case 'c': dr = 1; dc = 1; break;
                default:  dr = 0; dc = 0; break;
            }

            int rowAtt = static_cast<int>(HeroRow) + dr;
            int colAtt = static_cast<int>(HeroCol) + dc;

            try {
                if (rowAtt < 0 || rowAtt >= static_cast<int>(numRows)) {
                    throw runtime_error("Hero trying to move out-of-bounds with an invalid row");
                }
            }
            catch (runtime_error& excpt) {
                cout << excpt.what() << endl;
                rowAtt = static_cast<int>(HeroRow);
                cout << "Changing row for Hero position to stay in-bounds" << endl;
            }

            try {
                if (colAtt < 0 || colAtt >= static_cast<int>(numCols)) {
                    throw runtime_error("Hero trying to move out-of-bounds with an invalid column");
                }
            }
            catch (runtime_error& excpt) {
                cout << excpt.what() << endl;
                colAtt = static_cast<int>(HeroCol);
                cout << "Changing column for Hero position to stay in-bounds" << endl;
            }

            size_t newR = static_cast<size_t>(rowAtt);
            size_t newC = static_cast<size_t>(colAtt);

            int dR = rowAtt - static_cast<int>(HeroRow);
            int dC = colAtt - static_cast<int>(HeroCol);

            BoardCell* dest = board(newR, newC);

            if (dest->isBarrier()) {
                if (dR == 0 || dC == 0) {
                    newR = HeroRow;
                    newC = HeroCol;
                }
                else {
                    int vRowAtt = static_cast<int>(HeroRow) + dR;
                    int vColAtt = static_cast<int>(HeroCol);

                    try {
                        if (vRowAtt < 0 || vRowAtt >= static_cast<int>(numRows)) {
                            throw runtime_error("Hero diagonal vertical component out-of-bounds");
                        }
                    }
                    catch (runtime_error& excpt) {
                        cout << excpt.what() << endl;
                        vRowAtt = static_cast<int>(HeroRow);
                    }

                    if (vRowAtt == static_cast<int>(HeroRow)) {
                        newR = HeroRow;
                        newC = HeroCol;
                    }
                    else {
                        size_t vR = static_cast<size_t>(vRowAtt);
                        size_t vC = HeroCol;
                        BoardCell* vDest = board(vR, vC);

                        if (vDest->isBarrier()) {
                            newR = HeroRow;
                            newC = HeroCol;
                        }
                        else {
                            newR = vR;
                            newC = vC;
                            dest = vDest;
                        }
                    }
                }
            }

            if (newR == HeroRow && newC == HeroCol) {
                heroCell->setMoved(true);
            }
            else if (dest->isExit()) {
                delete heroCell;
                board(HeroRow, HeroCol) = new Nothing(HeroRow, HeroCol);
                wonGame = true;
                size_t off = static_cast<size_t>(-1);
                HeroRow = off;
                HeroCol = off;
                return false;
            }
            else if (dest->isHole()) {
                delete heroCell;
                board(HeroRow, HeroCol) = new Nothing(HeroRow, HeroCol);
                wonGame = false;
                size_t off = static_cast<size_t>(-1);
                HeroRow = off;
                HeroCol = off;
                return false;
            }
            else if (dest->isBaddie()) {
                delete heroCell;
                board(HeroRow, HeroCol) = new Nothing(HeroRow, HeroCol);
                wonGame = false;
                size_t off = static_cast<size_t>(-1);
                HeroRow = off;
                HeroCol = off;
                return false;
            }
            else if (dest->isSpace()) {
                delete dest;
                board(newR, newC) = heroCell;
                board(HeroRow, HeroCol) = new Nothing(HeroRow, HeroCol);
                heroCell->setPos(newR, newC);
                heroCell->setMoved(true);
                setHeroPosition(newR, newC);
            }

            findHero();
            if (HeroRow == offPos || HeroCol == offPos) {
                return false;
            }

            size_t hRow = HeroRow;            
            size_t hCol = HeroCol;

            // move all baddies that have not yet moved this round.
            for (size_t r = 0; r < numRows; ++r) {
                for (size_t c = 0; c < numCols; ++c) {
                    BoardCell* cell = board(r, c);
                    if (cell == nullptr) {
                        continue;
                    }
                    if (!cell->isBaddie()) {
                        continue;
                    }
                    if (cell->getMoved()) {
                        continue;
                    }

                    //if (HeroRow == offPos || HeroCol == offPos) {
                      //  return false;
                    //}

                    size_t tryR, tryC;
                    cell->attemptMoveTo(tryR, tryC, hRow, hCol);

                    int bRowAtt = static_cast<int>(tryR);
                    int bColAtt = static_cast<int>(tryC);

                    try {
                        if (bRowAtt < 0 || bRowAtt >= static_cast<int>(numRows)) {
                            throw runtime_error("Baddies trying to move out-of-bounds with an invalid row");
                        }
                    }
                    catch (runtime_error& excpt) {
                        cout << excpt.what() << endl;
                        bRowAtt = static_cast<int>(r);
                        cout << "Changing row for Baddie position to stay in-bounds" << endl;
                    }

                    try {
                        if (bColAtt < 0 || bColAtt >= static_cast<int>(numCols)) {
                            throw runtime_error("Baddie trying to move out-of-bounds with an invalid column");
                        }
                    }
                    catch (runtime_error& excpt) {
                        cout << excpt.what() << endl;
                        bColAtt = static_cast<int>(c);
                        cout << "changing column for Baddie position to stay in-bounds" << endl;
                    }

                    size_t bNewR = static_cast<size_t>(bRowAtt);
                    size_t bNewC = static_cast<size_t>(bColAtt);

                    if (bNewR == r && bNewC == c) {
                        cell->setMoved(true);
                        continue;
                    }

                    BoardCell* bDest = board(bNewR, bNewC);

                    if (bDest->isBarrier() || bDest->isExit()) {
                        cell->setMoved(true);
                        continue;
                    }

                    if (bDest->isHole()) {
                        delete cell;
                        board(r, c) = new Nothing(r, c);
                        continue;
                    }

                    if (bDest->isHero()) {
                        delete bDest;
                        board(bNewR, bNewC) = cell;
                        board(r, c) = new Nothing(r, c);
                        cell->setPos(bNewR, bNewC);
                        cell->setMoved(true);

                        wonGame = false;
                        size_t off = static_cast<size_t>(-1);
                        HeroRow = off;
                        HeroCol = off;
                        continue;
                    }

                    if (bDest->isBaddie()) {
                        cell->setMoved(true);
                        continue;
                    }

                    if (bDest->isSpace()) {
                        delete bDest;
                        board(bNewR, bNewC) = cell;
                        board(r, c) = new Nothing(r, c);
                        cell->setPos(bNewR, bNewC);
                        cell->setMoved(true);
                        continue;
                    }

                    cell->setMoved(true);
                }
            }

            findHero();
            if (HeroRow == offPos || HeroCol == offPos) {
                return false;
            } 

            return true;
        }
};

#endif //_GAMEBOARD_H