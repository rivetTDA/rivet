/**
 * \class	LCM
 * \brief	Stores a LCM: a multi-index pair along with a pointer to the curve representing the LCM in the arrangement
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_LCM_H__
#define __DCEL_LCM_H__

class Halfedge;

class LCM
{
    public:
        LCM(double x, double y);		//constructor, requires only time and distance values
        LCM(double x, double y, Halfedge* e);	//constructor, requires all three parameters
        LCM(const LCM& other);			//copy constructor

        LCM& operator= (const LCM& other);	//assignment operator
        bool operator== (const LCM& other) const;	//equality operator

        double get_x() const;		//get the x-coordinate (e.g. time) of the multi-index
        double get_y() const;		//get the y-coordinate (e.g. distance) of the multi-index

        void set_curve(Halfedge* e);	//set the pointer to the curve corresponding to this LCM in the arrangement
        Halfedge* get_curve() const;		//get the pointer to the curve corresponding to this LCM in the arrangement

        void set_position(unsigned p);  //sets the relative position of the LCM curve at the sweep line, used for Bentley-Ottmann DCEL construction algorithm
        unsigned get_position() const;  //gets the relative position of the LCM curve at the sweep line, used for Bentley-Ottmann DCEL construction algorithm

        double get_r_coord(double theta);	//returns the r-coordinate for the point on this LCM curve with the given value of theta

    private:
        double x_coord;		//x-coordinate (e.g. time) of multi-index
        double y_coord;		//y-coordinate (e.g. distance) of multi-inded
        Halfedge* curve;	//pointer to left-most halfedge corresponding to this LCM in the arrangement --- IS THIS USED FOR ANYTHING BESIDES TESTING???
        unsigned position;  //relative position of LCM curve at sweep line, used for Bentley-Ottmann DCEL construction algorithm
};


//TODO: IS IT OK TO IMPLEMENT THE FOLLOWING COMPARATOR HERE, IN THE .h FILE???

class LCM_LeftComparator
{
    public:
        bool operator() (const LCM* lhs, const LCM* rhs) const
        {
            if(lhs->get_y() < rhs->get_y())		//first compare distance value (natural order)
                return true;
            if(lhs->get_y() == rhs->get_y() && lhs->get_x() > rhs->get_x())		//then compare time value (reverse order!)
                return true;
            return false;
        }
};





#endif // __DCEL_LCM_H__
