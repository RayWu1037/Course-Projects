// boardcell.h
//
// BoardCell class starter code for course project
// University of Illinois at Chicago
// CS 211 - Programming Practicum
// Original Author: Professor Scott Reckinger
//
// Author: Ruiyi Wu
// Date: 2025-12-03
// Description: Completed movement logic and collision handing for hero and baddies.

#ifndef _BOARDCELL_H
#define _BOARDCELL_H

using namespace std;

// First, the BoardCell abstract base class 

class BoardCell {
	
    public:  
		BoardCell() {} // default contstructor (do nothing)
        virtual ~BoardCell() {} // destructor (do nothing)
        
        virtual char display( ) = 0; // pure virtual function; this is an abstract base class

    	virtual void attemptMoveTo(size_t& newR, size_t& newC, size_t hRow, size_t hCol) {
            
            //------------------------------------------------------------
            //       write the base class funtion for a board cell's 
            //       attempted move to position (newR,newC). 
            //
            //       The default action is to stay put, i.e. new position 
            //       (newR, newC) is same as old position (myRow, myCol).
            //
            //       Note: the Hero's current position (hRow,hCol) is
            //       needed for this polymorphic function in some derived
            //       classes, specifically for Baddie movement
            //------------------------------------------------------------
            (void)hRow;
            (void)hCol;
            newR = getRow();
            newC = getCol();
            
        }
        
        virtual bool isHero( ) {return false;}
    	virtual bool isBaddie( ) {return false;}
    	virtual bool isSpace( ) {return false;}
        virtual bool isBarrier( ) {return false;}
        virtual bool isHole( ) {return false;}
    	virtual bool isExit( ) {return false;}
    	virtual bool isStatic( ) {return true;}

        virtual void setPower(int num) {}
        virtual int getPower() {return 1;}
        virtual void setNextMove(char inChar) {}
        
        void setMoved(bool m) {moved = m;}
        bool getMoved() {return moved;}
        void setRow(size_t r) {myRow = r;}
        size_t getRow() {return myRow;}
        void setCol(size_t c) {myCol = c;}
        size_t getCol() {return myCol;}
        void setPos(size_t r, size_t c) {
            setRow(r);
            setCol(c);
        }

    private:
        size_t myRow; // current row for this board cell in a 2D grid
        size_t myCol; // current column for this board cell in a 2D grid
        bool moved;   // true = this board cell already moved in the current round

}; // BoardCell (abstract base class)


// Then, all the derived classes

class Hero: public BoardCell {
	
    public:
    	Hero(size_t r, size_t c) {
            setRow(r);
            setCol(c);
        }
        
        virtual bool isHero( ) {return true;}
        virtual bool isStatic( ) {return false;}
        virtual char display( ) {return 'H';}
        
        virtual void setNextMove(char inChar ) {
            nextMove = inChar;
        }
    	
        virtual void attemptMoveTo(size_t& newR, size_t& newC, size_t hRow, size_t hCol) {
            //------------------------------------------------------------------------
            //      write attemptMoveTo() for Hero 
            //      
            //      Hero's attempted move is determined by the nextMove data member 
            //      analyze nextMove to determine attempted new position for Hero
            //          'q' = up and left       'w' = up        'e' = up and right
            //          'a' = left              's' = stay      'd' = right
            //          'z' = down and left     'x' = down      'c' = down and right
            //       interpret ANY other input as 's' = stay
            //------------------------------------------------------------------------
            (void)hRow;
            (void)hCol;

            size_t curR = getRow();
            size_t curC = getCol();

            int dr = 0;
            int dc = 0;

            switch (nextMove) {
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

            int r = static_cast<int>(curR) + dr;
            int c = static_cast<int>(curC) + dc;

            newR = static_cast<size_t>(r); 
            newC = static_cast<size_t>(c); 
        }

    private:
        char nextMove;

}; // Hero


class Monster: public BoardCell {
    
    public:
		Monster(size_t r, size_t c) {
            setRow(r);
            setCol(c);
            power = 1;
        }
        
        virtual bool isBaddie( ) {return true;}
        virtual bool isStatic( ) {return false;}
        
        virtual char display( ) {
            if (power == 2) {
                return 'M'; // Super Monster
            } else {
                return 'm'; // Monster
            } 
        }

        virtual void setPower(int num) {
            power = num;
        }

        virtual int getPower() {
            return power;
        }
        
    	virtual void attemptMoveTo(size_t& newR, size_t& newC, size_t hRow, size_t hCol) {
            
            //------------------------------------------------------------------------
            //      write attemptMoveTo() for Monster 
            //      
            //       Monsters always attempt to navigate toward the hero;
            //          the hero's position is passed in as (hRow,hCol);
            //              - regular monsters attempt to move...
            //                  1 step vertically closer to hero
            //                  (unless already in same column)
            //                              AND 
            //                  1 step horizontally closer to hero
            //                  (unless already in same row)
            //              - super monsters attempt to move...
            //                  2 steps vertically closer to hero
            //                  (unless already in same column)
            //                              AND 
            //                  2 steps horizontally closer to hero
            //                  (unless already in same row)
            //          note: super monsters are BIG and CANNOT make 1-step moves in
            //                either direction; i.e. if they move vertically or 
            //                horizontally, it must be a 2-step move in either/both 
            //                direction(s)
            //------------------------------------------------------------------------
            size_t curR = getRow();
            size_t curC = getCol();

            int dr = 0;
            int dc = 0;
            int step = power;

            if (hRow < curR) {
                dr = -step;
            }
            else if (hRow > curR) {
                dr = +step;
            }

            if (hCol < curC) {
                dc = -step;
            }
            else if (hCol > curC) {
                dc = +step;
            } 

            int r = static_cast<int>(curR) + dr;
            int c = static_cast<int>(curC) + dc;

            newR = static_cast<size_t>(r); 
            newC = static_cast<size_t>(c); 
        }
    
    private:
        int power; // power = 1 for Monster, power = 2 for SuperMonster

}; // Monster

class Bat: public BoardCell {
    public:
		Bat(size_t r, size_t c) {
            setRow(r);
            setCol(c);
        }
        virtual bool isBaddie( ) {return true;}
        virtual bool isStatic( ) {return false;}
        virtual char display( ) {return '~';}
        
    	virtual void attemptMoveTo(size_t& newR, size_t& newC, size_t hRow, size_t hCol) {
            //------------------------------------------------------------------------
            //       write attemptMoveTo() for Bat 
            //      
            //       Bats always attempt to navigate to the hero's column, 
            //       but cannot change rows;
            //------------------------------------------------------------------------
            (void)hRow;
            newR = getRow();
            newC = hCol;
        }
}; // Bat


class Abyss: public BoardCell {
	public:
    	Abyss(size_t r, size_t c) {
            setRow(r);
            setCol(c);
        }
        virtual char display( ) {return '#';}
        virtual bool isHole( ) {return true;}
}; // Abyss


class Wall: public BoardCell {
	public:
    	Wall(size_t r, size_t c) {
            setRow(r);
            setCol(c);
        }
        virtual char display( ) {return '+';}
    	virtual bool isBarrier( ) {return true;}
}; // Wall


class Nothing: public BoardCell {
	public:
    	Nothing(size_t r, size_t c) {
            setRow(r);
            setCol(c);
        }
        virtual char display( ) {return ' ';}
    	virtual bool isSpace( ) {return true;}
}; // Nothing


class EscapeLadder: public BoardCell {
	public:
    	EscapeLadder(size_t r, size_t c) {
            setRow(r);
            setCol(c);
        }
        virtual char display( ) {return '*';}
    	virtual bool isExit( ) {return true;}
}; // EscapeLadder


#endif //_BOARDCELL_H
