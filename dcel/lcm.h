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
        LCM(xiMatrixEntry* point);  //constructor for a WEAK LCM, requires pointer to the xi support at the LCM

      ///TODO: these other constructors might be unnecessary now:
//        LCM(unsigned x, unsigned y);		//constructor, requires only time and distance values
//        LCM(unsigned x, unsigned y, Halfedge* e);	//constructor, requires all three parameters
        LCM(const LCM& other);			//copy constructor

        LCM& operator= (const LCM& other);	//assignment operator
        bool operator== (const LCM& other) const;	//equality operator

        bool comparable(LCM* other) const;   //tests whether two LCMs are (strongly) comparable

        unsigned get_x() const;		//get the discrete x-coordinate (e.g. time index) of the multi-index
        unsigned get_y() const;		//get the discrete y-coordinate (e.g. distance index) of the multi-index

        void set_line(Halfedge* e);	//set the pointer to the curve corresponding to this LCM in the arrangement
        Halfedge* get_line() const;		//get the pointer to the curve corresponding to this LCM in the arrangement

        void set_position(unsigned p);  //sets the relative position of the LCM curve at the sweep line, used for Bentley-Ottmann DCEL construction algorithm
        unsigned get_position() const;  //gets the relative position of the LCM curve at the sweep line, used for Bentley-Ottmann DCEL construction algorithm

        xiMatrixEntry* get_down();  //accessor
        xiMatrixEntry* get_left();  //accessor

    private:
        unsigned x_coord;	//discrete x-coordinate (e.g. time) of multi-index
        unsigned y_coord;	//discrete y-coordinate (e.g. distance) of multi-inded

        xiMatrixEntry* down;    //"down generator" of this LCM; if this is a weak LCM, then this generator is at the LCM position
        xiMatrixEntry* left;    //"left generator" of this LCM; this pointer is NULL if and only if this is a weak LCM

        Halfedge* curve;	//pointer to left-most halfedge corresponding to this LCM in the arrangement --- IS THIS USED FOR ANYTHING BESIDES TESTING???
        unsigned position;  //relative position of LCM curve at sweep line, used for Bentley-Ottmann DCEL construction algorithm
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
