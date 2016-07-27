
#ifndef __ANCHOR_H__
#define __ANCHOR_H__

#include <memory>

//forward declarations
class Halfedge;
struct xiMatrixEntry;


/**
 * \class	Anchor
 * \brief	Stores an Anchor: a multi-index pair along with a pointer to the line representing the Anchor in the arrangement
 * \author	Matthew L. Wright
 * \date	March 2014
 */
class Anchor
{
    public:
        Anchor(std::shared_ptr<xiMatrixEntry> e);           //default constructor
        Anchor(unsigned x, unsigned y);     //constructor, requires only x- and y-coordinates
//        Anchor(const Anchor& other);        //copy constructor
        Anchor(); //For serialization

//        Anchor& operator= (const Anchor& other);	//assignment operator
        bool operator== (const Anchor& other) const;	//equality operator

        bool comparable(const Anchor &other) const;   //tests whether two Anchors are (strongly) comparable

        unsigned get_x() const;		//get the discrete x-coordinate
        unsigned get_y() const;		//get the discrete y-coordinate

        void set_line(std::shared_ptr<Halfedge> e);	//set the pointer to the line corresponding to this Anchor in the arrangement
        std::shared_ptr<Halfedge> get_line() const;		//get the pointer to the line corresponding to this Anchor in the arrangement

        void set_position(unsigned p);  //sets the relative position of the Anchor line at the sweep line, used for Bentley-Ottmann DCEL construction algorithm
        unsigned get_position() const;  //gets the relative position of the Anchor line at the sweep line, used for Bentley-Ottmann DCEL construction algorithm

        bool is_above();       //returns true iff this Anchor is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
        void toggle();         //toggles above/below state of this Anchor; called whever the slice line crosses this Anchor in the vineyard-update process of storing persistence data

        std::shared_ptr<xiMatrixEntry> get_entry(); //accessor

        void set_weight(unsigned long w);   //sets the estimate of the cost of updating the RU-decomposition when crossing this anchor
        unsigned long get_weight();         //returns estimate of the cost of updating the RU-decomposition when crossing this anchor


    template <class Archive>
            void serialize(Archive &ar, const unsigned int version);
    private:
        unsigned x_coord;	//discrete x-coordinate
        unsigned y_coord;	//discrete y-coordinate

        std::shared_ptr<xiMatrixEntry> entry;   //xiMatrixEntry at the position of this anchor

        std::shared_ptr<Halfedge> dual_line;    //pointer to left-most halfedge corresponding to this Anchor in the arrangement
        unsigned position;      //relative position of Anchor line at sweep line, used for Bentley-Ottmann DCEL construction algorithm
        bool above_line;        //true iff this Anchor is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
        unsigned long weight;   //estimate of the cost of updating the RU-decomposition when crossing this anchor
};

template<typename T>
struct AnchorComparator {

public:
    bool operator() (const T &lhs, const T &rhs) const  //returns true if lhs comes before rhs
    {
        if(lhs.get_y() > rhs.get_y())		//first compare y-coordinates (reverse order)
            return true;
        return lhs.get_x() < rhs.get_x();
    }
};

//TODO: IS IT OK TO IMPLEMENT THE FOLLOWING COMPARATOR HERE, IN THE .h FILE???

struct Anchor_LeftComparator : AnchorComparator<Anchor> {};


#endif // __ANCHOR_H__
