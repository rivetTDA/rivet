#include <algorithm>
#include <fstream>
#include <iostream>
#include <math.h>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "dcel/lcm.hpp"

///// DATA STRUCTURES FOR BENTLEY-OTTMANN ALGORITHM /////
//struct to hold a future intersection event
struct Crossing {
    LCM* a; //pointer to one curve
    LCM* b; //pointer to the other curve -- must ensure that curve for LCM a is below curve for LCM b just before the crossing point!!!!!
    highprecision t; //theta-coordinate of intersection point

    Crossing(LCM* a, LCM* b, highprecision t)
        : a(a)
        , b(b)
        , t(t)
    {
    }
};

//comparator class for ordering crossings: first by theta (left to right); then for a given theta, by r (low to high)
struct CrossingComparator {
    bool operator()(const Crossing* c1, const Crossing* c2) const //returns true if c1 comes after c2
    {
        //TESTING
        if (c1->a->get_position() >= c1->b->get_position() || c2->a->get_position() >= c2->b->get_position()) {
            std::cerr << "INVERTED CROSSING ERROR\n";
            std::cerr << "crossing 1 involves LCMS " << c1->a << " (pos " << c1->a->get_position() << ") and " << c1->b << " (pos " << c1->b->get_position() << "),";
            std::cerr << "crossing 2 involves LCMS " << c2->a << " (pos " << c2->a->get_position() << ") and " << c2->b << " (pos " << c2->b->get_position() << "),";
            throw std::exception();
        }

        if (c1->t - c2->t > pow(2, -30)) //then c1 > c2      <<<------------ THIS ISN'T SO GOOD....IMPROVE!!!
            return true;
        if (c1->t - c2->t < -1 * pow(2, -30)) //then c1 < c2     <<<------------ THIS ISN'T SO GOOD....IMPROVE!!!
            return false;

        //else, then c1->t == c2->t, so consider relative position of curves

        //TESTING
        if (c1->a->get_position() == c2->a->get_position()) {
            std::cerr << "ILLEGAL CROSSING ERROR\n";
            std::cerr << "crossing 1 involves LCMS " << c1->a << " (pos " << c1->a->get_position() << ") and " << c1->b << " (pos " << c1->b->get_position() << "),";
            std::cerr << "crossing 2 involves LCMS " << c2->a << " (pos " << c2->a->get_position() << ") and " << c2->b << " (pos " << c2->b->get_position() << "),";
            throw std::exception();
        }

        return c1->a->get_position() > c2->a->get_position();
    }
};

struct MyComparator {
    bool operator()(const int a, const int b) const //returns true if a > b; this makes the priority_queue give priority to SMALLER elements
    {
        return a > b;
    }
};

///// TESTING THE PRIORITY QUEUE
int main(int argc, char* argv[])
{
    std::priority_queue<int, std::vector<int>, MyComparator> myqueue;

    myqueue.push(10);
    myqueue.push(20);
    myqueue.push(15);
    myqueue.push(5);

    std::cout << "The queue contains:\n";
    while (!myqueue.empty()) {
        std::cout << myqueue.top() << "\n";
        myqueue.pop();
    }

} //end main()
