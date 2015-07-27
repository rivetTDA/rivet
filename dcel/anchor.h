/**
 * \class	Anchor
 * \brief	Stores a Anchor: a multi-index pair along with a pointer to the line representing the Anchor in the arrangement
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __ANCHOR_H__
#define __ANCHOR_H__

//forward declarations
class Halfedge;
struct xiMatrixEntry;


class Anchor
{
    public:
        //JULY 2015 BUG FIX:
//        Anchor(xiMatrixEntry* down, xiMatrixEntry* left);  //constructor for a strict Anchor, requires pointers to the "generators" of the anchor
//        Anchor(xiMatrixEntry* point, bool strong);         //constructor for a supported Anchor, requires pointer to the xi support point at the anchor and a bool indicating whether this anchor is strict (i.e. whether xi support points exist both left and down)
        Anchor(xiMatrixEntry* e);                   //default constructor
        Anchor(unsigned x, unsigned y);                    //constructor, requires only x- and y-coordinates
        Anchor(const Anchor& other);                       //copy constructor

        Anchor& operator= (const Anchor& other);	//assignment operator
        bool operator== (const Anchor& other) const;	//equality operator

        bool comparable(Anchor* other) const;   //tests whether two Anchors are (strongly) comparable

        unsigned get_x() const;		//get the discrete x-coordinate (e.g. time index) of the multi-index
        unsigned get_y() const;		//get the discrete y-coordinate (e.g. distance index) of the multi-index

        void set_line(Halfedge* e);	//set the pointer to the line corresponding to this Anchor in the arrangement
        Halfedge* get_line() const;		//get the pointer to the line corresponding to this Anchor in the arrangement

        void set_position(unsigned p);  //sets the relative position of the Anchor line at the sweep line, used for Bentley-Ottmann DCEL construction algorithm
        unsigned get_position() const;  //gets the relative position of the Anchor line at the sweep line, used for Bentley-Ottmann DCEL construction algorithm

        bool is_above();       //returns true iff this Anchor is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
        void toggle();         //toggles above/below state of this Anchor; called whever the slice line crosses this Anchor in the vineyard-update process of storing persistence data

        xiMatrixEntry* get_entry(); //accessor -- JULY 2015 BUG FIX:
        xiMatrixEntry* get_down();  //accessor -- unnecessary now
        xiMatrixEntry* get_left();  //accessor -- unnecessary now

    private:
        unsigned x_coord;	//discrete x-coordinate (e.g. time) of multi-index
        unsigned y_coord;	//discrete y-coordinate (e.g. distance) of multi-inded

        //JULY 2015 BUG FIX:
        xiMatrixEntry* entry;   //xiMatrixEntry at the position of this anchor

//        xiMatrixEntry* down;    //generator of this anchor located below the anchor
//        xiMatrixEntry* left;    //generator of this anchor located left of the anchor
            //NOTE: if the anchor is both strict and supported, then down is NULL and left points to the xiMatrixEntry at the anchor coordinates
            //      if the anchor is not strict (but supported), then left is NULL and down points to the xiMatrixEntry at the anchor coordinates

        Halfedge* dual_line;    //pointer to left-most halfedge corresponding to this Anchor in the arrangement
        unsigned position;      //relative position of Anchor line at sweep line, used for Bentley-Ottmann DCEL construction algorithm
        bool above_line;        //true iff this Anchor is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
};


//TODO: IS IT OK TO IMPLEMENT THE FOLLOWING COMPARATOR HERE, IN THE .h FILE???

class Anchor_LeftComparator
{
    public:
        bool operator() (const Anchor* lhs, const Anchor* rhs) const  //returns true if lhs comes before rhs
        {
            if(lhs->get_y() > rhs->get_y())		//first compare y-coordinates (reverse order)
                return true;
            if(lhs->get_y() == rhs->get_y() && lhs->get_x() < rhs->get_x())		//then compare x-coordinates (natural order)
                return true;
            return false;
        }
};


#endif // __ANCHOR_H__
