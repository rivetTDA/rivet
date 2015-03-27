/**
 * \class	LCM
 * \brief	Stores a LCM: a multi-index pair along with a pointer to the curve representing the LCM in the arrangement
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_LCM_H__
#define __DCEL_LCM_H__

struct xiMatrixEntry;

class Halfedge;

class LCM   //updated to store only discrete indexes
{
    public:
        LCM(xiMatrixEntry* down, xiMatrixEntry* left);  //constructor for a NON-WEAK LCM, requires pointers to the "generators" of the LCM
        LCM(xiMatrixEntry* point, bool strong);         //constructor for a WEAK LCM, requires pointer to the xi support at the LCM and a bool indicating whether this LCM is strong (i.e. whether xi support points exist both left and down)
        LCM(unsigned x, unsigned y);                    //constructor, requires only x- and y-coordinates
        LCM(const LCM& other);                          //copy constructor

        LCM& operator= (const LCM& other);	//assignment operator
        bool operator== (const LCM& other) const;	//equality operator

        bool comparable(LCM* other) const;   //tests whether two LCMs are (strongly) comparable

        unsigned get_x() const;		//get the discrete x-coordinate (e.g. time index) of the multi-index
        unsigned get_y() const;		//get the discrete y-coordinate (e.g. distance index) of the multi-index

        void set_line(Halfedge* e);	//set the pointer to the line corresponding to this LCM in the arrangement
        Halfedge* get_line() const;		//get the pointer to the line corresponding to this LCM in the arrangement

        void set_position(unsigned p);  //sets the relative position of the LCM curve at the sweep line, used for Bentley-Ottmann DCEL construction algorithm
        unsigned get_position() const;  //gets the relative position of the LCM curve at the sweep line, used for Bentley-Ottmann DCEL construction algorithm

        bool is_above();       //returns true iff this LCM is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
        void toggle();         //toggles above/below state of this LCM; called whever the slice line crosses this LCM in the vineyard-update process of storing persistence data

        xiMatrixEntry* get_down();  //accessor
        xiMatrixEntry* get_left();  //accessor

    private:
        unsigned x_coord;	//discrete x-coordinate (e.g. time) of multi-index
        unsigned y_coord;	//discrete y-coordinate (e.g. distance) of multi-inded

        xiMatrixEntry* down;    //"down generator" of this LCM; if this is a weak LCM, then this generator is at the LCM position
        xiMatrixEntry* left;    //"left generator" of this LCM; this pointer is NULL if and only if this is a weak LCM

        Halfedge* curve;	//pointer to left-most halfedge corresponding to this LCM in the arrangement --- IS THIS USED FOR ANYTHING BESIDES TESTING???
        unsigned position;  //relative position of LCM curve at sweep line, used for Bentley-Ottmann DCEL construction algorithm
        bool above_line;    //true iff this LCM is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
};


//TODO: IS IT OK TO IMPLEMENT THE FOLLOWING COMPARATOR HERE, IN THE .h FILE???

class LCM_LeftComparator    //updated to work for linear arrangements, using discrete x- and y-coordinates
{
    public:
        bool operator() (const LCM* lhs, const LCM* rhs) const  //returns true if lhs comes before rhs
        {
            if(lhs->get_y() > rhs->get_y())		//first compare y-coordinates (reverse order)
                return true;
            if(lhs->get_y() == rhs->get_y() && lhs->get_x() < rhs->get_x())		//then compare x-coordinates (natural order)
                return true;
            return false;
        }
};





#endif // __DCEL_LCM_H__
