/**
 * \class	PersistenceData
 * \brief	Stores the data necessary to draw a persistence diagram
 * \author	Matthew L. Wright
 * \date	May 2014
 */

#ifndef __PERSISTENCE_DATA_H__
#define __PERSISTENCE_DATA_H__

//includes????
#include <set>
#include <utility> // std::pair

///THIS IS OBSOLETE! REPLACED BY class Barcode

//// comparator class for sorting the long bars first ////
//NOTE: floating-point errors have negligible effect here
struct PairComparator {
    bool operator()(const std::pair<double, double>& p1, const std::pair<double, double>& p2) const //returns true if p1 comes before p2
    {
        //first, sort by length of bar
        if (p1.second - p1.first > p2.second - p2.first) //then first bar is longer
            return true;

        //if equal, sort by birth time
        if (p1.second - p1.first == p2.second - p2.first)
            return p1.first < p2.first;

        //else
        return false;
    }
};

//// PersistenceData ////

//NOTE: would it be simpler if this were just a set of pairs, with infinity (stored as a double) used to denote cycles?

class PersistenceData {
public:
    PersistenceData(); //constructor

    void add_pair(double birth, double death); //stores a persistence pair
    void add_cycle(double birth); //stores an essential cycle

    std::multiset<std::pair<double, double>, PairComparator>* get_pairs(); //gets a pointer to the vector of persistence pairs
    std::multiset<double>* get_cycles(); //gets a pointer to the vector of essential cycles

private:
    std::multiset<std::pair<double, double>, PairComparator> pairs; //stores persistence pairs (birth and death times)

    std::multiset<double> cycles; //stores birth times of essential cycles
};

#endif // __PERSISTENCE_DATA_H__
